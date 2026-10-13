/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "benchmark_base.h"
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <iostream>

#if defined(_WIN32)
	#include <io.h>
#else
	#include <unistd.h>
#endif

#include "testing_tools/auto_fixture/std/array.h"


namespace
{
	/**
	 * @brief Determines whether the standard output is attached to a console.
	 *
	 * Used to hide the live progress updates when the output is redirected to a file or pipe.
	 */
	bool IsConsoleOutput() noexcept
	{
#if defined(_WIN32)
		return _isatty(_fileno(stdout)) != 0;
#else
		return ::isatty(STDOUT_FILENO) != 0;
#endif
	}
}


BenchmarkBase::BenchmarkBase()
{
	BuildFixture(mSourceTestModel);
	// Reserve buffers to minimize the impact of memory allocation on test results
	mSerializedData.reserve(16384);
}

size_t BenchmarkBase::GetTotalFieldsCount() const noexcept
{
	// Total size of all fields (each element of array also counting as field)
	return mSourceTestModel.size() * CommonTestModel::value_type::GetTotalFieldsCount() + mSourceTestModel.size();
}

LibraryTestResults BenchmarkBase::RunBenchmark(const std::chrono::seconds testTime)
{
	constexpr auto nanosecondsInMs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1)).count();

	const bool isConsoleOutput = IsConsoleOutput();
	int progressPercent = -1;
	LibraryTestResults libTestResults;
	libTestResults.libraryName = GetLibraryName();
	libTestResults.testModelFieldsCount = GetTotalFieldsCount();

	do
	{
		const auto beginTime = Timer::now();
		const auto endTime = beginTime + std::chrono::seconds(testTime);
		const auto testTimeMSec = std::chrono::duration_cast<std::chrono::milliseconds>(testTime).count();
		std::chrono::nanoseconds minTime{};

		PrepareStage();
		LibraryTestResults::TestMetrics testMetrics;
		for (auto time = beginTime; time < endTime; time = Timer::now())
		{
			// Print progress (only when the output is a console)
			const auto newPercent = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(time - beginTime).count() * 100 / testTimeMSec);
			if (isConsoleOutput && progressPercent != newPercent)
			{
				progressPercent = newPercent;
				std::cout << "\r" << GetLibraryName() << " | " << GetCurrentStageName() << ": " << progressPercent << "%";
			}

			// Run benchmark
			PrepareTest();
			auto startTime = Timer::now();
			RunOneTimeTest(mTestStage);
			const auto testDuration = Timer::now() - startTime;

			// Record serialization time
			if (minTime == std::chrono::nanoseconds(0) || minTime > testDuration)
			{
				minTime = testDuration;
			}

			// Record serialized data size
			if (mTestStage == TestStage::SaveToMemory && libTestResults.serializedDataSize != mSerializedData.size())
			{
				if (libTestResults.serializedDataSize == 0)
				{
					libTestResults.serializedDataSize = mSerializedData.size();
				}
				else
				{
					throw std::runtime_error("The size of serialized data varies between multiple test runs!");
				}
			}
		}
		if (isConsoleOutput)
		{
			std::cout << "\r";
		}

		// Calculate serialization speed (fields/ms)
		testMetrics.serializationSpeed = std::llround(
			nanosecondsInMs / static_cast<double>(minTime.count()) * static_cast<double>(libTestResults.testModelFieldsCount));
		libTestResults.stagesTestResults.emplace(mTestStage, testMetrics);

		// Display result
		std::cout << GetLibraryName() << " | " << GetCurrentStageName() << ": " << testMetrics.serializationSpeed << " (fields/ms)" << std::endl;

	} while (NextStage());

	return libTestResults;
}

void BenchmarkBase::PrepareStage()
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

std::string BenchmarkBase::GetCurrentStageName() const
{
	return BitSerializer::Convert::ToString(mTestStage);
}

bool BenchmarkBase::NextStage()
{
	assert(mInProgress);

	// Validate target model after each load stage
	if (mTestStage == TestStage::LoadFromMemory || mTestStage == TestStage::LoadFromStream)
	{
		ValidateTargetModel();
	}

	OnFinishedStage(mTestStage);

	// Find next stage
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

void BenchmarkBase::ValidateTargetModel() const
{
	// Compare loaded model with original
	size_t targetIdx = 0;
	for (const auto& sourceTestModel : mSourceTestModel)
	{
		sourceTestModel.Assert(mTargetModel[targetIdx]);
		++targetIdx;
	}
}
