/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <iostream>
#include <chrono>

#include "testing_tools/perf_utils.h"
#include "bitserializer/types/std/unordered_map.h"

#include "competitors/bitserializer_benchmark.h"
#include "competitors/rapidjson_benchmark.h"
#include "competitors/pugixml_benchmark.h"
#include "competitors/rapidyaml_benchmark.h"

#if defined(_DEBUG) || (!defined(NDEBUG) && !defined(RELEASE))
// In the debug configuration using minimal testing time
static constexpr std::chrono::seconds DefaultStageTestTime = std::chrono::seconds(1);
#else
// Need enough time to produce repeatable results
static constexpr std::chrono::seconds DefaultStageTestTime = std::chrono::seconds(30);
#endif

int main()	// NOLINT(bugprone-exception-escape)
{
	// Set the highest priority for the process to minimize fluctuations in results.
	// Setting physical core affinity leads to worse results (possibly due to throttling).
	PerfUtils::SetMaxPriority();

	std::vector<CLibraryTestResults> benchmarkResults;
	std::cout << "Testing, please do not touch mouse and keyboard." << std::endl;

	// Benchmarking libraries
	try
	{
#ifdef CSV_BENCHMARK
		CBitSerializerBenchmark<BitSerializer::Csv::CsvArchive> csvBenchmark;
		benchmarkResults.push_back(csvBenchmark.RunBenchmark(DefaultStageTestTime));
#endif
#ifdef MSGPACK_BENCHMARK
		CBitSerializerBenchmark<BitSerializer::MsgPack::MsgPackArchive> msgPackBenchmark;
		benchmarkResults.push_back(msgPackBenchmark.RunBenchmark(DefaultStageTestTime));
#endif
#ifdef RAPIDJSON_BENCHMARK
		CBitSerializerBenchmark<BitSerializer::Json::RapidJson::JsonArchive> bitSerializerRapidJsonBenchmark;
		benchmarkResults.push_back(bitSerializerRapidJsonBenchmark.RunBenchmark(DefaultStageTestTime));

		CRapidJsonBenchmark rapidJsonBenchmark;
		benchmarkResults.push_back(rapidJsonBenchmark.RunBenchmark(DefaultStageTestTime));
#endif
#ifdef PUGIXML_BENCHMARK
		CBitSerializerBenchmark<BitSerializer::Xml::PugiXml::XmlArchive> bitSerializerPugiXmlBenchmark;
		benchmarkResults.push_back(bitSerializerPugiXmlBenchmark.RunBenchmark(DefaultStageTestTime));

		CPugiXmlBenchmark pugiXmlBenchmark;
		benchmarkResults.push_back(pugiXmlBenchmark.RunBenchmark(DefaultStageTestTime));
#endif
#ifdef RAPIDYAML_BENCHMARK
		CBitSerializerBenchmark<BitSerializer::Yaml::RapidYaml::YamlArchive> bitSerializerRapidYamlBenchmark;
		benchmarkResults.push_back(bitSerializerRapidYamlBenchmark.RunBenchmark(DefaultStageTestTime));

		CRapidYamlBenchmark rapidYamlBenchmark;
		benchmarkResults.push_back(rapidYamlBenchmark.RunBenchmark(DefaultStageTestTime));
#endif
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
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

		for (const CLibraryTestResults& benchmarkResult : benchmarkResults)
		{
			std::unordered_map<TestStage, uint64_t> stagesTest;
			for (const auto& stageTestResults : benchmarkResult.StagesTestResults)
			{
				stagesTest.emplace(stageTestResults.first, stageTestResults.second.SerializationSpeed);
			}
			serializationSpeedReport.emplace(benchmarkResult.LibraryName, std::move(stagesTest));
			// Filter results from base libraries (doesn't make sense since they are equal)
			if (benchmarkResult.LibraryName != "RapidJson" && benchmarkResult.LibraryName != "PugiXml" && benchmarkResult.LibraryName != "RapidYaml")
			{
				serializationOutputSizeReport.emplace(benchmarkResult.LibraryName, benchmarkResult.SerializedDataSize);
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
		catch (const std::exception& ex) {
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
