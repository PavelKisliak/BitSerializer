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
    // Sets the maximum priority for the current process
    static void SetMaxPriority()
	{
#ifdef _WIN32
        // Windows: Set the process to real-time priority
        if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS)) {
            std::cerr << "Failed to set real-time priority on Windows." << std::endl;
        }

#elif __linux__
        // Linux: Set the process to real-time scheduling policy (SCHED_FIFO) with maximum priority
        struct sched_param param;
        param.sched_priority = sched_get_priority_max(SCHED_FIFO);
        if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
            std::cerr << "Failed to set real-time priority on Linux." << std::endl;
        }

#elif __APPLE__
        // macOS: Increase thread priority using Mach thread policy
        thread_extended_policy_data_t policy;
        policy.timeshare = false; // Disable timesharing (set to fixed priority)

        // Get the current thread's Mach port
        thread_port_t threadPort = pthread_mach_thread_np(pthread_self());

        // Set the thread policy
        kern_return_t result = thread_policy_set(
            threadPort, // Current thread's Mach port
            THREAD_EXTENDED_POLICY, // Policy type
            (thread_policy_t)&policy, // Policy data
            THREAD_EXTENDED_POLICY_COUNT // Number of policy elements
        );

        if (result != KERN_SUCCESS) {
            std::cerr << "Failed to set thread priority on macOS. Error: " << result << std::endl;
        }

#else
        std::cerr << "Unsupported platform for setting maximum priority." << std::endl;
#endif
    }
}
