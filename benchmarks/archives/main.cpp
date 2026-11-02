/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "perf_utils.h"
#include "bitserializer/types/std/unordered_map.h"

#include "benchmark_cli.h"
#include "benchmark_base.h"
#include "competitors/bitserializer_benchmark.h"
#ifdef RAPIDJSON_BENCHMARK
#include "competitors/rapidjson_benchmark.h"
#endif
#ifdef PUGIXML_BENCHMARK
#include "competitors/pugixml_benchmark.h"
#endif
#ifdef RAPIDYAML_BENCHMARK
#include "competitors/rapidyaml_benchmark.h"
#endif
#include "competitors/nlohmannjson_benchmark.h"

#ifdef JSON_BENCHMARK
#include "bitserializer/bit_serializer.h"
#include "bitserializer/json_archive.h"
#endif

namespace
{
	using BenchmarkFactory = std::function<std::unique_ptr<BenchmarkBase>()>;

	struct BenchmarkEntry
	{
		std::string key;
		BenchmarkFactory create;
	};

	bool IsBitSerializerKey(std::string_view key)
	{
		return key.rfind("bs-", 0) == 0;
	}

	std::vector<BenchmarkEntry> BuildBenchmarkRegistry()
	{
		std::vector<BenchmarkEntry> entries;
#ifdef JSON_BENCHMARK
		entries.push_back({ "bs-json", [] { return std::make_unique<BitSerializerBenchmark<BitSerializer::Json::JsonArchive>>(); } });
		entries.push_back({ "bs-jsonc", [] { return std::make_unique<BitSerializerBenchmark<BitSerializer::Json::JsoncArchive>>(); } });
#endif
#ifdef CSV_BENCHMARK
		entries.push_back({ "bs-csv", [] { return std::make_unique<BitSerializerBenchmark<BitSerializer::Csv::CsvArchive>>(); } });
#endif
#ifdef MSGPACK_BENCHMARK
		entries.push_back({ "bs-msgpack", [] { return std::make_unique<BitSerializerBenchmark<BitSerializer::MsgPack::MsgPackArchive>>(); } });
#endif
#ifdef RAPIDJSON_BENCHMARK
		entries.push_back({ "bs-rapidjson", [] { return std::make_unique<BitSerializerBenchmark<BitSerializer::Json::RapidJson::JsonArchive>>(); } });
		entries.push_back({ "rapidjson", [] { return std::make_unique<RapidJsonBenchmark>(); } });
#endif
#ifdef PUGIXML_BENCHMARK
		entries.push_back({ "bs-pugixml", [] { return std::make_unique<BitSerializerBenchmark<BitSerializer::Xml::PugiXml::XmlArchive>>(); } });
		entries.push_back({ "pugixml", [] { return std::make_unique<PugiXmlBenchmark>(); } });
#endif
#ifdef RAPIDYAML_BENCHMARK
		entries.push_back({ "bs-rapidyaml", [] { return std::make_unique<BitSerializerBenchmark<BitSerializer::Yaml::RapidYaml::YamlArchive>>(); } });
		entries.push_back({ "rapidyaml", [] { return std::make_unique<RapidYamlBenchmark>(); } });
#endif
		entries.push_back({ "nlohmann-msgpack", [] { return std::make_unique<NlohmannMsgPackBenchmark>(); } });
		return entries;
	}

	std::vector<BenchmarkEntry> SelectBenchmarks(const std::vector<BenchmarkEntry>& registry, const std::vector<std::string>& keys)
	{
		if (keys.empty())
		{
			return registry;
		}

		std::vector<bool> selected(registry.size(), false);
		for (const std::string& key : keys)
		{
			if (key == "all")
			{
				std::fill(selected.begin(), selected.end(), true);
			}
			else if (key == "bitserializer" || key == "competitors")
			{
				const bool wantBitSerializer = key == "bitserializer";
				for (std::size_t i = 0; i < registry.size(); ++i)
				{
					if (IsBitSerializerKey(registry[i].key) == wantBitSerializer)
					{
						selected[i] = true;
					}
				}
			}
			else
			{
				bool found = false;
				for (std::size_t i = 0; i < registry.size(); ++i)
				{
					if (registry[i].key == key)
					{
						selected[i] = true;
						found = true;
					}
				}
				if (!found)
				{
					throw std::runtime_error("Unknown benchmark: " + key);
				}
			}
		}

		std::vector<BenchmarkEntry> result;
		for (std::size_t i = 0; i < registry.size(); ++i)
		{
			if (selected[i])
			{
				result.push_back(registry[i]);
			}
		}
		return result;
	}

	std::string JoinStageNames(const std::vector<TestStage>& stages)
	{
		std::string result;
		for (const TestStage stage : stages)
		{
			if (!result.empty())
			{
				result += ", ";
			}
			result += BitSerializer::Convert::ToString(stage);
		}
		return result;
	}

