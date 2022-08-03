/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <iostream>
#include <chrono>
#include <cmath>
#include "test_helpers/common_test_methods.h"
#include "rapid_json_performance_test.h"
#include "cpprest_json_performance_test.h"
#include "pugixml_performance_test.h"
#include "rapid_yaml_performance_test.h"

constexpr auto DefaultArchiveTestTimeSec = 60;

using Timer = std::chrono::high_resolution_clock;

struct TestArchiveMetadata
{
	TestArchiveMetadata() = default;
	explicit TestArchiveMetadata(std::string name)
		: mName(std::move(name)), mMinSaveTime(0), mMinLoadTime(0), mMinNativeSaveTime(0), mMinNativeLoadTime(0) {}

	std::string mName;
	std::chrono::nanoseconds mMinSaveTime;
	std::chrono::nanoseconds mMinLoadTime;
	std::chrono::nanoseconds mMinNativeSaveTime;
	std::chrono::nanoseconds mMinNativeLoadTime;
};

template <class TPerformanceTest, int TestTimeSec = DefaultArchiveTestTimeSec>
TestArchiveMetadata TestArchivePerformance()
{
	TPerformanceTest performanceTest;
	performanceTest.Prepare();
	TestArchiveMetadata metadata(performanceTest.GetArchiveName());

	int percent = -1;
	bool needVerify = true;

	const auto beginTime = Timer::now();
	const auto endTime = beginTime + std::chrono::seconds(TestTimeSec);
	const auto testTimeMSec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(TestTimeSec)).count();

	for (auto time = beginTime; time < endTime; time = Timer::now())
	{
		const auto newPercent = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(time - beginTime).count() * 100 / testTimeMSec);
		if (percent != newPercent)
		{
			percent = newPercent;
			std::cout << "\r" << metadata.mName << ": " << percent << "%";
		}

		// Save model via BitSerializer
		auto startTime = Timer::now();
		performanceTest.SaveModelViaBitSerializer();
		const auto saveTime = Timer::now() - startTime;
		if (metadata.mMinSaveTime == std::chrono::nanoseconds(0) || metadata.mMinSaveTime > saveTime) metadata.mMinSaveTime = saveTime;

		// Load model via BitSerializer
		startTime = Timer::now();
		performanceTest.LoadModelViaBitSerializer();
		const auto loadTime = Timer::now() - startTime;
		if (metadata.mMinLoadTime == std::chrono::nanoseconds(0) || metadata.mMinLoadTime > loadTime) metadata.mMinLoadTime = loadTime;

		// Test serialization via native library
		if (performanceTest.IsUseNativeLib())
		{
			// Save model via native code
			startTime = Timer::now();
			performanceTest.SaveModelViaNativeLib();
			const auto nativeSaveTime = Timer::now() - startTime;
			if (metadata.mMinNativeSaveTime == std::chrono::nanoseconds(0) || metadata.mMinNativeSaveTime > nativeSaveTime) metadata.mMinNativeSaveTime = nativeSaveTime;

			// Load model via native code
			startTime = Timer::now();
			performanceTest.LoadModelViaNativeLib();
			const auto nativeLoadTime = Timer::now() - startTime;
			if (metadata.mMinNativeLoadTime == std::chrono::nanoseconds(0) || metadata.mMinNativeLoadTime > nativeLoadTime) metadata.mMinNativeLoadTime = nativeLoadTime;
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
	const auto minSaveTimeMs = std::chrono::round<std::chrono::microseconds>(metadata.mMinSaveTime).count();
	const auto minNativeSaveTimeMs = std::chrono::round<std::chrono::microseconds>(metadata.mMinNativeSaveTime).count();
	const auto diffSavePercent = std::round((minNativeSaveTimeMs / (minSaveTimeMs / 100.0) - 100) * 10) / 10;
	std::cout << metadata.mName << " save time: " << minNativeSaveTimeMs << " BitSerializer: " << minSaveTimeMs
		<< " - difference " << minSaveTimeMs - minNativeSaveTimeMs << " (" << diffSavePercent << "%)" << std::endl;

	const auto minLoadTimeMs = std::chrono::round<std::chrono::microseconds>(metadata.mMinLoadTime).count();
	const auto minNativeLoadTimeMs = std::chrono::round<std::chrono::microseconds>(metadata.mMinNativeLoadTime).count();
	const auto diffLoadPercent = std::round((minNativeLoadTimeMs / (minLoadTimeMs / 100.0) - 100) * 10) / 10;
	std::cout << metadata.mName << " load time: " << minNativeLoadTimeMs << " BitSerializer: " << minLoadTimeMs
		<< " - difference " << minLoadTimeMs - minNativeLoadTimeMs << " (" << diffLoadPercent << "%)" << std::endl;

	return metadata;
}

int main()
{
	try
	{
		std::cout << "Testing, please do not touch mouse and keyboard (test may take few minutes)." << std::endl;
		const TestArchiveMetadata metadataList[] = {
			  TestArchivePerformance<CRapidJsonPerformanceTest>()
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
