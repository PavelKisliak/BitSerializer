/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "perf_utils.h"

#include <cmath>
#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <thread>
#include <vector>

#if defined(_WIN32)
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
#elif defined(__linux__)
	#include <pthread.h>
	#include <sched.h>
	#include <sys/resource.h>
#elif defined(__APPLE__)
	#include <mach/mach_init.h>
	#include <mach/thread_act.h>
	#include <mach/thread_policy.h>
	#include <pthread.h>
#endif

namespace BenchHarness
{
	namespace
	{
		// Prevents the compiler from optimizing the warm-up loop away.
		volatile double heatSink = 0.0;
	}

	void SetMaxPriority()
	{
#if defined(_WIN32)
		// Windows: set the process to the real-time priority class.
		if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
		{
			std::cerr << "Failed to set real-time priority on Windows. "
				"Error code: " << GetLastError() << std::endl;
		}

#elif defined(__linux__)
		// Linux: use the SCHED_FIFO real-time scheduling policy with the maximum priority.
		struct sched_param param;
		param.sched_priority = sched_get_priority_max(SCHED_FIFO);
		if (param.sched_priority == -1)
		{
			std::cerr << "Failed to retrieve max real-time priority on Linux." << std::endl;
		}
		else if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
		{
			perror("Failed to set real-time priority on Linux");
		}

#elif defined(__APPLE__)
		// macOS: configure the thread as non-timesharing (fixed priority).
		thread_extended_policy_data_t policy;
		policy.timeshare = false;

		const thread_port_t threadPort = pthread_mach_thread_np(pthread_self());
		const kern_return_t result = thread_policy_set(
			threadPort,
			THREAD_EXTENDED_POLICY,
			reinterpret_cast<thread_policy_t>(&policy),
			THREAD_EXTENDED_POLICY_COUNT);

		if (result != KERN_SUCCESS)
		{
			std::cerr << "Failed to set thread priority on macOS. "
				<< "Kernel error code: " << result << std::endl;
		}

#else
		std::cerr << "Setting max priority is not supported on this platform." << std::endl;
#endif
	}

	void PreheatCPU(std::chrono::steady_clock::duration duration)
	{
		const auto start = std::chrono::steady_clock::now();

		// A modest working set (~64 KB) to stress the L1/L2 caches.
		constexpr std::size_t BufferSize = 8192; // 8192 * sizeof(double) ~ 64 KB
		std::vector<double> data(BufferSize);

		// Initialize with non-constant, non-zero values.
		std::default_random_engine rng(12345);
		std::uniform_real_distribution<double> dist(0.1, 1.0);
		for (auto& x : data)
		{
			x = dist(rng);
		}

		double accumulator = 1.0;
		while (std::chrono::steady_clock::now() - start < duration)
		{
			for (std::size_t i = 0; i < BufferSize; ++i)
			{
				// Mix of transcendental, arithmetic and memory operations.
				double value = data[i];
				value = std::sin(value) + std::cos(value);
				value = std::sqrt(value * value + 0.5);
				value = std::log(value + 1.0) * std::exp(-value);
				data[i] = value; // write back to stress the caches
				accumulator += value;
				accumulator = std::fma(accumulator, 0.999, value * 1e-6);
			}
		}

		heatSink = accumulator;
	}

	void SetCurrentThreadAffinity(std::uint32_t coreId)
	{
#if defined(_WIN32)
		const DWORD_PTR mask = static_cast<DWORD_PTR>(1) << coreId;
		if (SetThreadAffinityMask(GetCurrentThread(), mask) == 0)
		{
			throw std::runtime_error("Failed to set thread affinity on Windows");
		}

#elif defined(__linux__)
		if (coreId >= CPU_SETSIZE)
		{
			throw std::runtime_error("Core ID exceeds CPU_SETSIZE on Linux");
		}
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(coreId, &cpuset);
		if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0)
		{
			throw std::runtime_error("Failed to set thread affinity on Linux");
		}

#elif defined(__APPLE__)
		// macOS exposes advisory "affinity tags" rather than core IDs.
		thread_affinity_policy_data_t policy = { static_cast<integer_t>(coreId) };
		const kern_return_t result = thread_policy_set(
			mach_thread_self(),
			THREAD_AFFINITY_POLICY,
			reinterpret_cast<thread_policy_t>(&policy),
			THREAD_AFFINITY_POLICY_COUNT);
		if (result != KERN_SUCCESS)
		{
			throw std::runtime_error("Failed to set thread affinity on macOS");
		}

#else
		static_cast<void>(coreId);
		throw std::runtime_error("Thread affinity not supported on this platform");
#endif
	}

	void PrepareEnvironment(std::chrono::steady_clock::duration preheatDuration, bool pinToCore)
	{
		SetMaxPriority();
		if (pinToCore)
		{
			const auto hardwareThreads = std::thread::hardware_concurrency();
			if (hardwareThreads > 0)
			{
				SetCurrentThreadAffinity((hardwareThreads - 1) >> 1);
			}
		}
		if (preheatDuration > std::chrono::steady_clock::duration::zero())
		{
			PreheatCPU(preheatDuration);
		}
	}
} // namespace BenchHarness