	void PrintBenchmarkList(const std::vector<BenchmarkEntry>& registry)
	{
		std::cout << "Available benchmarks (compiled in this binary):\n";
		auto printGroup = [](const char* title, const std::vector<BenchmarkEntry>& entries, bool bitSerializerGroup)
		{
			std::cout << "\n  " << title << ":\n";
			for (const BenchmarkEntry& entry : entries)
			{
				if (IsBitSerializerKey(entry.key) != bitSerializerGroup)
				{
					continue;
				}
				const std::unique_ptr<BenchmarkBase> benchmark = entry.create();
				constexpr std::size_t keyWidth = 20;
				std::cout << "    " << entry.key
					<< std::string(keyWidth > entry.key.size() ? keyWidth - entry.key.size() : 0, ' ')
					<< benchmark->GetLibraryName() << ": " << JoinStageNames(benchmark->GetStagesList()) << '\n';
			}
		};
		printGroup("BitSerializer", registry, true);
		printGroup("Competitors", registry, false);
	}
} // namespace

int main(int argc, char* argv[])	// NOLINT(bugprone-exception-escape)
{
	BenchmarkCli::Options options;
	try
	{
		options = BenchmarkCli::ParseOptions(argc, argv);
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << "\n\n";
		BenchmarkCli::PrintUsage();
		return 1;
	}

	if (options.showHelp)
	{
		BenchmarkCli::PrintUsage();
		return 0;
	}

	const std::vector<BenchmarkEntry> registry = BuildBenchmarkRegistry();
	if (options.listOnly)
	{
		PrintBenchmarkList(registry);
		return 0;
	}

	std::vector<BenchmarkEntry> selected;
	try
	{
		selected = SelectBenchmarks(registry, options.benchKeys);
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << "\n\n";
		BenchmarkCli::PrintUsage();
		return 1;
	}

	if (selected.empty())
	{
		std::cerr << "No benchmarks selected." << std::endl;
		return 1;
	}

	std::vector<LibraryTestResults> benchmarkResults;
	std::cout << "Testing, please do not touch mouse and keyboard." << std::endl;

	if (options.warmup.count() > 0)
	{
		std::cout << "Warm up the CPU" << std::endl;
	}
	BenchHarness::PrepareEnvironment(options.warmup);

	try
	{
		for (const BenchmarkEntry& entry : selected)
		{
			const std::unique_ptr<BenchmarkBase> benchmark = entry.create();
			benchmark->SetStageFilter(options.stages);
			if (benchmark->GetSelectedStages().empty())
			{
				std::cout << entry.key << " | " << benchmark->GetLibraryName()
					<< ": skipped (no selected stages are supported)" << std::endl;
				continue;
			}
			benchmarkResults.push_back(benchmark->RunBenchmark(options.stageTime));
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << std::endl << ex.what() << std::endl;
		return 1;
	}

	if (!options.noReport)
	{
#ifdef JSON_BENCHMARK
		const std::filesystem::path outputDir = options.outputDir;
		if (std::filesystem::exists(outputDir) || std::filesystem::create_directories(outputDir))
		{
			// Prepare reports
			std::unordered_map<std::string, std::unordered_map<TestStage, uint64_t>> serializationSpeedReport;
			std::unordered_map<std::string, size_t> serializationOutputSizeReport;

			for (const LibraryTestResults& benchmarkResult : benchmarkResults)
			{
				std::unordered_map<TestStage, uint64_t> stagesTest;
				for (const auto& stageTestResults : benchmarkResult.stagesTestResults)
				{
					stagesTest.emplace(stageTestResults.first, stageTestResults.second.serializationSpeed);
				}
				serializationSpeedReport.emplace(benchmarkResult.libraryName, std::move(stagesTest));
				// Filter results from base libraries (doesn't make sense since they are equal)
				if (benchmarkResult.libraryName != "RapidJson" && benchmarkResult.libraryName != "PugiXml" && benchmarkResult.libraryName != "RapidYaml")
				{
					serializationOutputSizeReport.emplace(benchmarkResult.libraryName, benchmarkResult.serializedDataSize);
				}
			}

			// Save reports
			try
			{
				BitSerializer::SerializationOptions reportOptions;
				reportOptions.streamOptions.writeBom = false;
				reportOptions.formatOptions.enableFormat = true;

				// Save serialization speed report
				auto outputFile = outputDir / "serialization_speed_report.json";
				BitSerializer::SaveObjectToFile<BitSerializer::Json::JsonArchive>(serializationSpeedReport, outputFile, reportOptions, true);

				// Save serialization output size report
				outputFile = outputDir / "serialization_output_size_report.json";
				BitSerializer::SaveObjectToFile<BitSerializer::Json::JsonArchive>(serializationOutputSizeReport, outputFile, reportOptions, true);
			}
			catch (const std::exception& ex)
			{
				std::cerr << "Unable to save benchmark results: " << ex.what() << std::endl;
				return 1;
			}
			std::cout << std::endl << "Benchmark results has been saved to directory: " << outputDir << std::endl;
		}
		else
		{
			std::cerr << "Unable to create output directory: " << outputDir << std::endl;
		}
#else
		std::cerr << "Warning: JSON reports are disabled because BUILD_JSON_ARCHIVE is OFF; "
			"configure with -DBUILD_JSON_ARCHIVE=ON or pass --no-report." << std::endl;
#endif
	}

	return 0;
}
