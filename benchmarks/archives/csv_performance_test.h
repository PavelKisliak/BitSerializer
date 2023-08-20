/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/csv_archive.h"
#include "bitserializer/types/std/array.h"
#include "base_perf_test.h"
#include "base_test_models.h"


using CsvTestModel = std::array<TestModelWithBasicTypes<char>, TestArraySize>;
using CsvBasePerfTest = CArchiveBasePerfTest<BitSerializer::Csv::CsvArchive, CsvTestModel, char>;

class CsvPerformanceTestModel final : public CsvBasePerfTest
{
public:
	using model_t = CsvTestModel;
	using base_class_t = CsvBasePerfTest;

	void Assert() const override
	{
		for (size_t i = 0; i < TestArraySize; i++)
		{
			mSourceTestModel[i].Assert(mBitSerializerModel[i]);
		}
	}
};
