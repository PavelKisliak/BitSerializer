/*******************************************************************************
* Copyright (C) 2020 by Artsiom Marozau                                        *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/

#pragma once
#include <stdexcept>
#include "bitserializer_rapidyaml/rapidyaml_archive.h"
#include "base_test_models.h"

class RapidYamlPerformanceTestModel final: public BasePerformanceTestModel<char>
{
public:
	const char* GetName() override { return "RapidYaml"; }

	std::string TestSave()
	{
		ryml::Tree tree;
		auto root = tree.rootref();

		root |= ryml::MAP;

		// Save array of booleans
		auto booleansYamlNode = root.append_child();
		booleansYamlNode << ryml::key("ArrayOfBooleans");
		booleansYamlNode |= ryml::SEQ;
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			booleansYamlNode.append_child() << mArrayOfBooleans[i];
		}

		// Save array of integers
		auto intsYamlArray = root.append_child();
		intsYamlArray << ryml::key("ArrayOfInts");
		intsYamlArray |= ryml::SEQ;
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			intsYamlArray.append_child() << mArrayOfInts[i];
		}

		// Save array of floats
		auto floatsYamlArray = root.append_child();
		floatsYamlArray << ryml::key("ArrayOfFloats");
		floatsYamlArray |= ryml::SEQ;
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			floatsYamlArray.append_child() << mArrayOfFloats[i];
		}

		// Save array of strings
		auto stringsYamlArray = root.append_child();
		stringsYamlArray << ryml::key("ArrayOfStrings");
		stringsYamlArray |= ryml::SEQ;
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			stringsYamlArray.append_child() << mArrayOfStrings[i];
		}

		// Save array of objects
		auto objectsYamlArray = root.append_child();
		objectsYamlArray << ryml::key("ArrayOfObjects");
		objectsYamlArray |= ryml::SEQ;
		for (size_t i = 0; i < ARRAY_SIZE; ++i)
		{
			const auto& obj = mArrayOfObjects[i];
			auto yamlObj = objectsYamlArray.append_child();
			yamlObj |= ryml::MAP;
			yamlObj.append_child() << ryml::key("TestBoolValue") << obj.mTestBoolValue;
			yamlObj.append_child() << ryml::key("TestCharValue") << obj.mTestCharValue;
			yamlObj.append_child() << ryml::key("TestInt16Value") << obj.mTestInt16Value;
			yamlObj.append_child() << ryml::key("TestInt32Value") << obj.mTestInt32Value;
			yamlObj.append_child() << ryml::key("TestInt64Value") << obj.mTestInt64Value;
			yamlObj.append_child() << ryml::key("TestFloatValue") << obj.mTestFloatValue;
			yamlObj.append_child() << ryml::key("TestDoubleValue") << obj.mTestDoubleValue;
			yamlObj.append_child() << ryml::key("TestStringValue") << obj.mTestStringValue;
		}
	
		return ryml::emitrs<std::string>(tree);
	}

	void TestLoad(const std::string& yaml)
	{
		auto tree = ryml::parse(c4::to_csubstr(yaml));
		auto root = tree.rootref();

		//TODO: add parse error check
		/*
		if (root.IsNull())
			throw std::runtime_error("YamlCpp parse error");*/

		// Load array of booleans
		const auto booleansYamlArray = root["ArrayOfBooleans"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			booleansYamlArray[i] >> mArrayOfBooleans[i];
		}

		// Load array of integers
		const auto integersYamlArray = root["ArrayOfInts"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			integersYamlArray[i] >> mArrayOfInts[i];
		}

		// Load array of floats
		const auto floatsYamlArray = root["ArrayOfFloats"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			floatsYamlArray[i] >> mArrayOfFloats[i];
		}

		// Load array of strings
		const auto& stringsYamlArray = root["ArrayOfStrings"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			stringsYamlArray[i] >> mArrayOfStrings[i];
		}

		// Load array of objects
		const auto& objectsYamlArray = root["ArrayOfObjects"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i)
		{
			auto& obj = mArrayOfObjects[i];
			auto yamlVal = objectsYamlArray[i];
			yamlVal["TestBoolValue"] >> obj.mTestBoolValue;
			yamlVal["TestCharValue"] >> obj.mTestCharValue;
			yamlVal["TestInt16Value"] >> obj.mTestInt16Value;
			yamlVal["TestInt32Value"] >> obj.mTestInt32Value;
			yamlVal["TestInt64Value"] >> obj.mTestInt64Value;
			yamlVal["TestFloatValue"] >> obj.mTestFloatValue;
			yamlVal["TestDoubleValue"] >> obj.mTestDoubleValue;
			yamlVal["TestStringValue"] >> obj.mTestStringValue;
		}
	}
};
