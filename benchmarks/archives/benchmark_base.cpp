/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "benchmark_base.h"
#include <cmath>


CBenchmarkBase::CBenchmarkBase()
{
	BuildFixture(mSourceTestModel);
	// Reserve buffers to minimize the impact of memory allocation on test results
	mSerializedData.reserve(16384);
}

size_t CBenchmarkBase::GetTotalFieldsCount() const noexcept
{
	// Total size of all fields (each element of array also counting as field)
	return mSourceTestModel.size() * CCommonTestModel::value_type::GetTotalFieldsCount() + mSourceTestModel.size();
}

CLibraryTestResults CBenchmarkBase::RunBenchmark(const std::chrono::seconds testTime)
{
	constexpr auto NanosecondsInMs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1)).count();

	int progressPercent = -1;
	CLibraryTestResults libTestResults;
	libTestResults.LibraryName = GetLibraryName();
	libTestResults.TestModelFieldsCount = GetTotalFieldsCount();

	do
	{
		const auto beginTime = Timer::now();
		const auto endTime = beginTime + std::chrono::seconds(testTime);
		const auto testTimeMSec = std::chrono::duration_cast<std::chrono::milliseconds>(testTime).count();
		std::chrono::nanoseconds minTime{};

		PrepareStage();
		CLibraryTestResults::CTestMetrics testMetrics;
		for (auto time = beginTime; time < endTime; time = Timer::now())
		{
			// Print progress
			const auto newPercent = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(time - beginTime).count() * 100 / testTimeMSec);
			if (progressPercent != newPercent)
			{
				progressPercent = newPercent;
				std::cout << "\r" << GetLibraryName() << " | " << GetCurrentStageName() << ": " << progressPercent << "%";
			}

			// Run benchmark
			PrepareTest();
			auto startTime = Timer::now();
			RunOneTimeTest();
			const auto testDuration = Timer::now() - startTime;

			// Record serialization time
			if (minTime == std::chrono::nanoseconds(0) || minTime > testDuration)
			{
				minTime = testDuration;
			}

			// Record serialized data size
			if (mTestStage == TestStage::SaveToMemory && libTestResults.SerializedDataSize != mSerializedData.size())
			{
				if (libTestResults.SerializedDataSize == 0)
				{
					libTestResults.SerializedDataSize = mSerializedData.size();
				}
				else
				{
					throw std::runtime_error("The size of serialized data varies between multiple test runs!");
				}
			}
		}
		std::cout << "\r";

		// Calculate serialization speed (fields/ms)
		testMetrics.SerializationSpeed = std::llround(
			NanosecondsInMs / static_cast<double>(minTime.count()) * static_cast<double>(libTestResults.TestModelFieldsCount));
		libTestResults.StagesTestResults.emplace(mTestStage, testMetrics);

		// Display result
		std::cout << GetLibraryName() << " | " << GetCurrentStageName() << ": " << testMetrics.SerializationSpeed << " (fields/ms)" << std::endl;

	} while (NextStage());

	return libTestResults;
}

void CBenchmarkBase::PrepareStage()
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

std::string CBenchmarkBase::GetCurrentStageName() const
{
	return BitSerializer::Convert::ToString(mTestStage);
}

bool CBenchmarkBase::NextStage()
{
	assert(mInProgress);

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

void CBenchmarkBase::ValidateTargetModel() const
{
	// Compare loaded model with original
	size_t targetIdx = 0;
	for (const auto& sourceTestModel : mSourceTestModel)
	{
		sourceTestModel.Assert(mTargetModel[targetIdx]);
		++targetIdx;
	}
}
