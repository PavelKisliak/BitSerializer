/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "benchmark_base.h"
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidyaml_archive.h"
#include "bitserializer/types/std/array.h"

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

// Suppress C4702 - unreachable code
#pragma warning(push)
#pragma warning(disable: 4702)

/// <summary>
/// Serializes `CBasicTestModel` using BitSerializer.
/// </summary>
template<typename TArchive>
void SerializeObject(TArchive& archive, CBasicTestModel& testModel)
{
	archive << BitSerializer::KeyValue("BooleanValue", testModel.BooleanValue);
	archive << BitSerializer::KeyValue("SignedIntValue", testModel.SignedIntValue);
	archive << BitSerializer::KeyValue("UnsignedIntValue", testModel.UnsignedIntValue);
	archive << BitSerializer::KeyValue("FloatValue", testModel.FloatValue);
	archive << BitSerializer::KeyValue("DoubleValue", testModel.DoubleValue);
	archive << BitSerializer::KeyValue("ShortString", testModel.ShortString);
	archive << BitSerializer::KeyValue("StringWithLongKeyAndValue", testModel.StringWithLongKeyAndValue);
	archive << BitSerializer::KeyValue("UnicodeString", testModel.UnicodeString);
	archive << BitSerializer::KeyValue("StringWithEscapedChars", testModel.StringWithEscapedChars);
	archive << BitSerializer::KeyValue("MultiLineString", testModel.MultiLineString);
}

/// <summary>
/// BitSerializer benchmark.
/// </summary>
template <class TArchive, class TModel = CommonTestModel>
class CBitSerializerBenchmark : public CBenchmarkBase<TModel>
{
public:
	[[nodiscard]] std::string GetLibraryName() const override
	{
#ifdef RAPIDJSON_BENCHMARK
		if constexpr (std::is_same_v<TArchive, BitSerializer::Json::RapidJson::JsonArchive>) {
			return "BitSerializer-RapidJson";
		}
#endif
#ifdef PUGIXML_BENCHMARK
		if constexpr (std::is_same_v<TArchive, BitSerializer::Xml::PugiXml::XmlArchive>) {
			return "BitSerializer-PugiXml";
		}
#endif
#ifdef RAPIDYAML_BENCHMARK
		if constexpr (std::is_same_v<TArchive, BitSerializer::Yaml::RapidYaml::YamlArchive>) {
			return "BitSerializer-RapidYaml";
		}
#endif
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

#pragma warning(pop)
