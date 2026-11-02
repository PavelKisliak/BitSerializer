/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

#include "benchmark_base.h"

namespace BenchmarkCli
{
#if defined(_DEBUG) || (!defined(NDEBUG) && !defined(RELEASE))
	inline constexpr bool IsDebugBuild = true;
#else
	inline constexpr bool IsDebugBuild = false;
#endif

	/**
	 * @brief Command-line options of the archives benchmark.
	 */
	struct Options
	{
		std::vector<std::string> benchKeys;
		std::vector<TestStage> stages;
		std::chrono::seconds stageTime{IsDebugBuild ? 1 : 30};
		std::chrono::seconds warmup{IsDebugBuild ? 0 : 10};
		std::filesystem::path outputDir = "benchmark_results";
		bool noReport = false;
		bool listOnly = false;
		bool showHelp = false;
	};

	/**
	 * @brief Parses the command-line options.
	 *
	 * @throws std::runtime_error on an unknown option, an unknown stage or an
	 * invalid value.
	 */
	[[nodiscard]] Options ParseOptions(int argc, char* argv[]);

	/**
	 * @brief Prints the command-line usage.
	 */
	void PrintUsage();
} // namespace BenchmarkCli
