/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>

#ifdef CSV_BENCHMARK
#include "csv_benchmark.h"
#endif
#ifdef MSGPACK_BENCHMARK
#include "msgpack_benchmark.h"
#endif
#ifdef RAPIDJSON_BENCHMARK
#include "rapid_json_benchmark.h"
#endif
// ToDo: remove deprecated JSON archive based on CppRestSdk
//#ifdef CPPRESTJSON_BENCHMARK
//#include "cpprest_json_benchmark.h"
//#endif
#ifdef PUGIXML_BENCHMARK
#include "pugixml_benchmark.h"
#endif
#ifdef RAPIDYAMLN_BENCHMARK
#include "rapid_yaml_benchmark.h"
#endif


constexpr auto DefaultArchiveTestTime = std::chrono::seconds(60);
constexpr auto NanosecondsInMs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1)).count();

using Timer = std::chrono::high_resolution_clock;

struct TestArchiveResult
{
	struct PerfTestData
	{
		std::chrono::nanoseconds Time{};
		size_t ProcessedFields = 0;
	};

	TestArchiveResult() = default;
	explicit TestArchiveResult(std::string name)
		: Name(std::move(name))
	{ }

	std::string Name;
	PerfTestData BitSerializerLoadTest;
	PerfTestData BitSerializerSaveTest;
	PerfTestData BaseLibLoadTest;
	PerfTestData BaseLibSaveTest;
};

