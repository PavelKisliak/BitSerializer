/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <iostream>
#include <chrono>
#include <cmath>

#include "bitserializer_benchmark.h"
#include "bitserializer/types/std/map.h"

#include "competitors/rapid_json_benchmark.h"
#include "competitors/pugixml_benchmark.h"
#include "competitors/rapid_yaml_benchmark.h"

#if defined(_DEBUG) || (!defined(NDEBUG) && !defined(RELEASE))
// In the debug configuration using minimal testing time
constexpr auto DefaultStageTestTime = std::chrono::seconds(1);
#else
// Need enough time to produce repeatable results
constexpr auto DefaultStageTestTime = std::chrono::seconds(30);
#endif

using Timer = std::chrono::high_resolution_clock;
using CLibraryTestResult = std::map<std::string, uint64_t>;

/// <summary>
/// Executes benchmark.
/// </summary>
template <class TModel>
CLibraryTestResult RunBenchmark(CBenchmarkBase<TModel>& benchmark, const std::chrono::seconds testTime = DefaultStageTestTime)
{
	constexpr auto NanosecondsInMs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1)).count();
	int progressPercent = -1;
	CLibraryTestResult libTestResult;

	do
	{
		const auto beginTime = Timer::now();
		const auto endTime = beginTime + std::chrono::seconds(testTime);
		const auto testTimeMSec = std::chrono::duration_cast<std::chrono::milliseconds>(testTime).count();
		std::chrono::nanoseconds minTime{};

		for (auto time = beginTime; time < endTime; time = Timer::now())
		{
			// Print progress
			const auto newPercent = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(time - beginTime).count() * 100 / testTimeMSec);
			if (progressPercent != newPercent)
			{
				progressPercent = newPercent;
				std::cout << "\r" << benchmark.GetLibraryName() << " | " << benchmark.GetCurrentStageName() << ": " << progressPercent << "%";
			}

			auto startTime = Timer::now();
			benchmark.PerformTest();
			const auto testDuration = Timer::now() - startTime;
			if (minTime == std::chrono::nanoseconds(0) || minTime > testDuration)
			{
				minTime = testDuration;
			}
		}

		std::cout << "\r";

		// Calculate speed - fields/ms
		const int64_t fieldsSpeed = std::llround(
			NanosecondsInMs / static_cast<double>(minTime.count()) * static_cast<double>(benchmark.GetTotalFieldsCount()));
		libTestResult.emplace(benchmark.GetCurrentStageName(), fieldsSpeed);

		// Display result
		std::cout << benchmark.GetLibraryName() << " | " << benchmark.GetCurrentStageName() << ": " << fieldsSpeed << " (fields/ms)" << std::endl;
	}
	while (benchmark.NextStage());

	return libTestResult;
}

int main()
{
	std::map<std::string, CLibraryTestResult> benchmarkResults;
	std::cout << "Testing, please do not touch mouse and keyboard (test may take few minutes)." << std::endl;

	try
	{
#ifdef CSV_BENCHMARK
		CBitSerializerBenchmark<BitSerializer::Csv::CsvArchive> csvBenchmark;
		benchmarkResults.emplace(csvBenchmark.GetLibraryName(), RunBenchmark(csvBenchmark));
#endif
#ifdef MSGPACK_BENCHMARK
		CBitSerializerBenchmark<BitSerializer::MsgPack::MsgPackArchive> msgPackBenchmark;
		benchmarkResults.emplace(msgPackBenchmark.GetLibraryName(), RunBenchmark(msgPackBenchmark));
#endif
#ifdef RAPIDJSON_BENCHMARK
		CBitSerializerBenchmark<BitSerializer::Json::RapidJson::JsonArchive> bitSerializerRapidJsonBenchmark;
		benchmarkResults.emplace(bitSerializerRapidJsonBenchmark.GetLibraryName(), RunBenchmark(bitSerializerRapidJsonBenchmark));

		CRapidJsonBenchmark rapidJsonBenchmark;
		benchmarkResults.emplace(rapidJsonBenchmark.GetLibraryName(), RunBenchmark(rapidJsonBenchmark));
#endif
#ifdef PUGIXML_BENCHMARK
		CBitSerializerBenchmark<BitSerializer::Csv::CsvArchive> bitSerializerPugiXmlBenchmark;
		benchmarkResults.emplace(bitSerializerPugiXmlBenchmark.GetLibraryName(), RunBenchmark(bitSerializerPugiXmlBenchmark));

		CPugiXmlBenchmark pugiXmlBenchmark;
		benchmarkResults.emplace(pugiXmlBenchmark.GetLibraryName(), RunBenchmark(pugiXmlBenchmark));
#endif
#ifdef RAPIDYAML_BENCHMARK
		CBitSerializerBenchmark<BitSerializer::Yaml::RapidYaml::YamlArchive> bitSerializerRapidYamlBenchmark;
		benchmarkResults.emplace(bitSerializerRapidYamlBenchmark.GetLibraryName(), RunBenchmark(bitSerializerRapidYamlBenchmark));

		CRapidYamlBenchmark rapidYamlBenchmark;
		benchmarkResults.emplace(rapidYamlBenchmark.GetLibraryName(), RunBenchmark(rapidYamlBenchmark));
#endif
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}

#ifdef RAPIDJSON_BENCHMARK
	// Serializes the results of the libraries test
	auto outputFile = std::filesystem::current_path() / "bitserializer_benchmark_results.json";
	BitSerializer::SerializationOptions options;
	options.streamOptions.writeBom = false;
	options.formatOptions.enableFormat = true;
	try
	{
		BitSerializer::SaveObjectToFile<BitSerializer::Json::RapidJson::JsonArchive>(benchmarkResults, outputFile, options, true);
		std::cout << "Benchmark results has been saved to file: " << outputFile << std::endl;
	}
	catch (const std::exception& ex) {
		std::cerr << "Unable to save benchmark results: " << ex.what() << std::endl;
		return 1;
	}
#endif

	return 0;
}
