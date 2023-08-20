/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/csv_archive.h"
#include "benchmark_base.h"

using RapidJsonTestModel = CommonTestModel<>;
using CsvBasePerfTest = CBenchmarkBase<BitSerializer::Csv::CsvArchive, CommonTestModel<>, char>;

class CsvBenchmark final : public CsvBasePerfTest
{
public:
	using base_class_t = CsvBasePerfTest;

	void Assert() const override
	{
		for (size_t i = 0; i < mSourceTestModel.size(); i++)
		{
			mSourceTestModel[i].Assert(mBitSerializerModel[i]);
		}
	}
};
