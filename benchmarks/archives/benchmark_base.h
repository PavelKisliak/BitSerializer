/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <vector>
#include "test_model.h"

/// <summary>
/// Test stages.
/// </summary>
enum class TestStage
{
	SaveToMemory,
	LoadFromMemory
};

REGISTER_ENUM(TestStage, {
	{ TestStage::SaveToMemory, "Save to memory" },
	{ TestStage::LoadFromMemory, "Load from memory" }
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
	/// Performs test.
	/// </summary>
	void PerformTest()
	{
		switch (mTestStage)
		{
		case TestStage::SaveToMemory:
			mSerializedData.clear();
			BenchmarkSaveToMemory(mSourceTestModel, mSerializedData);
			break;
		case TestStage::LoadFromMemory:
			BenchmarkLoadFromMemory(mSerializedData, mTargetModel);
			break;
		}
	}

	/// <summary>
	/// Go to next stage.
	/// </summary>
	bool NextStage()
	{
		OnFinishedStage(mTestStage);

		std::vector<TestStage> stagesList = GetStagesList();
		auto currentStageIt = std::find(stagesList.cbegin(), stagesList.cend(), mTestStage);
		assert(currentStageIt != stagesList.cend());

		if (++currentStageIt != stagesList.cend())
		{
			mTestStage = *currentStageIt;
			OnNextStage(mTestStage);
			return true;
		}
		return false;
	}

protected:
	// Needs to be overridden with benchmark implementation
	virtual void BenchmarkSaveToMemory(const TModel& SourceTestModel, std::string& outputData) = 0;
	virtual void BenchmarkLoadFromMemory(const std::string& sourceData, TModel& TargetTestModel) = 0;

	virtual void OnFinishedStage(TestStage testStage)
	{
		if (testStage == TestStage::LoadFromMemory)
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
	}

	virtual void OnNextStage(TestStage) {}

private:
	TestStage mTestStage = TestStage::SaveToMemory;
	TModel mSourceTestModel;
	TModel mTargetModel;
	std::string mSerializedData;
};
