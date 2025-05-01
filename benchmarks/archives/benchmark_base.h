/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <vector>
#include <sstream>
#include <stdexcept>
#include "test_model.h"

/// <summary>
/// Test stages.
/// </summary>
enum class TestStage
{
	SaveToMemory,
	LoadFromMemory,
	SaveToStream,
	LoadFromStream
};

REGISTER_ENUM(TestStage, {
	{ TestStage::SaveToMemory, "Save to memory" },
	{ TestStage::LoadFromMemory, "Load from memory" },
	{ TestStage::SaveToStream, "Save to std::ostream" },
	{ TestStage::LoadFromStream, "Load from std::istream" }
})

/// <summary>
/// Test stages.
/// </summary>
struct CLibraryTestResults
{
	std::string LibraryName;

	// Common metrics for all stages
	size_t TestModelFieldsCount = 0;
	size_t SerializedDataSize = 0;

	/// Stage test metrics.
	struct CTestMetrics
	{
		int64_t SerializationSpeed = 0;	// Fields/ms
	};

	std::unordered_map<TestStage, CTestMetrics> StagesTestResults;
};


/// <summary>
/// Base class of serialization library benchmark (consist multiple stages, e.g. save/load).
/// </summary>
class CBenchmarkBase
{
public:
	using Timer = std::chrono::high_resolution_clock;

	CBenchmarkBase();
	virtual ~CBenchmarkBase() = default;

	/// <summary>
	/// Returns name of testing library.
	/// </summary>
	[[nodiscard]] virtual std::string GetLibraryName() const = 0;

	/// <summary>
	/// Get a list of supported stages, can be overridden to exclude unsupported.
	/// </summary>
	[[nodiscard]] virtual std::vector<TestStage> GetStagesList() const {
		return { TestStage::SaveToMemory, TestStage::LoadFromMemory, TestStage::SaveToStream, TestStage::LoadFromStream };
	}

	/// <summary>
	/// Running library benchmarks.
	/// </summary>
	CLibraryTestResults RunBenchmark(std::chrono::seconds testTime);

protected:
	// Benchmark calls, serialization must be implemented using the library being tested
	virtual void BenchmarkSaveToMemory(const CCommonTestModel& /*sourceTestModel*/, std::string& /*outputData*/) {
		throw std::runtime_error("Not implemented!");
	}
	virtual void BenchmarkLoadFromMemory(CCommonTestModel& /*targetTestModel*/, const std::string& /*sourceData*/) {
		throw std::runtime_error("Not implemented!");
	}
	virtual void BenchmarkSaveToStream(const CCommonTestModel& /*targetTestModel*/, std::ostream& /*outputStream*/) {
		throw std::runtime_error("Not implemented!");
	}
	virtual void BenchmarkLoadFromStream(CCommonTestModel& /*targetTestModel*/, std::istream& /*inputStream*/) {
		throw std::runtime_error("Not implemented!");
	}

	// Maintenance calls (optional, not counted)
	virtual void OnBeginStage(TestStage /*testStage*/) {}
	virtual void OnPrepareTest(TestStage /*testStage*/) {}
	virtual void OnNextStage(TestStage /*testStage*/) {}
	virtual void OnFinishedStage(TestStage /*testStage*/) {}

private:
	size_t GetTotalFieldsCount() const noexcept;
	void PrepareStage();
	std::string GetCurrentStageName() const;

	// Prepares a test, called before each test run (keep as inline for better performance)
	void PrepareTest()
	{
		assert(mInProgress);
		switch (mTestStage)
		{
		case TestStage::SaveToMemory:
			mSerializedData.clear();
			break;
		case TestStage::LoadFromMemory:
			if (mSerializedData.empty()) {
				throw std::runtime_error("There are no serialized data (you need to do `SaveToMemory` test first)");
			}
			mTargetModel = {};
			break;
		case TestStage::SaveToStream:
			mStringStream = {};
			break;
		case TestStage::LoadFromStream:
			mTargetModel = {};
			// Need to clear the error flags for able to rewind
			mStringStream.clear();
			mStringStream.seekg(0, std::ios::beg);
			if (mStringStream.eof()) {
				throw std::runtime_error("There are no serialized data (you need to do `SaveToStream` test first)");
			}
			break;
		}

		OnPrepareTest(mTestStage);
	}

	// Run one time test (keep as inline for better performance)
	void RunOneTimeTest()
	{
		assert(mInProgress);
		switch (mTestStage)
		{
		case TestStage::SaveToMemory:
			BenchmarkSaveToMemory(mSourceTestModel, mSerializedData);
			break;
		case TestStage::LoadFromMemory:
			BenchmarkLoadFromMemory(mTargetModel, mSerializedData);
			break;
		case TestStage::SaveToStream:
			BenchmarkSaveToStream(mSourceTestModel, mStringStream);
			break;
		case TestStage::LoadFromStream:
			BenchmarkLoadFromStream(mTargetModel, mStringStream);
			break;
		}
	}

	bool NextStage();
	void ValidateTargetModel() const;

	TestStage mTestStage = TestStage::SaveToMemory;
	bool mInProgress = false;
	CCommonTestModel mSourceTestModel;
	CCommonTestModel mTargetModel;
	std::string mSerializedData;
	std::stringstream mStringStream;
};
