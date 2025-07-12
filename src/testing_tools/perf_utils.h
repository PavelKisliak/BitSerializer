/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <iostream>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#elif __linux__
#include <sched.h>
#include <sys/resource.h>
#elif __APPLE__
#include <pthread.h>
#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#endif

namespace PerfUtils
{
	/**
	 * @brief Sets the current process or thread to the maximum available priority.
	 *
	 * This function attempts to elevate the scheduling priority of the current process/thread
	 * to reduce interference from other processes during performance-critical operations, such as benchmarking.
	 */
	static void SetMaxPriority()
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

} // namespace PerfUtils
