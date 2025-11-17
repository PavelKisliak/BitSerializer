/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <chrono>

namespace PerfUtils
{
	/**
	 * @brief Sets the current process or thread to the maximum available priority.
	 *
	 * This function attempts to elevate the scheduling priority of the current process/thread
	 * to reduce interference from other processes during performance-critical operations, such as benchmarking.
	 */
	void SetMaxPriority();

	/// <summary>
	/// Pre-heating the CPU.
	/// </summary>
	void PreheatCPU(std::chrono::steady_clock::duration duration);

	/**
	  * @brief Pins the current thread to a specific CPU core.
	  *
	  * @param coreId Zero-based logical core index (e.g., 0, 1, 2...).
	  * @throws std::runtime_error if the operation fails or is unsupported.
	  */
	void SetCurrentThreadAffinity(std::uint32_t coreId);
} // namespace PerfUtils
