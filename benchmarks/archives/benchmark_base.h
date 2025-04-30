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
/// Base class of serialization library benchmark (consist multiple stages, e.g. save/load).
/// </summary>
template <class TModel = CommonTestModel>
class CBenchmarkBase
{
public:
	CBenchmarkBase()
	{
		BuildFixture(mSourceTestModel);
	}

	virtual ~CBenchmarkBase() = default;

	/// <summary>
	/// Returns name of testing library.
	/// </summary>
	[[nodiscard]] virtual std::string GetLibraryName() const = 0;

	/// <summary>
	/// Get a list of supported stages, can be overridden to exclude unsupported.
	/// </summary>
	[[nodiscard]] virtual std::vector<TestStage> GetStagesList() const
	{
		return { TestStage::SaveToMemory, TestStage::LoadFromMemory };
	}

	/// <summary>
	/// Prepare stage.
	/// </summary>
	void PrepareStage()
	{
		if (!mInProgress)
		{
			// Initialize first stage
			std::vector<TestStage> stagesList = GetStagesList();
			assert(!stagesList.empty());
			mTestStage = stagesList.front();
			mInProgress = true;
		}

		OnBeginStage(mTestStage);
	}

	/// <summary>
	/// Get current stage name.
	/// </summary>
	[[nodiscard]] std::string GetCurrentStageName() const
	{
		return BitSerializer::Convert::ToString(mTestStage);
	}

	/// <summary>
	/// Returns the number of fields in the model.
	/// </summary>
	size_t GetTotalFieldsCount() noexcept
	{
		if constexpr (BitSerializer::has_size_v<TModel>)
		{
			assert(!mSourceTestModel.empty());
			// Total size of all fields (each element of array also counting as field)
			return mSourceTestModel.size() * TModel::value_type::GetTotalFieldsCount() + mSourceTestModel.size();
		}
		else {
			return TModel::GetTotalFieldsCount();
		}
	}

	/// <summary>
	/// Prepare test (clean-up model, etc.).
	/// </summary>
	void PrepareTest()
	{
		assert(mInProgress);
		if (!mInProgress) {
			return;
		}

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

	/// <summary>
	/// Runs benchmark.
	/// </summary>
	void Run()
	{
		assert(mInProgress);
		if (!mInProgress) {
			return;
		}

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

	/// <summary>
	/// Go to next stage.
	/// </summary>
	bool NextStage()
	{
		assert(mInProgress);
		if (!mInProgress) {
			return false;
		}

		// Validate target model after each load stage
		if (mTestStage == TestStage::LoadFromMemory || mTestStage == TestStage::LoadFromStream)
		{
			ValidateTargetModel();
		}

		OnFinishedStage(mTestStage);

		// Find next tage
		std::vector<TestStage> stagesList = GetStagesList();
		auto currentStageIt = std::find(stagesList.cbegin(), stagesList.cend(), mTestStage);
		assert(currentStageIt != stagesList.cend());

		if (++currentStageIt != stagesList.cend())
		{
			mTestStage = *currentStageIt;
			OnNextStage(mTestStage);
			return true;
		}

		// When all stages completed
		mInProgress = false;
		return false;
	}

protected:
	// Benchmark calls, serialization must be implemented using the library being tested
	virtual void BenchmarkSaveToMemory(const TModel& /*sourceTestModel*/, std::string& /*outputData*/) {
		throw std::runtime_error("Not implemented!");
	}
	virtual void BenchmarkLoadFromMemory(TModel& /*targetTestModel*/, const std::string& /*sourceData*/) {
		throw std::runtime_error("Not implemented!");
	}
	virtual void BenchmarkSaveToStream(const TModel& /*targetTestModel*/, std::ostream& /*outputStream*/) {
		throw std::runtime_error("Not implemented!");
	}
	virtual void BenchmarkLoadFromStream(TModel& /*targetTestModel*/, std::istream& /*inputStream*/) {
		throw std::runtime_error("Not implemented!");
	}

	// Maintenance calls (not counted)
	virtual void OnBeginStage(TestStage /*testStage*/) {}
	virtual void OnPrepareTest(TestStage /*testStage*/) {}
	virtual void OnNextStage(TestStage /*testStage*/) {}
	virtual void OnFinishedStage(TestStage /*testStage*/) {}

private:
	void ValidateTargetModel()
	{
		// Compare loaded model with original
		if constexpr (BitSerializer::is_enumerable_v<TModel>)
		{
			// Sizes can't be different since used std::array
			assert(mSourceTestModel.size() == mTargetModel.size());
			auto targetIt = mTargetModel.cbegin();
			for (const auto& sourceTestModel : mSourceTestModel)
			{
				sourceTestModel.Assert(*targetIt);
				++targetIt;
			}
		}
		else if constexpr (has_assert_method_v<TModel>)
		{
			mSourceTestModel.Assert(mTargetModel);
		}
	}

	TestStage mTestStage = TestStage::SaveToMemory;
	bool mInProgress = false;
	TModel mSourceTestModel;
	TModel mTargetModel;
	std::string mSerializedData;
	std::stringstream mStringStream;
};
