/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/msgpack_archive.h"
#include "benchmark_base.h"

using MsgPackBasePerfTest = CBenchmarkBase<BitSerializer::MsgPack::MsgPackArchive, CommonTestModel<>, char>;

class MsgPackBenchmark final : public MsgPackBasePerfTest
{
public:
	using base_class_t = MsgPackBasePerfTest;

	void Assert() const override
	{
		for (size_t i = 0; i < mSourceTestModel.size(); i++)
		{
			mSourceTestModel[i].Assert(mBitSerializerModel[i]);
		}
	}
};
