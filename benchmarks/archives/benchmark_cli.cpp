/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "benchmark_cli.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>

namespace BenchmarkCli
{
	namespace
	{
		[[nodiscard]] std::string ToLower(std::string value)
		{
			std::transform(value.begin(), value.end(), value.begin(),
				[](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
			return value;
		}

		[[nodiscard]] std::vector<std::string> SplitList(const std::string& value)
		{
			std::vector<std::string> items;
			std::string current;
			for (const char ch : value)
			{
				if (ch == ',' || std::isspace(static_cast<unsigned char>(ch)))
				{
					if (!current.empty())
					{
						items.push_back(ToLower(current));
						current.clear();
					}
				}
				else
				{
					current += ch;
				}
			}
			if (!current.empty())
			{
				items.push_back(ToLower(current));
			}
			return items;
		}

		[[nodiscard]] TestStage ParseStageToken(const std::string& token)
		{
			if (token == "save-memory")
			{
				return TestStage::SaveToMemory;
			}
			if (token == "load-memory")
			{
				return TestStage::LoadFromMemory;
			}
			if (token == "save-stream")
			{
				return TestStage::SaveToStream;
			}
			if (token == "load-stream")
			{
				return TestStage::LoadFromStream;
			}
			throw std::runtime_error("Unknown stage: " + token);
		}

		[[nodiscard]] std::vector<TestStage> ParseStages(const std::string& value)
		{
			std::vector<TestStage> stages;
			auto addStage = [&stages](TestStage stage)
			{
				if (std::find(stages.cbegin(), stages.cend(), stage) == stages.cend())
				{
					stages.push_back(stage);
				}
			};

			for (const std::string& token : SplitList(value))
			{
				if (token == "save")
				{
					addStage(TestStage::SaveToMemory);
					addStage(TestStage::SaveToStream);
				}
				else if (token == "load")
				{
					addStage(TestStage::LoadFromMemory);
					addStage(TestStage::LoadFromStream);
				}
				else if (token == "memory")
				{
					addStage(TestStage::SaveToMemory);
					addStage(TestStage::LoadFromMemory);
				}
				else if (token == "stream")
				{
					addStage(TestStage::SaveToStream);
					addStage(TestStage::LoadFromStream);
				}
				else if (token == "all")
				{
					addStage(TestStage::SaveToMemory);
					addStage(TestStage::LoadFromMemory);
					addStage(TestStage::SaveToStream);
					addStage(TestStage::LoadFromStream);
				}
				else
				{
					addStage(ParseStageToken(token));
				}
			}
			return stages;
		}

		[[nodiscard]] int ParseInt(const std::string& value, const char* optionName)
		{
			try
			{
				std::size_t consumed = 0;
				const int result = std::stoi(value, &consumed);
				if (consumed != value.size())
				{
					throw std::invalid_argument("trailing characters");
				}
				return result;
			}
			catch (const std::exception&)
			{
				throw std::runtime_error(std::string("Invalid value for ") + optionName + ": " + value);
			}
		}
	} // namespace

	void PrintUsage()
	{
		std::cout
			<< "Usage: archives_benchmarks [options]\n\n"
			<< "Selection:\n"
			<< "  --bench <list>       Benchmarks (default: all):\n"
			<< "                       bs-json, bs-jsonc, bs-csv, bs-msgpack,\n"
			<< "                       bs-rapidjson, bs-pugixml, bs-rapidyaml,\n"
			<< "                       rapidjson, pugixml, rapidyaml, nlohmann-msgpack\n"
			<< "                       meta: bitserializer, competitors, all\n"
			<< "  --stages <list>      Stages (default: all):\n"
			<< "                       save-memory, load-memory, save-stream, load-stream\n"
			<< "                       aliases: save, load, memory, stream, all\n"
			<< "  --list               List compiled benchmarks and their stages\n\n"
			<< "Timing:\n"
			<< "  --stage-time <sec>   Time per stage (default: 30; debug: 1)\n"
			<< "  --warmup <sec>       CPU warm-up before the run (default: 10; debug: 0)\n\n"
			<< "Output:\n"
			<< "  --output <dir>       Report directory (default: benchmark_results)\n"
			<< "  --no-report          Do not write JSON reports\n\n"
			<< "  --help, -h           Show this help\n";
	}

	Options ParseOptions(int argc, char* argv[])
	{
		Options options;
		for (int i = 1; i < argc; ++i)
		{
			const std::string arg = argv[i];
			auto readValue = [&](const char* optionName) -> std::string
			{
				if (i + 1 >= argc)
				{
					throw std::runtime_error(std::string("Missing value for ") + optionName);
				}
				return argv[++i];
			};

			if (arg == "--bench")
			{
				options.benchKeys = SplitList(readValue("--bench"));
			}
			else if (arg == "--stages")
			{
				options.stages = ParseStages(readValue("--stages"));
			}
			else if (arg == "--stage-time")
			{
				const int seconds = ParseInt(readValue("--stage-time"), "--stage-time");
				if (seconds < 1)
				{
					throw std::runtime_error("--stage-time must be at least 1 second");
				}
				options.stageTime = std::chrono::seconds(seconds);
			}
			else if (arg == "--warmup")
			{
				const int seconds = ParseInt(readValue("--warmup"), "--warmup");
				if (seconds < 0)
				{
					throw std::runtime_error("--warmup must be non-negative");
				}
				options.warmup = std::chrono::seconds(seconds);
			}
			else if (arg == "--output")
			{
				options.outputDir = readValue("--output");
			}
			else if (arg == "--no-report")
			{
				options.noReport = true;
			}
			else if (arg == "--list")
			{
				options.listOnly = true;
			}
			else if (arg == "--help" || arg == "-h")
			{
				options.showHelp = true;
			}
			else
			{
				throw std::runtime_error("Unknown option: " + arg);
			}
		}
		return options;
	}
} // namespace BenchmarkCli
