/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <chrono>
#include <cstdint>

namespace BenchHarness
{
	/**
	 * @brief Sets the current process or thread to the maximum available priority.
	 *
	 * Elevates the scheduling priority to reduce interference from other processes
	 * during performance-critical operations such as benchmarking.
	 */
	void SetMaxPriority();

	/**
	 * @brief Warms up the CPU to bring it out of a low-power state before benchmarking.
	 * @param duration How long the warm-up workload runs.
	 */
	void PreheatCPU(std::chrono::steady_clock::duration duration);

	/**
	 * @brief Pins the current thread to a specific CPU core.
	 * @param coreId Zero-based logical core index (e.g. 0, 1, 2...).
	 * @throws std::runtime_error if the operation fails or is unsupported.
	 */
	void SetCurrentThreadAffinity(std::uint32_t coreId);

	/**
	 * @brief Prepares the process and the current thread for benchmarking.
	 *
	 * Raises the process/thread priority, optionally pins the current thread to a
	 * fixed core, and warms up the CPU to bring it out of a low-power state.
	 *
	 * @param preheatDuration How long the CPU warm-up runs; pass zero to skip it.
	 * @param pinToCore When true, pin the current thread to a fixed core.
	 */
	void PrepareEnvironment(std::chrono::steady_clock::duration preheatDuration = std::chrono::seconds(1), bool pinToCore = true);
} // namespace BenchHarness
