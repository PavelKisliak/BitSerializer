/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include "bitserializer/string_conversion.h"
#include "bitserializer_pugixml/pugixml_archive.h"
#include "base_test_models.h"


class PugiXmlPerformanceTestModel : public BasePerformanceTestModel<char>
{
public:
	const char* GetName() override
	{
#ifdef PUGIXML_WCHAR_MODE
	return "PugiXml (std::wstring)";
#else
	return "PugiXml (std::string)";
#endif
	}

	std::string TestSave()
	{
		pugi::xml_document mDoc;

		auto decl = mDoc.prepend_child(pugi::node_declaration);
		decl.append_attribute("version") = "1.0";
		decl.append_attribute("encoding") = "UTF-8";
		auto rootNode =  mDoc.append_child("root");

		// Save array of booleans
		auto booleansXmlNode = rootNode.append_child("ArrayOfBooleans");
		for (auto item : mArrayOfBooleans) {
			booleansXmlNode.append_child("bool").text().set(item);
		}

		// Save array of integers
		auto integersXmlNode = rootNode.append_child("ArrayOfInts");
		for (auto item : mArrayOfInts) {
			integersXmlNode.append_child("int").text().set(item);
		}

		// Save array of floats
		auto floatsXmlNode = rootNode.append_child("ArrayOfFloats");
		for (auto item : mArrayOfFloats) {
			floatsXmlNode.append_child("float").text().set(item);
		}

		// Save array of strings
		auto stringsXmlNode = rootNode.append_child("ArrayOfStrings");
		for (const auto& item : mArrayOfStrings) {
			stringsXmlNode.append_child("string").text().set(BitSerializer::Convert::ToString(item).c_str());
		}

		// Save array of objects
		auto objectsXmlNode = rootNode.append_child("ArrayOfObjects");
		for (const auto& item : mArrayOfObjects) {
			auto objectNode = objectsXmlNode.append_child("object");
			objectNode.append_child("TestBoolValue").text().set(item.mTestBoolValue);
			objectNode.append_child("TestCharValue").text().set(item.mTestCharValue);
			objectNode.append_child("TestInt16Value").text().set(item.mTestInt16Value);
			objectNode.append_child("TestInt32Value").text().set(item.mTestInt32Value);
			objectNode.append_child("TestInt64Value").text().set(item.mTestInt64Value);
			objectNode.append_child("TestFloatValue").text().set(item.mTestFloatValue);
			objectNode.append_child("TestDoubleValue").text().set(item.mTestDoubleValue);
			objectNode.append_child("TestStringValue").text().set(item.mTestStringValue.c_str());
			objectNode.append_child("TestWStringValue").text().set(BitSerializer::Convert::ToString(item.mTestWStringValue).c_str());
		}

		// Build
		std::ostringstream stream;
		mDoc.print(stream);
		return stream.str();
	}

	void TestLoad(const std::string& data)
	{
		pugi::xml_document mDoc;
		const auto result = mDoc.load_string(data.c_str());
		if (!result)
			throw std::runtime_error("PugiXml parse error");

		auto rootNode = mDoc.child("root");

		// Load array of booleans
		auto booleansXmlNode = rootNode.child("ArrayOfBooleans");
		int i = 0;
		for (auto it = booleansXmlNode.begin(); it != booleansXmlNode.end(); ++it)
		{
			mArrayOfBooleans[i] = it->text().as_bool();
			++i;
		}

		// Load array of integers
		auto integersXmlNode = rootNode.child("ArrayOfInts");
		i = 0;
		for (auto it = integersXmlNode.begin(); it != integersXmlNode.end(); ++it)
		{
			mArrayOfInts[i] = it->text().as_llong();
			++i;
		}

		// Load array of floats
		auto floatsXmlNode = rootNode.child("ArrayOfFloats");
		i = 0;
		for (auto it = floatsXmlNode.begin(); it != floatsXmlNode.end(); ++it)
		{
			mArrayOfFloats[i] = it->text().as_double();
			++i;
		}

		// Load array of strings
		auto stringsXmlNode = rootNode.child("ArrayOfStrings");
		i = 0;
		for (auto it = stringsXmlNode.begin(); it != stringsXmlNode.end(); ++it)
		{
			mArrayOfStrings[i] = BitSerializer::Convert::ToWString(it->text().as_string());
			++i;
		}

		// Load array of objects
		auto objectsXmlNode = rootNode.child("ArrayOfObjects");
		i = 0;
		for (auto it = objectsXmlNode.begin(); it != objectsXmlNode.end(); ++it)
		{
			auto& obj = mArrayOfObjects[i];
			obj.mTestBoolValue = it->child("TestBoolValue").text().as_bool();
			obj.mTestCharValue = it->child("TestCharValue").text().as_int();
			obj.mTestInt16Value = it->child("TestInt16Value").text().as_int();
			obj.mTestInt32Value = it->child("TestInt32Value").text().as_int();
			obj.mTestInt64Value = it->child("TestInt64Value").text().as_llong();
			obj.mTestFloatValue = it->child("TestFloatValue").text().as_float();
			obj.mTestDoubleValue = it->child("TestDoubleValue").text().as_double();
			obj.mTestStringValue = it->child("TestStringValue").text().as_string();
			obj.mTestWStringValue = BitSerializer::Convert::ToWString(it->child("TestWStringValue").text().as_string());
			++i;
		}
	}
};