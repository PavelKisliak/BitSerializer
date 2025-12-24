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
	CBitSerializerBenchmark()
	{
		if constexpr (BitSerializer::is_archive_support_output_data_type_v<typename TArchive::output_archive_type, std::string>)
		{
			mSupportedStagesList.push_back(TestStage::SaveToMemory);
		}
		if constexpr (BitSerializer::is_archive_support_input_data_type_v<typename TArchive::input_archive_type, std::string_view>)
		{
			mSupportedStagesList.push_back(TestStage::LoadFromMemory);
		}

		if constexpr (BitSerializer::is_archive_support_output_data_type_v<typename TArchive::output_archive_type, std::ostream>)
		{
			mSupportedStagesList.push_back(TestStage::SaveToStream);
		}
		if constexpr (BitSerializer::is_archive_support_input_data_type_v<typename TArchive::input_archive_type, std::istream>)
		{
			mSupportedStagesList.push_back(TestStage::LoadFromStream);
		}
	}

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

	std::vector<TestStage> GetStagesList() const override
	{
		return mSupportedStagesList;
	}

protected:
	void BenchmarkSaveToMemory(const CCommonTestModel& sourceTestModel, std::string& outputData) override
	{
		if constexpr (BitSerializer::is_archive_support_output_data_type_v<typename TArchive::output_archive_type, std::string>)
		{
			BitSerializer::SaveObject<TArchive>(sourceTestModel, outputData);
		}
		else
		{
			std::cout << GetLibraryName() << ": Internal error - save to memory is not supported." << std::endl;
		}
	}

	void BenchmarkLoadFromMemory(CCommonTestModel& targetTestModel, const std::string& sourceData) override
	{
		if constexpr (BitSerializer::is_archive_support_input_data_type_v<typename TArchive::input_archive_type, std::string_view>)
		{
			BitSerializer::LoadObject<TArchive>(targetTestModel, sourceData);
		}
		else
		{
			std::cout << GetLibraryName() << ": Internal error - load from memory is not supported." << std::endl;
		}
	}

	void BenchmarkSaveToStream(const CCommonTestModel& sourceTestModel, std::ostream& outputStream) override
	{
		if constexpr (BitSerializer::is_archive_support_output_data_type_v<typename TArchive::output_archive_type, std::ostream>)
		{
			BitSerializer::SaveObject<TArchive>(sourceTestModel, outputStream);
		}
		else
		{
			std::cout << GetLibraryName() << ": Internal error - save to stream is not supported." << std::endl;
		}
	}

	void BenchmarkLoadFromStream(CCommonTestModel& targetTestModel, std::istream& inputStream) override
	{
		if constexpr (BitSerializer::is_archive_support_input_data_type_v<typename TArchive::input_archive_type, std::istream>)
		{
			BitSerializer::LoadObject<TArchive>(targetTestModel, inputStream);
		}
		else
		{
			std::cout << GetLibraryName() << ": Internal error - load from stream is not supported." << std::endl;
		}
	}

private:
	std::vector<TestStage> mSupportedStagesList;
};

#pragma warning(pop)
