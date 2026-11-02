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
			objectNode.append_child(PUGIXML_TEXT("active")).text().set(item.active);
			objectNode.append_child(PUGIXML_TEXT("id")).text().set(item.id);
			objectNode.append_child(PUGIXML_TEXT("stock")).text().set(item.stock);
			objectNode.append_child(PUGIXML_TEXT("price")).text().set(item.price);
			objectNode.append_child(PUGIXML_TEXT("rating")).text().set(item.rating);
			objectNode.append_child(PUGIXML_TEXT("name")).text().set(item.name.c_str());
			objectNode.append_child(PUGIXML_TEXT("detailedProductDescription")).text().set(item.detailedProductDescription.c_str());
			objectNode.append_child(PUGIXML_TEXT("localizedName")).text().set(item.localizedName.c_str());
			objectNode.append_child(PUGIXML_TEXT("note")).text().set(item.note.c_str());
			objectNode.append_child(PUGIXML_TEXT("specifications")).text().set(item.specifications.c_str());
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
			obj.active = it.child(PUGIXML_TEXT("active")).text().as_bool();
			obj.id = static_cast<int32_t>(it.child(PUGIXML_TEXT("id")).text().as_int());
			obj.stock = it.child(PUGIXML_TEXT("stock")).text().as_llong();
			obj.price = it.child(PUGIXML_TEXT("price")).text().as_float();
			obj.rating = it.child(PUGIXML_TEXT("rating")).text().as_double();
			obj.name = it.child(PUGIXML_TEXT("name")).text().as_string();
			obj.detailedProductDescription = it.child(PUGIXML_TEXT("detailedProductDescription")).text().as_string();
			obj.localizedName = it.child(PUGIXML_TEXT("localizedName")).text().as_string();
			obj.note = it.child(PUGIXML_TEXT("note")).text().as_string();
			obj.specifications = it.child(PUGIXML_TEXT("specifications")).text().as_string();
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
