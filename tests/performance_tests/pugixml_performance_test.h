/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include "bitserializer/convert.h"
#include "bitserializer/pugixml_archive.h"
#include "base_test_models.h"


class PugiXmlPerformanceTestModel final : public BasePerformanceTestModel<char>
{
public:
	const char* GetName() override { return "PugiXml"; }

	std::string TestSave()
	{
		pugi::xml_document mDoc;
		auto rootNode =  mDoc.append_child(PUGIXML_TEXT("root"));

		// Save array of booleans
		auto booleansXmlNode = rootNode.append_child(PUGIXML_TEXT("ArrayOfBooleans"));
		for (auto item : mArrayOfBooleans) {
			booleansXmlNode.append_child(PUGIXML_TEXT("bool")).text().set(item);
		}

		// Save array of integers
		auto integersXmlNode = rootNode.append_child(PUGIXML_TEXT("ArrayOfInts"));
		for (auto item : mArrayOfInts) {
			integersXmlNode.append_child(PUGIXML_TEXT("int")).text().set(item);
		}

		// Save array of floats
		auto floatsXmlNode = rootNode.append_child(PUGIXML_TEXT("ArrayOfFloats"));
		for (auto item : mArrayOfFloats) {
			floatsXmlNode.append_child(PUGIXML_TEXT("float")).text().set(item);
		}

		// Save array of strings
		auto stringsXmlNode = rootNode.append_child(PUGIXML_TEXT("ArrayOfStrings"));
		for (const auto& item : mArrayOfStrings) {
			stringsXmlNode.append_child(PUGIXML_TEXT("string")).text().set(BitSerializer::Convert::ToString(item).c_str());
		}

		// Save array of objects
		auto objectsXmlNode = rootNode.append_child(PUGIXML_TEXT("ArrayOfObjects"));
		for (const auto& item : mArrayOfObjects) {
			auto objectNode = objectsXmlNode.append_child(PUGIXML_TEXT("object"));
			objectNode.append_child(PUGIXML_TEXT("TestBoolValue")).text().set(item.mTestBoolValue);
			objectNode.append_child(PUGIXML_TEXT("TestCharValue")).text().set(item.mTestCharValue);
			objectNode.append_child(PUGIXML_TEXT("TestInt16Value")).text().set(item.mTestInt16Value);
			objectNode.append_child(PUGIXML_TEXT("TestInt32Value")).text().set(item.mTestInt32Value);
			objectNode.append_child(PUGIXML_TEXT("TestInt64Value")).text().set(item.mTestInt64Value);
			objectNode.append_child(PUGIXML_TEXT("TestFloatValue")).text().set(item.mTestFloatValue);
			objectNode.append_child(PUGIXML_TEXT("TestDoubleValue")).text().set(item.mTestDoubleValue);
			objectNode.append_child(PUGIXML_TEXT("TestStringValue")).text().set(item.mTestStringValue.c_str());
		}

		// Build
		out_string_stream_t stream;
		mDoc.save(stream, PUGIXML_TEXT("\t"), pugi::format_raw, pugi::encoding_utf8);
		return stream.str();
	}

	void TestLoad(const std::string& data)
	{
		pugi::xml_document mDoc;
		const auto result = mDoc.load_buffer(data.data(), data.size(), pugi::parse_default, pugi::encoding_auto);
		if (!result)
			throw std::runtime_error("PugiXml parse error");

		auto rootNode = mDoc.child(PUGIXML_TEXT("root"));

		// Load array of booleans
		auto booleansXmlNode = rootNode.child(PUGIXML_TEXT("ArrayOfBooleans"));
		int i = 0;
		for (auto it = booleansXmlNode.begin(); it != booleansXmlNode.end(); ++it)
		{
			mArrayOfBooleans[i] = it->text().as_bool();
			++i;
		}

		// Load array of integers
		auto integersXmlNode = rootNode.child(PUGIXML_TEXT("ArrayOfInts"));
		i = 0;
		for (auto it = integersXmlNode.begin(); it != integersXmlNode.end(); ++it)
		{
			mArrayOfInts[i] = it->text().as_llong();
			++i;
		}

		// Load array of floats
		auto floatsXmlNode = rootNode.child(PUGIXML_TEXT("ArrayOfFloats"));
		i = 0;
		for (auto it = floatsXmlNode.begin(); it != floatsXmlNode.end(); ++it)
		{
			mArrayOfFloats[i] = it->text().as_double();
			++i;
		}

		// Load array of strings
		auto stringsXmlNode = rootNode.child(PUGIXML_TEXT("ArrayOfStrings"));
		i = 0;
		for (auto it = stringsXmlNode.begin(); it != stringsXmlNode.end(); ++it)
		{
			mArrayOfStrings[i] = it->text().as_string();
			++i;
		}

		// Load array of objects
		auto objectsXmlNode = rootNode.child(PUGIXML_TEXT("ArrayOfObjects"));
		i = 0;
		for (auto it = objectsXmlNode.begin(); it != objectsXmlNode.end(); ++it)
		{
			auto& obj = mArrayOfObjects[i];
			obj.mTestBoolValue = it->child(PUGIXML_TEXT("TestBoolValue")).text().as_bool();
			obj.mTestCharValue = static_cast<char>(it->child(PUGIXML_TEXT("TestCharValue")).text().as_int());
			obj.mTestInt16Value = static_cast<int16_t>(it->child(PUGIXML_TEXT("TestInt16Value")).text().as_int());
			obj.mTestInt32Value = it->child(PUGIXML_TEXT("TestInt32Value")).text().as_int();
			obj.mTestInt64Value = it->child(PUGIXML_TEXT("TestInt64Value")).text().as_llong();
			obj.mTestFloatValue = it->child(PUGIXML_TEXT("TestFloatValue")).text().as_float();
			obj.mTestDoubleValue = it->child(PUGIXML_TEXT("TestDoubleValue")).text().as_double();
			obj.mTestStringValue = it->child(PUGIXML_TEXT("TestStringValue")).text().as_string();
			++i;
		}
	}
};