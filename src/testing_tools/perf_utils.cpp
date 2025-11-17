/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "perf_utils.h"

#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#elif __linux__
#include <sched.h>
#include <pthread.h>
#include <sys/resource.h>
#elif __APPLE__
#include <pthread.h>
#include <mach/mach_init.h>
#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#endif

namespace PerfUtils
{
	namespace
	{
		// Prevent optimization: volatile sink
		volatile double HeatSink = 0.0;
	}

	void SetMaxPriority()
	{
#ifdef _WIN32
		// Windows: Set the process to real-time priority class
		if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS)) {
			std::cerr << "Failed to set real-time priority on Windows. "
				"Error code: " << GetLastError() << std::endl;
		}

#elif __linux__
		// Linux: Use SCHED_FIFO real-time scheduling policy with max priority
		struct sched_param param;
		param.sched_priority = sched_get_priority_max(SCHED_FIFO);
		if (param.sched_priority == -1)
		{
			std::cerr << "Failed to retrieve max real-time priority on Linux." << std::endl;
		}
		else if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
			perror("Failed to set real-time priority on Linux");
		}

#elif __APPLE__
		// macOS: Configure thread to non-timesharing mode (fixed priority)
		thread_extended_policy_data_t policy;
		policy.timeshare = false; // Disable timesharing (set to fixed priority)

		// Get the Mach thread port for the current thread
		thread_port_t threadPort = pthread_mach_thread_np(pthread_self());

		// Apply the new thread policy
		kern_return_t result = thread_policy_set(
			threadPort,                // Current thread's Mach port
			THREAD_EXTENDED_POLICY,    // Policy type
			(thread_policy_t)&policy,  // Policy data
			THREAD_EXTENDED_POLICY_COUNT // Number of policy elements
		);

		if (result != KERN_SUCCESS) {
			std::cerr << "Failed to set thread priority on macOS. "
				<< "Kernel error code: " << result << std::endl;
		}

#else
		std::cerr << "Setting max priority is not supported on this platform." << std::endl;
#endif
	}

	void PreheatCPU(std::chrono::steady_clock::duration duration)
	{
		using namespace std::chrono;

		const auto start = steady_clock::now();

		// Allocate a modest working set (~64 KB) to stress L1/L2 cache
		constexpr size_t N = 8192; // 8192 * sizeof(double) ≈ 64 KB
		std::vector<double> data(N);

		// Initialize with non-constant, non-zero values
		std::default_random_engine rng(12345);
		std::uniform_real_distribution<double> dist(0.1, 1.0);
		for (auto& x : data) {
			x = dist(rng);
		}

		double accumulator = 1.0;

		while (steady_clock::now() - start < duration)
		{
			// Perform FP-heavy work with data dependency and memory access
			for (size_t i = 0; i < N; ++i)
			{
				// Mix of transcendental, arithmetic, and memory ops
				double val = data[i];
				val = std::sin(val) + std::cos(val);
				val = std::sqrt(val * val + 0.5);
				val = std::log(val + 1.0) * std::exp(-val);
				data[i] = val; // write back to stress cache
				accumulator += val;
				// Keep dependency chain alive
				accumulator = std::fma(accumulator, 0.999, val * 1e-6);
			}
		}

		// Force compiler to preserve computation
		HeatSink = accumulator;
	}

	void SetCurrentThreadAffinity(std::uint32_t coreId)
	{
#if defined(_WIN32) || defined(_WIN64)
		// Windows: SetThreadAffinityMask
		DWORD_PTR mask = 1ULL << coreId;
		HANDLE thread = GetCurrentThread();
		if (SetThreadAffinityMask(thread, mask) == 0) {
			throw std::runtime_error("Failed to set thread affinity on Windows");
		}

#elif defined(__linux__)
		// Linux: pthread_setaffinity_np
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		if (coreId >= CPU_SETSIZE) {
			throw std::runtime_error("Core ID exceeds CPU_SETSIZE on Linux");
		}
		CPU_SET(coreId, &cpuset);
		if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
			throw std::runtime_error("Failed to set thread affinity on Linux");
		}

#elif defined(__APPLE__) && defined(__MACH__)
		// macOS: Thread affinity is not directly exposed via CPU ID.
		// Workaround: Use thread_policy_set with THREAD_AFFINITY_POLICY.
		// Note: macOS uses "affinity tags", not core IDs — behavior is advisory.
		thread_affinity_policy_data_t policy = { static_cast<integer_t>(coreId) };
		kern_return_t ret = thread_policy_set(
			mach_thread_self(),
			THREAD_AFFINITY_POLICY,
			reinterpret_cast<thread_policy_t>(&policy),
			THREAD_AFFINITY_POLICY_COUNT
		);
		if (ret != KERN_SUCCESS) {
			throw std::runtime_error("Failed to set thread affinity on macOS");
		}

#else
		// Unsupported platform
		static_cast<void>(coreId); // suppress unused warning
		throw std::runtime_error("Thread affinity not supported on this platform");
#endif
	}
}
