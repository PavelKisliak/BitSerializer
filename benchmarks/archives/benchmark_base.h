/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <vector>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <chrono>
#include "test_model.h"

/**
 * @brief Represents different stages of a serialization benchmark.
 */
enum class TestStage
{
	SaveToMemory,	///< Serialize object model to memory.
	LoadFromMemory,	///< Deserialize object model from memory.
	SaveToStream,	///< Serialize object model to an output stream.
	LoadFromStream	///< Deserialize object model from an input stream.
};

REGISTER_ENUM(TestStage, {
	{ TestStage::SaveToMemory, "Save to memory" },
	{ TestStage::LoadFromMemory, "Load from memory" },
	{ TestStage::SaveToStream, "Save to std::ostream" },
	{ TestStage::LoadFromStream, "Load from std::istream" }
})

/**
 * @brief Stores test results for a serialization library.
 */
struct CLibraryTestResults
{
	/// Name of the tested serialization library.
	std::string LibraryName;

	/// Total number of fields in the test model.
	size_t TestModelFieldsCount = 0;

	/// Size of serialized data in bytes.
	size_t SerializedDataSize = 0;

	/**
	 * @brief Metrics collected per test stage.
	 */
	struct CTestMetrics
	{
		int64_t SerializationSpeed = 0; ///< Number of fields processed per millisecond.
	};

	/// Mapping of test stages to their respective metrics.
	std::unordered_map<TestStage, CTestMetrics> StagesTestResults;
};


/**
 * @brief Base class for benchmarking serialization libraries.
 *
 * Provides a framework for measuring performance across multiple stages including:
 * - Save to memory
 * - Load from memory
 * - Save to stream
 * - Load from stream
 *
 * Derived classes must implement the actual serialization logic using the target library.
 */
class CBenchmarkBase
{
public:
	using Timer = std::chrono::high_resolution_clock;

	CBenchmarkBase();
	virtual ~CBenchmarkBase() = default;

	/**
	 * @brief Gets the name of the serialization library being tested.
	 *
	 * @return The library name as a string.
	 */
	[[nodiscard]] virtual std::string GetLibraryName() const = 0;

	/**
	 * @brief Gets the list of supported test stages.
	 *
	 * Override this method to exclude unsupported stages.
	 *
	 * @return A vector containing the supported test stages.
	 */
	[[nodiscard]] virtual std::vector<TestStage> GetStagesList() const {
		return { TestStage::SaveToMemory, TestStage::LoadFromMemory,
				 TestStage::SaveToStream, TestStage::LoadFromStream };
	}

	/**
	 * @brief Runs the benchmark for the supported stages.
	 *
	 * @param testTime Duration of each stage in seconds.
	 * @return A structure containing the collected test results.
	 */
	CLibraryTestResults RunBenchmark(std::chrono::seconds testTime);

protected:
	// Benchmark implementation methods — derived classes should override these

	/**
	 * @brief Serializes the test model into a memory buffer.
	 * @param sourceTestModel Reference to the source model.
	 * @param outputData Output buffer for serialized data.
	 */
	virtual void BenchmarkSaveToMemory(const CCommonTestModel& /*sourceTestModel*/, std::string& /*outputData*/) {
		throw std::runtime_error("Benchmark SaveToMemory is not implemented!");
	}

	/**
	 * @brief Deserializes the test model from a memory buffer.
	 * @param targetTestModel Reference to the target model.
	 * @param sourceData Input buffer with serialized data.
	 */
	virtual void BenchmarkLoadFromMemory(CCommonTestModel& /*targetTestModel*/, const std::string& /*sourceData*/) {
		throw std::runtime_error("Benchmark LoadFromMemory is not implemented!");
	}

	/**
	 * @brief Serializes the test model into an output stream.
	 * @param targetTestModel Reference to the source model.
	 * @param outputStream Output stream to write to.
	 */
	virtual void BenchmarkSaveToStream(const CCommonTestModel& /*targetTestModel*/, std::ostream& /*outputStream*/) {
		throw std::runtime_error("Benchmark SaveToStream is not implemented!");
	}

	/**
	 * @brief Deserializes the test model from an input stream.
	 * @param targetTestModel Reference to the target model.
	 * @param inputStream Input stream with serialized data.
	 */
	virtual void BenchmarkLoadFromStream(CCommonTestModel& /*targetTestModel*/, std::istream& /*inputStream*/) {
		throw std::runtime_error("Benchmark LoadFromStream is not implemented!");
	}

	// Optional lifecycle hooks (not measured)

	/**
	 * @brief Called before starting a new test stage.
	 * @param testStage Completed test stage.
	 */
	virtual void OnBeginStage(TestStage /*testStage*/) {}

	/**
	 * @brief Called before executing a single test run.
	 */
	virtual void OnPrepareTest(TestStage /*testStage*/) {}

	/**
	 * @brief Called after completing a test stage.
	 * @param testStage Completed test stage.
	 */
	virtual void OnNextStage(TestStage /*testStage*/) {}

	/**
	 * @brief Called after finishing all test stages.
	 * @param testStage Final test stage.
	 */
	virtual void OnFinishedStage(TestStage /*testStage*/) {}

private:
	size_t GetTotalFieldsCount() const noexcept;
	void PrepareStage();
	std::string GetCurrentStageName() const;

	/**
	 * @brief Prepares the environment for the current test stage.
	 */
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
				throw std::runtime_error("No serialized data available. Perform 'SaveToMemory' test first.");
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
				throw std::runtime_error("No serialized data available. Perform 'SaveToStream' test first.");
			}
			break;
		}

		OnPrepareTest(mTestStage);
	}

	/**
	 * @brief Executes a single iteration of the current test stage.
	 */
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
