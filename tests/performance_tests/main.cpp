/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/

// Define for suppress warning STL4015 : The std::iterator class template (used as a base class to provide typedefs) is deprecated in C++17.
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include <cassert>
#include <iostream>
#include <chrono>
#include <math.h>
#include "../test_helpers/common_test_methods.h"
#include "rapid_json_performance_test.h"
#include "cpprest_json_performance_test.h"

static const int TestCycles = 20000;
using Timer = std::chrono::high_resolution_clock;

struct TestArchiveMetadata
{
	TestArchiveMetadata() = default;
	TestArchiveMetadata(const char* name) : mName(name), mMinSaveTime(0), mMinLoadTime(0), mMinNativeSaveTime(0), mMinNativeLoadTime(0) {}

	const char* mName;
	std::chrono::nanoseconds mMinSaveTime;
	std::chrono::nanoseconds mMinLoadTime;
	std::chrono::nanoseconds mMinNativeSaveTime;
	std::chrono::nanoseconds mMinNativeLoadTime;
};

template <class TArchive, class TPerformanceTestModel>
TestArchiveMetadata TestArchivePerformance()
{
	auto origModel = BuildFixture<TPerformanceTestModel>();
	TestArchiveMetadata metadata(origModel.GetName());

	int percent = -1;
	for (int i = 0; i < TestCycles; i++)
	{
		const int newPercent = static_cast<int>(i / (TestCycles * 0.01));
		if (percent != newPercent)
		{
			percent = newPercent;
			std::cout << "\r" << metadata.mName << ": " << percent << "%";
		}

		// Save model via BitSerializer
		auto startTime = Timer::now();
		const auto bitSerializerSaveResult = BitSerializer::SaveObject<TArchive>(origModel);
		const auto saveTime = Timer::now() - startTime;
		if (metadata.mMinSaveTime == std::chrono::nanoseconds(0) || metadata.mMinSaveTime > saveTime) metadata.mMinSaveTime = saveTime;

		// Load model via BitSerializer
		TPerformanceTestModel testModelForBitSerializer;
		startTime = Timer::now();
		BitSerializer::LoadObject<TArchive>(testModelForBitSerializer, bitSerializerSaveResult);
		const auto loadTime = Timer::now() - startTime;
		if (metadata.mMinLoadTime == std::chrono::nanoseconds(0) || metadata.mMinLoadTime > loadTime) metadata.mMinLoadTime = loadTime;

		// Save model via native code
		startTime = Timer::now();
		auto nativeCodeSaveResult = origModel.TestSave();
		const auto nativeSaveTime = Timer::now() - startTime;
		if (metadata.mMinNativeSaveTime == std::chrono::nanoseconds(0) || metadata.mMinNativeSaveTime > nativeSaveTime) metadata.mMinNativeSaveTime = nativeSaveTime;

		// Check an identity of two results (for test saver via native code)
		if (i == 0)
			assert(nativeCodeSaveResult == bitSerializerSaveResult);

		// Load model via native code
		TPerformanceTestModel testModelForNativeCode;
		startTime = Timer::now();
		testModelForNativeCode.TestLoad(nativeCodeSaveResult);
		const auto nativeLoadTime = Timer::now() - startTime;
		if (metadata.mMinNativeLoadTime == std::chrono::nanoseconds(0) || metadata.mMinNativeLoadTime > nativeLoadTime) metadata.mMinNativeLoadTime = nativeLoadTime;

		// Check integrity (for test loader via native code)
		if (i == 0)
			testModelForNativeCode.Assert(origModel);
	}

	std::cout << "\r";

	// Display result
	const auto minSaveTimeMs = std::chrono::duration_cast<std::chrono::microseconds>(metadata.mMinSaveTime).count();
	const auto minNativeSaveTimeMs = std::chrono::duration_cast<std::chrono::microseconds>(metadata.mMinNativeSaveTime).count();
	std::cout << metadata.mName << " save time: " << minNativeSaveTimeMs << " BitSerializer: " << minSaveTimeMs <<
		" - difference " << minSaveTimeMs - minNativeSaveTimeMs << " (" << std::round((minNativeSaveTimeMs / (minSaveTimeMs / 100.0) - 100) * 10) / 10 << "%)" << std::endl;

	const auto minLoadTimeMs = std::chrono::duration_cast<std::chrono::microseconds>(metadata.mMinLoadTime).count();
	const auto minNativeLoadTimeMs = std::chrono::duration_cast<std::chrono::microseconds>(metadata.mMinNativeLoadTime).count();
	std::cout << metadata.mName << " load time: " << minNativeLoadTimeMs << " BitSerializer: " << minLoadTimeMs <<
		" - difference " << minLoadTimeMs - minNativeLoadTimeMs << " (" << std::round((minNativeLoadTimeMs / (minLoadTimeMs / 100.0) - 100) * 10) / 10 << "%)" << std::endl;

	return metadata;
}

int main()
{
	std::cout << "Testing, please do not touch mouse and keyboard (test may take few minutes)." << std::endl;

	const TestArchiveMetadata metadataList[] = {
		TestArchivePerformance<BitSerializer::Json::RapidJson::JsonArchive, RapidJsonPerformanceTestModel>(),
		TestArchivePerformance<BitSerializer::Json::CppRest::JsonArchive, CppRestJsonPerformanceTestModel>()
	};
}