template <class TBenchmark>
TestArchiveResult RunBenchmark(const std::chrono::seconds testTime = DefaultArchiveTestTime)
{
	TBenchmark performanceTest;
	performanceTest.Prepare();
	TestArchiveResult metadata(performanceTest.GetArchiveName());

	int progressPercent = -1;

	const auto beginTime = Timer::now();
	const auto endTime = beginTime + std::chrono::seconds(testTime);
	const auto testTimeMSec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(testTime)).count();

	for (auto time = beginTime; time < endTime; time = Timer::now())
	{
		const auto newPercent = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(time - beginTime).count() * 100 / testTimeMSec);
		if (progressPercent != newPercent)
		{
			progressPercent = newPercent;
			std::cout << "\r" << metadata.Name << ": " << progressPercent << "%";
		}

		// Save model via BitSerializer
		auto startTime = Timer::now();
		const auto savedChars = performanceTest.SaveModelViaBitSerializer();
		const auto saveTime = Timer::now() - startTime;
		if (metadata.BitSerializerSaveTest.Time == std::chrono::nanoseconds(0) || metadata.BitSerializerSaveTest.Time > saveTime)
		{
			metadata.BitSerializerSaveTest.Time = saveTime;
			metadata.BitSerializerSaveTest.ProcessedFields = performanceTest.GetTotalFieldsCount();
		}

		// Load model via BitSerializer
		startTime = Timer::now();
		const auto loadedChars = performanceTest.LoadModelViaBitSerializer();
		const auto loadTime = Timer::now() - startTime;
		if (metadata.BitSerializerLoadTest.Time == std::chrono::nanoseconds(0) || metadata.BitSerializerLoadTest.Time > loadTime)
		{
			metadata.BitSerializerLoadTest.Time = loadTime;
			metadata.BitSerializerLoadTest.ProcessedFields = performanceTest.GetTotalFieldsCount();
		}

		// Test serialization via native library
		if (performanceTest.IsUseNativeLib())
		{
			// Save model via native code
			startTime = Timer::now();
			const auto savedNativeLibChars = performanceTest.SaveModelViaNativeLib();
			const auto nativeSaveTime = Timer::now() - startTime;
			if (metadata.BaseLibSaveTest.Time == std::chrono::nanoseconds(0) || metadata.BaseLibSaveTest.Time > nativeSaveTime)
			{
				metadata.BaseLibSaveTest.Time = nativeSaveTime;
				metadata.BaseLibSaveTest.ProcessedFields = performanceTest.GetTotalFieldsCount();
			}

			// Load model via native code
			startTime = Timer::now();
			const auto loadedNativeLibChars = performanceTest.LoadModelViaNativeLib();
			const auto nativeLoadTime = Timer::now() - startTime;
			if (metadata.BaseLibLoadTest.Time == std::chrono::nanoseconds(0) || metadata.BaseLibLoadTest.Time > nativeLoadTime)
			{
				metadata.BaseLibLoadTest.Time = nativeLoadTime;
				metadata.BaseLibLoadTest.ProcessedFields = performanceTest.GetTotalFieldsCount();
			}
		}

		// Check integrity of loaded data
		performanceTest.Assert();
	}

	std::cout << "\r";

	// Display result
	//------------------------------------------------------------------------------
	// Save
	const int64_t saveFieldsSpeed = std::llround(
		NanosecondsInMs / static_cast<double>(metadata.BitSerializerSaveTest.Time.count()) * static_cast<double>(metadata.BitSerializerSaveTest.ProcessedFields));
	std::cout << metadata.Name << " save speed (fields/ms): " << saveFieldsSpeed << " | Base lib: ";
	if (performanceTest.IsUseNativeLib())
	{
		const int64_t saveNativeLibSpeed = std::llround(
			NanosecondsInMs / static_cast<double>(metadata.BaseLibSaveTest.Time.count()) * static_cast<double>(metadata.BaseLibSaveTest.ProcessedFields));
		const auto diffSavePercent = std::round((saveFieldsSpeed / (saveNativeLibSpeed / 100.0) - 100) * 10) / 10;
		std::cout << saveNativeLibSpeed << " | difference: " << diffSavePercent << "%" << std::endl;
	}
	else
	{
		std::cout << "N/A" << std::endl;
	}

	// Load
	const int64_t loadFieldsSpeed = std::llround(
		NanosecondsInMs / static_cast<double>(metadata.BitSerializerLoadTest.Time.count()) * static_cast<double>(metadata.BitSerializerLoadTest.ProcessedFields));
	std::cout << metadata.Name << " load speed (fields/ms): " << loadFieldsSpeed << " | Base lib: ";
	if (performanceTest.IsUseNativeLib())
	{
		const int64_t loadNativeLibSpeed = std::llround(
			NanosecondsInMs / static_cast<double>(metadata.BaseLibLoadTest.Time.count()) * static_cast<double>(metadata.BaseLibLoadTest.ProcessedFields));
		const auto diffLoadPercent = std::round((loadFieldsSpeed / (loadNativeLibSpeed / 100.0) - 100) * 10) / 10;
		std::cout << loadNativeLibSpeed << " | difference: " << diffLoadPercent << "%" << std::endl;
	}
	else
	{
		std::cout << "N/A" << std::endl;
	}

	return metadata;
}

int main()
{
	try
	{
		std::cout << "Testing, please do not touch mouse and keyboard (test may take few minutes)." << std::endl;
		std::vector<TestArchiveResult> benchmarkResults;

#ifdef CSV_BENCHMARK
		benchmarkResults.emplace_back(RunBenchmark<CsvBenchmark>());
#endif
#ifdef MSGPACK_BENCHMARK
		benchmarkResults.emplace_back(RunBenchmark<MsgPackBenchmark>());
#endif
#ifdef RAPIDJSON_BENCHMARK
		benchmarkResults.emplace_back(RunBenchmark<CRapidJsonBenchmark>());
#endif
// ToDo: remove deprecated JSON archive based on CppRestSdk
//#ifdef CPPRESTJSON_BENCHMARK
//		benchmarkResults.emplace_back(RunBenchmark<CCppRestJsonBenchmark>());
//#endif
#ifdef PUGIXML_BENCHMARK
		benchmarkResults.emplace_back(RunBenchmark<CPugiXmlBenchmark>());
#endif
#ifdef RAPIDYAMLN_BENCHMARK
		benchmarkResults.emplace_back(RunBenchmark<CRapidYamlBenchmark>());
#endif
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
