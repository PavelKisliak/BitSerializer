/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "perf_utils.h"
#include "bitserializer/types/std/unordered_map.h"

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

#if defined(_DEBUG) || (!defined(NDEBUG) && !defined(RELEASE))
static constexpr bool isDebugBuild = true;
#else
static constexpr bool isDebugBuild = false;
#endif

// In the debug configuration using minimal testing time, otherwise take enough time to produce repeatable results
static constexpr std::chrono::seconds defaultStageTestTime = std::chrono::seconds(isDebugBuild ? 1 : 30);

int main()	// NOLINT(bugprone-exception-escape)
{
	std::vector<LibraryTestResults> benchmarkResults;
	std::cout << "Testing, please do not touch mouse and keyboard." << std::endl;

	// Raise priority, pin the current thread to a fixed core, and warm up the CPU
	// (skip the warm-up in debug builds to keep them fast).
	if constexpr (!isDebugBuild)
	{
		std::cout << "Warm up the CPU" << std::endl;
	}
	BenchHarness::PrepareEnvironment(isDebugBuild ? std::chrono::steady_clock::duration::zero() : std::chrono::seconds(10));

	// Benchmarking libraries
	try
	{
#ifdef JSON_BENCHMARK
		BitSerializerBenchmark<BitSerializer::Json::JsonArchive> jsonBenchmark;
		benchmarkResults.push_back(jsonBenchmark.RunBenchmark(defaultStageTestTime));

		BitSerializerBenchmark<BitSerializer::Json::JsoncArchive> jsoncBenchmark;
		benchmarkResults.push_back(jsoncBenchmark.RunBenchmark(defaultStageTestTime));
#endif
#ifdef CSV_BENCHMARK
		BitSerializerBenchmark<BitSerializer::Csv::CsvArchive> csvBenchmark;
		benchmarkResults.push_back(csvBenchmark.RunBenchmark(defaultStageTestTime));
#endif
#ifdef MSGPACK_BENCHMARK
		BitSerializerBenchmark<BitSerializer::MsgPack::MsgPackArchive> msgPackBenchmark;
		benchmarkResults.push_back(msgPackBenchmark.RunBenchmark(defaultStageTestTime));
#endif
#ifdef RAPIDJSON_BENCHMARK
		BitSerializerBenchmark<BitSerializer::Json::RapidJson::JsonArchive> bitSerializerRapidJsonBenchmark;
		benchmarkResults.push_back(bitSerializerRapidJsonBenchmark.RunBenchmark(defaultStageTestTime));

		RapidJsonBenchmark rapidJsonBenchmark;
		benchmarkResults.push_back(rapidJsonBenchmark.RunBenchmark(defaultStageTestTime));
#endif
#ifdef PUGIXML_BENCHMARK
		BitSerializerBenchmark<BitSerializer::Xml::PugiXml::XmlArchive> bitSerializerPugiXmlBenchmark;
		benchmarkResults.push_back(bitSerializerPugiXmlBenchmark.RunBenchmark(defaultStageTestTime));

		PugiXmlBenchmark pugiXmlBenchmark;
		benchmarkResults.push_back(pugiXmlBenchmark.RunBenchmark(defaultStageTestTime));
#endif
#ifdef RAPIDYAML_BENCHMARK
		BitSerializerBenchmark<BitSerializer::Yaml::RapidYaml::YamlArchive> bitSerializerRapidYamlBenchmark;
		benchmarkResults.push_back(bitSerializerRapidYamlBenchmark.RunBenchmark(defaultStageTestTime));

		RapidYamlBenchmark rapidYamlBenchmark;
		benchmarkResults.push_back(rapidYamlBenchmark.RunBenchmark(defaultStageTestTime));
#endif
		NlohmannMsgPackBenchmark nlohmannMsgPackBenchmark;
		benchmarkResults.push_back(nlohmannMsgPackBenchmark.RunBenchmark(defaultStageTestTime));
	}
	catch (const std::exception& ex)
	{
		std::cerr << std::endl << ex.what() << std::endl;
		return 1;
	}

	// Serializes the results of the libraries tests (requires the BitSerializer RapidJson archive build to be enabled)
#ifdef RAPIDJSON_BENCHMARK
	auto outputDir = std::filesystem::current_path() / "benchmark_results";
	if (exists(outputDir) || create_directories(outputDir))
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
			BitSerializer::SerializationOptions options;
			options.streamOptions.writeBom = false;
			options.formatOptions.enableFormat = true;

			// Save serialization speed report
			auto outputFile = outputDir / "serialization_speed_report.json";
			BitSerializer::SaveObjectToFile<BitSerializer::Json::RapidJson::JsonArchive>(serializationSpeedReport, outputFile, options, true);

			// Save serialization output size report
			outputFile = outputDir / "serialization_output_size_report.json";
			BitSerializer::SaveObjectToFile<BitSerializer::Json::RapidJson::JsonArchive>(serializationOutputSizeReport, outputFile, options, true);
		}
		catch (const std::exception& ex)
		{
			std::cerr << "Unable to save benchmark results: " << ex.what() << std::endl;
			return 1;
		}
		std::cout << std::endl << "Benchmark results has been saved to directory:" << outputDir << std::endl;
	}
	else
	{
		std::cerr << "Unable to create output directory: " << outputDir << std::endl;
	}
#endif

	return 0;
}
