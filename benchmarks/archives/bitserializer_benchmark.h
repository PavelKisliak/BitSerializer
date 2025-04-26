/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "benchmark_base.h"

#ifdef CSV_BENCHMARK
#include "bitserializer/csv_archive.h"
#endif

#ifdef MSGPACK_BENCHMARK
#include "bitserializer/msgpack_archive.h"
#endif

#ifdef RAPIDJSON_BENCHMARK
#include "bitserializer/rapidjson_archive.h"
#endif

#ifdef PUGIXML_BENCHMARK
#include "bitserializer/pugixml_archive.h"
#endif

#ifdef RAPIDYAMLN_BENCHMARK
#include "bitserializer/rapidyaml_archive.h"
#endif

template <class TArchive, class TModel = CommonTestModel>
class CBitSerializerBenchmark : public CBenchmarkBase<TModel>
{
public:
	[[nodiscard]] std::string GetLibraryName() const override
	{
		return "BitSerializer-" + BitSerializer::Convert::ToString(TArchive::archive_type);
	}

protected:
	void BenchmarkSaveToMemory(const TModel& sourceTestModel, std::string& outputData) override
	{
		outputData = BitSerializer::SaveObject<TArchive>(sourceTestModel);
	}

	void BenchmarkLoadFromMemory(const std::string& sourceData, TModel& targetTestModel) override
	{
		BitSerializer::LoadObject<TArchive>(targetTestModel, sourceData);
	}
};
