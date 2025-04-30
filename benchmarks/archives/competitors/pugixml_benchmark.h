/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
// Benchmark headers
#include "benchmark_base.h"

// PugiXml library
#include "pugixml.hpp"


template <class TModel = CommonTestModel>
class CPugiXmlBenchmark final : public CBenchmarkBase<TModel>
{
public:
	[[nodiscard]] std::string GetLibraryName() const override
	{
		return "PugiXml";
	}

	[[nodiscard]] std::vector<TestStage> GetStagesList() const override
	{
		return { TestStage::SaveToMemory, TestStage::LoadFromMemory, TestStage::SaveToStream, TestStage::LoadFromStream };
	}

protected:
	void SaveToXmlDocument(const TModel& sourceTestModel, pugi::xml_document& xmlDoc)
	{
		auto rootNode = xmlDoc.append_child(PUGIXML_TEXT("array"));

		// Save array of objects
		for (const auto& item : sourceTestModel)
		{
			auto objectNode = rootNode.append_child(PUGIXML_TEXT("object"));
			objectNode.append_child(PUGIXML_TEXT("BooleanValue")).text().set(item.BooleanValue);
			objectNode.append_child(PUGIXML_TEXT("SignedIntValue")).text().set(item.SignedIntValue);
			objectNode.append_child(PUGIXML_TEXT("UnsignedIntValue")).text().set(item.UnsignedIntValue);
			objectNode.append_child(PUGIXML_TEXT("FloatValue")).text().set(item.FloatValue);
			objectNode.append_child(PUGIXML_TEXT("DoubleValue")).text().set(item.DoubleValue);
			objectNode.append_child(PUGIXML_TEXT("ShortString")).text().set(item.ShortString.c_str());
			objectNode.append_child(PUGIXML_TEXT("StringWithLongKeyAndValue")).text().set(item.StringWithLongKeyAndValue.c_str());
			objectNode.append_child(PUGIXML_TEXT("UnicodeString")).text().set(item.UnicodeString.c_str());
			objectNode.append_child(PUGIXML_TEXT("StringWithEscapedChars")).text().set(item.StringWithEscapedChars.c_str());
			objectNode.append_child(PUGIXML_TEXT("MultiLineString")).text().set(item.MultiLineString.c_str());
		}
	}

	void LoadFromXmlDocument(TModel& targetTestModel, const pugi::xml_document& xmlDoc)
	{
		const auto rootNode = xmlDoc.child(PUGIXML_TEXT("array"));

		// Load array of objects
		int i = 0;
		for (auto it = rootNode.begin(); it != rootNode.end(); ++it)
		{
			auto& obj = targetTestModel[i];
			obj.BooleanValue = it->child(PUGIXML_TEXT("BooleanValue")).text().as_bool();
			obj.SignedIntValue = static_cast<int8_t>(it->child(PUGIXML_TEXT("SignedIntValue")).text().as_int());
			obj.UnsignedIntValue = it->child(PUGIXML_TEXT("UnsignedIntValue")).text().as_llong();
			obj.FloatValue = it->child(PUGIXML_TEXT("FloatValue")).text().as_float();
			obj.DoubleValue = it->child(PUGIXML_TEXT("DoubleValue")).text().as_double();
			obj.ShortString = it->child(PUGIXML_TEXT("ShortString")).text().as_string();
			obj.StringWithLongKeyAndValue = it->child(PUGIXML_TEXT("StringWithLongKeyAndValue")).text().as_string();
			obj.UnicodeString = it->child(PUGIXML_TEXT("UnicodeString")).text().as_string();
			obj.StringWithEscapedChars = it->child(PUGIXML_TEXT("StringWithEscapedChars")).text().as_string();
			obj.MultiLineString = it->child(PUGIXML_TEXT("MultiLineString")).text().as_string();
			++i;
		}
	}

	void BenchmarkSaveToMemory(const TModel& sourceTestModel, std::string& outputData) override
	{
		pugi::xml_document xmlDoc;
		SaveToXmlDocument(sourceTestModel, xmlDoc);

		// Build XML
		CXmlStringWriter xmlStringWriter(outputData);
		xmlDoc.save(xmlStringWriter, PUGIXML_TEXT("\t"), pugi::format_raw, pugi::encoding_utf8);
	}

	void BenchmarkLoadFromMemory(TModel& targetTestModel, const std::string& sourceData) override
	{
		pugi::xml_document xmlDoc;
		const auto result = xmlDoc.load_buffer(sourceData.data(), sourceData.size(), pugi::parse_default, pugi::encoding_utf8);
		if (!result) {
			throw std::runtime_error("PugiXml parse error");
		}

		LoadFromXmlDocument(targetTestModel, xmlDoc);
	}

	void BenchmarkSaveToStream(const TModel& sourceTestModel, std::ostream& outputStream) override
	{
		pugi::xml_document xmlDoc;
		SaveToXmlDocument(sourceTestModel, xmlDoc);

		// Build XML
		constexpr unsigned int flags = pugi::format_write_bom;
		xmlDoc.save(outputStream, PUGIXML_TEXT("\t"), flags, pugi::xml_encoding::encoding_utf8);
	}

	void BenchmarkLoadFromStream(TModel& targetTestModel, std::istream& inputStream) override
	{
		rapidjson::IStreamWrapper isw(inputStream);
		rapidjson::AutoUTFInputStream<uint32_t, rapidjson::IStreamWrapper> eis(isw);

		pugi::xml_document xmlDoc;
		const auto result = xmlDoc.load(inputStream);
		if (!result) {
			throw std::runtime_error("PugiXml parse error");
		}
		LoadFromXmlDocument(targetTestModel, xmlDoc);
	}

private:
	class CXmlStringWriter : public pugi::xml_writer
	{
	public:
		CXmlStringWriter(std::string& outputStr)
			: mOutputString(outputStr)
		{
		}

		void write(const void* data, size_t size) override
		{
			mOutputString.append(static_cast<const PUGIXML_CHAR*>(data), size);
		}

	private:
		std::string& mOutputString;
	};
};
