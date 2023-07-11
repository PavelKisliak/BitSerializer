/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <array>
#include <stdexcept>
#include "bitserializer/csv_archive.h"
#include "bitserializer/types/std/vector.h"
#include "base_test_models.h"


using CsvTestModel = std::vector<TestModelWithBasicTypes<char>>;
using CsvBasePerfTest = CArchiveBasePerfTest<BitSerializer::Csv::CsvArchive, CsvTestModel, char>;

class CsvPerformanceTestModel final : public CsvBasePerfTest
{
public:
	using model_t = CsvTestModel;
	using base_class_t = CsvBasePerfTest;

	static constexpr size_t CsvRowsCount = 30;

	void Prepare() override
	{
		mSourceTestModel.resize(CsvRowsCount);
		for (auto& item : mSourceTestModel)
		{
			BuildFixture(item);
		}
	}

	void Assert() const override
	{
		assert(mSourceTestModel.size() == mBitSerializerModel.size());
		for (size_t i = 0; i < CsvRowsCount; i++)
		{
			mSourceTestModel[i].Assert(mBitSerializerModel[i]);
		}
	}
};
