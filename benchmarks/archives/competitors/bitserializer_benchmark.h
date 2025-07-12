/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
// Benchmark headers
#include "benchmark_base.h"

// BitSerializer library
#include "bitserializer/bit_serializer.h"
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
#ifdef RAPIDYAML_BENCHMARK
#include "bitserializer/rapidyaml_archive.h"
#endif


// Suppress C4702 - unreachable code
#pragma warning(push)
#pragma warning(disable: 4702)

/**
 * @brief Serializes the `CBasicTestModel` using BitSerializer.
 *
 * @tparam TArchive The archive type used for serialization.
 * @param archive Reference to the archive object.
 * @param testModel Reference to the model to serialize.
 */
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

/**
 * @brief Benchmark implementation for the BitSerializer library.
 *
 * @tparam TArchive The specific archive type to benchmark.
 */
template <class TArchive>
class CBitSerializerBenchmark final : public CBenchmarkBase
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
	void BenchmarkSaveToMemory(const CCommonTestModel& sourceTestModel, std::string& outputData) override
	{
		BitSerializer::SaveObject<TArchive>(sourceTestModel, outputData);
	}

	void BenchmarkLoadFromMemory(CCommonTestModel& targetTestModel, const std::string& sourceData) override
	{
		BitSerializer::LoadObject<TArchive>(targetTestModel, sourceData);
	}

	void BenchmarkSaveToStream(const CCommonTestModel& sourceTestModel, std::ostream& outputStream) override
	{
		BitSerializer::SaveObject<TArchive>(sourceTestModel, outputStream);
	}

	void BenchmarkLoadFromStream(CCommonTestModel& targetTestModel, std::istream& inputStream) override
	{
		BitSerializer::LoadObject<TArchive>(targetTestModel, inputStream);
	}
};

#pragma warning(pop)
