/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <cassert>
#include <iostream>
#include <chrono>
#include <cmath>
#include "test_helpers/common_test_methods.h"
#include "rapid_json_performance_test.h"
#include "cpprest_json_performance_test.h"
#include "pugixml_performance_test.h"
#include "rapid_yaml_perfomance_test.h"

constexpr auto DefaultArchiveTestTimeSec = 60;

using Timer = std::chrono::high_resolution_clock;

struct TestArchiveMetadata
{
	TestArchiveMetadata() = default;
	explicit TestArchiveMetadata(const char* name)
		: mName(name), mMinSaveTime(0), mMinLoadTime(0), mMinNativeSaveTime(0), mMinNativeLoadTime(0) {}

	const char* mName;
	std::chrono::nanoseconds mMinSaveTime;
	std::chrono::nanoseconds mMinLoadTime;
	std::chrono::nanoseconds mMinNativeSaveTime;
	std::chrono::nanoseconds mMinNativeLoadTime;
};

template <class TArchive, class TPerformanceTestModel, int TestTimeSec = DefaultArchiveTestTimeSec>
TestArchiveMetadata TestArchivePerformance()
{
	auto origModel = BuildFixture<TPerformanceTestModel>();
	TestArchiveMetadata metadata(origModel.GetName());

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
		if (needVerify)
		{
			using NativeOutputType = decltype(origModel.TestSave());
			if constexpr (std::is_same_v<NativeOutputType, typename TArchive::preferred_output_format>) {
				assert(nativeCodeSaveResult == bitSerializerSaveResult);
			}
			else {
				assert(nativeCodeSaveResult == BitSerializer::Convert::To<NativeOutputType>(bitSerializerSaveResult));
			}
		}

		// Load model via native code
		TPerformanceTestModel testModelForNativeCode;
		startTime = Timer::now();
		testModelForNativeCode.TestLoad(nativeCodeSaveResult);
		const auto nativeLoadTime = Timer::now() - startTime;
		if (metadata.mMinNativeLoadTime == std::chrono::nanoseconds(0) || metadata.mMinNativeLoadTime > nativeLoadTime) metadata.mMinNativeLoadTime = nativeLoadTime;

		// Check integrity (for test loader via native code)
		if (needVerify)
		{
			needVerify = false;
			testModelForNativeCode.Assert(origModel);
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
			  TestArchivePerformance<BitSerializer::Json::RapidJson::JsonArchive, RapidJsonPerformanceTestModel>()
			, TestArchivePerformance<BitSerializer::Json::CppRest::JsonArchive, CppRestJsonPerformanceTestModel>()
			, TestArchivePerformance<BitSerializer::Xml::PugiXml::XmlArchive, PugiXmlPerformanceTestModel>()
			, TestArchivePerformance<BitSerializer::Yaml::RapidYaml::YamlArchive, RapidYamlPerformanceTestModel>()
		};
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
