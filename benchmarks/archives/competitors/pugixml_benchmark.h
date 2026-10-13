/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
// Benchmark headers
#include "benchmark_base.h"

// PugiXml library
#include "pugixml.hpp"


/**
 * @brief Benchmark implementation for the PugiXml library.
 */
class PugiXmlBenchmark final : public BenchmarkBase
{
public:
	[[nodiscard]] std::string GetLibraryName() const override
	{
		return "PugiXml";
	}

protected:
	void SaveToXmlDocument(const CommonTestModel& sourceTestModel, pugi::xml_document& xmlDoc)	// NOLINT(readability-convert-member-functions-to-static)
	{
		auto rootNode = xmlDoc.append_child(PUGIXML_TEXT("array"));

		// Save array of objects
		for (const auto& item : sourceTestModel)
		{
			auto objectNode = rootNode.append_child(PUGIXML_TEXT("object"));
			objectNode.append_child(PUGIXML_TEXT("BooleanValue")).text().set(item.booleanValue);
			objectNode.append_child(PUGIXML_TEXT("SignedIntValue")).text().set(item.signedIntValue);
			objectNode.append_child(PUGIXML_TEXT("UnsignedIntValue")).text().set(item.unsignedIntValue);
			objectNode.append_child(PUGIXML_TEXT("FloatValue")).text().set(item.floatValue);
			objectNode.append_child(PUGIXML_TEXT("DoubleValue")).text().set(item.doubleValue);
			objectNode.append_child(PUGIXML_TEXT("ShortString")).text().set(item.shortString.c_str());
			objectNode.append_child(PUGIXML_TEXT("StringWithLongKeyAndValue")).text().set(item.stringWithLongKeyAndValue.c_str());
			objectNode.append_child(PUGIXML_TEXT("UnicodeString")).text().set(item.unicodeString.c_str());
			objectNode.append_child(PUGIXML_TEXT("StringWithEscapedChars")).text().set(item.stringWithEscapedChars.c_str());
			objectNode.append_child(PUGIXML_TEXT("MultiLineString")).text().set(item.multiLineString.c_str());
		}
	}

	void LoadFromXmlDocument(CommonTestModel& targetTestModel, const pugi::xml_document& xmlDoc)	// NOLINT(readability-convert-member-functions-to-static)
	{
		const auto rootNode = xmlDoc.child(PUGIXML_TEXT("array"));

		// Load array of objects
		int i = 0;
		for (auto& it : rootNode)
		{
			auto& obj = targetTestModel[i];
			obj.booleanValue = it.child(PUGIXML_TEXT("BooleanValue")).text().as_bool();
			obj.signedIntValue = static_cast<int8_t>(it.child(PUGIXML_TEXT("SignedIntValue")).text().as_int());
			obj.unsignedIntValue = it.child(PUGIXML_TEXT("UnsignedIntValue")).text().as_llong();
			obj.floatValue = it.child(PUGIXML_TEXT("FloatValue")).text().as_float();
			obj.doubleValue = it.child(PUGIXML_TEXT("DoubleValue")).text().as_double();
			obj.shortString = it.child(PUGIXML_TEXT("ShortString")).text().as_string();
			obj.stringWithLongKeyAndValue = it.child(PUGIXML_TEXT("StringWithLongKeyAndValue")).text().as_string();
			obj.unicodeString = it.child(PUGIXML_TEXT("UnicodeString")).text().as_string();
			obj.stringWithEscapedChars = it.child(PUGIXML_TEXT("StringWithEscapedChars")).text().as_string();
			obj.multiLineString = it.child(PUGIXML_TEXT("MultiLineString")).text().as_string();
			++i;
		}
	}

	void BenchmarkSaveToMemory(const CommonTestModel& sourceTestModel, std::string& outputData) override
	{
		pugi::xml_document xmlDoc;
		SaveToXmlDocument(sourceTestModel, xmlDoc);

		// Build XML
		XmlStringWriter xmlStringWriter(outputData);
		xmlDoc.save(xmlStringWriter, PUGIXML_TEXT("\t"), pugi::format_raw, pugi::encoding_utf8);
	}

	void BenchmarkLoadFromMemory(CommonTestModel& targetTestModel, const std::string& sourceData) override
	{
		pugi::xml_document xmlDoc;
		const auto result = xmlDoc.load_buffer(sourceData.data(), sourceData.size(), pugi::parse_default, pugi::encoding_utf8);
		if (!result) {
			throw std::runtime_error("PugiXml parse error");
		}

		LoadFromXmlDocument(targetTestModel, xmlDoc);
	}

	void BenchmarkSaveToStream(const CommonTestModel& sourceTestModel, std::ostream& outputStream) override
	{
		pugi::xml_document xmlDoc;
		SaveToXmlDocument(sourceTestModel, xmlDoc);

		// Build XML
		constexpr unsigned int flags = pugi::format_write_bom;
		xmlDoc.save(outputStream, PUGIXML_TEXT("\t"), flags, pugi::xml_encoding::encoding_utf8);
	}

	void BenchmarkLoadFromStream(CommonTestModel& targetTestModel, std::istream& inputStream) override
	{
		pugi::xml_document xmlDoc;
		const auto result = xmlDoc.load(inputStream);
		if (!result) {
			throw std::runtime_error("PugiXml parse error");
		}
		LoadFromXmlDocument(targetTestModel, xmlDoc);
	}

private:
	class XmlStringWriter : public pugi::xml_writer
	{
	public:
		XmlStringWriter(std::string& outputStr)
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
