/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>

#include "bitserializer_benchmark.h"
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

struct TestArchiveResult
{
	struct PerfTestData
	{
		std::chrono::nanoseconds Time{};
		size_t ProcessedFields = 0;
	};

	TestArchiveResult() = default;
	explicit TestArchiveResult(std::string libName)
		: LibraryName(std::move(libName))
	{ }

	std::string LibraryName;
	std::map<std::string, PerfTestData> StagesResult;
};

template <class TModel>
TestArchiveResult RunBenchmark(CBenchmarkBase<TModel>&& benchmark, const std::chrono::seconds testTime = DefaultStageTestTime)
{
	constexpr auto NanosecondsInMs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1)).count();
	TestArchiveResult metadata(benchmark.GetLibraryName());
	int progressPercent = -1;

	do
	{
		TestArchiveResult::PerfTestData& perfTestData = metadata.StagesResult[benchmark.GetCurrentStageName()];

		const auto beginTime = Timer::now();
		const auto endTime = beginTime + std::chrono::seconds(testTime);
		const auto testTimeMSec = std::chrono::duration_cast<std::chrono::milliseconds>(testTime).count();

		for (auto time = beginTime; time < endTime; time = Timer::now())
		{
			// Print progress
			const auto newPercent = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(time - beginTime).count() * 100 / testTimeMSec);
			if (progressPercent != newPercent)
			{
				progressPercent = newPercent;
				std::cout << "\r" << metadata.LibraryName << " | " << benchmark.GetCurrentStageName() << ": " << progressPercent << "%";
			}

			auto startTime = Timer::now();
			benchmark.PerformTest();
			const auto testDuration = Timer::now() - startTime;
			if (perfTestData.Time == std::chrono::nanoseconds(0) || perfTestData.Time > testDuration)
			{
				perfTestData.Time = testDuration;
				perfTestData.ProcessedFields = benchmark.GetTotalFieldsCount();
			}
		}

		std::cout << "\r";

		// Display result
		const int64_t saveFieldsSpeed = std::llround(
			NanosecondsInMs / static_cast<double>(perfTestData.Time.count()) * static_cast<double>(perfTestData.ProcessedFields));
		std::cout << metadata.LibraryName << " | " << benchmark.GetCurrentStageName() << ": " << saveFieldsSpeed << " (fields/ms)" << std::endl;
	}
	while (benchmark.NextStage());

	return metadata;
}

int main()
{
	try
	{
		std::cout << "Testing, please do not touch mouse and keyboard (test may take few minutes)." << std::endl;
		std::vector<TestArchiveResult> benchmarkResults;

#ifdef CSV_BENCHMARK
		benchmarkResults.emplace_back(RunBenchmark(CBitSerializerBenchmark<BitSerializer::Csv::CsvArchive>()));
#endif
#ifdef MSGPACK_BENCHMARK
		benchmarkResults.emplace_back(RunBenchmark(CBitSerializerBenchmark<BitSerializer::MsgPack::MsgPackArchive>()));
#endif
#ifdef RAPIDJSON_BENCHMARK
		benchmarkResults.emplace_back(RunBenchmark(CBitSerializerBenchmark<BitSerializer::Json::RapidJson::JsonArchive>()));
		benchmarkResults.emplace_back(RunBenchmark(CRapidJsonBenchmark()));
#endif
#ifdef PUGIXML_BENCHMARK
		benchmarkResults.emplace_back(RunBenchmark(CBitSerializerBenchmark<BitSerializer::Xml::PugiXml::XmlArchive>()));
		benchmarkResults.emplace_back(RunBenchmark(CPugiXmlBenchmark()));
#endif
#ifdef RAPIDYAMLN_BENCHMARK
		benchmarkResults.emplace_back(RunBenchmark(CBitSerializerBenchmark<BitSerializer::Yaml::RapidYaml::YamlArchive>()));
		benchmarkResults.emplace_back(RunBenchmark(CRapidYamlBenchmark()));
#endif
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
