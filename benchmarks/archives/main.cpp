/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <iostream>
#include <chrono>
#include <cmath>
#include "testing_tools/common_test_methods.h"
#include "rapid_json_performance_test.h"
#include "cpprest_json_performance_test.h"
#include "pugixml_performance_test.h"
#include "rapid_yaml_performance_test.h"
#include "csv_performance_test.h"

constexpr auto DefaultArchiveTestTimeSec = 30;
constexpr size_t NanosecondsInMs = 1000000;

using Timer = std::chrono::high_resolution_clock;

struct TestArchiveMetadata
{
	struct PerfTestData
	{
		std::chrono::nanoseconds Time{};
		size_t ProcessedFields = 0;
	};

	TestArchiveMetadata() = default;
	explicit TestArchiveMetadata(std::string name)
		: Name(std::move(name))
	{ }

	std::string Name;
	PerfTestData BitSerializerLoadTest;
	PerfTestData BitSerializerSaveTest;
	PerfTestData BaseLibLoadTest;
	PerfTestData BaseLibSaveTest;
};

template <class TPerformanceTest, int TestTimeSec = DefaultArchiveTestTimeSec>
TestArchiveMetadata TestArchivePerformance()
{
	TPerformanceTest performanceTest;
	performanceTest.Prepare();
	TestArchiveMetadata metadata(performanceTest.GetArchiveName());

	int progressPercent = -1;
	bool needVerify = true;

	const auto beginTime = Timer::now();
	const auto endTime = beginTime + std::chrono::seconds(TestTimeSec);
	const auto testTimeMSec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(TestTimeSec)).count();

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

		// Check integrity (for test serialization via native library)
		if (needVerify)
		{
			needVerify = false;
			performanceTest.Assert();
		}
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
		const TestArchiveMetadata metadataList[] = {
			  TestArchivePerformance<CsvPerformanceTestModel>()
			, TestArchivePerformance<CRapidJsonPerformanceTest>()
			, TestArchivePerformance<CCppRestJsonPerformanceTest>()
			, TestArchivePerformance<CPugiXmlPerformanceTest>()
			, TestArchivePerformance<CRapidYamlPerformanceTest>()
		};
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
