/*******************************************************************************
* Copyright (C) 2020 by Artsiom Marozau                                        *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/

#pragma once
#include <stdexcept>
#include "bitserializer_yaml_cpp/yaml_cpp_archive.h"
#include "base_test_models.h"

class YamlCppPerformanceTestModel final: public BasePerformanceTestModel<char>
{
public:
	const char* GetName() override { return "YamlCpp"; }

	std::string TestSave()
	{
		auto root = YAML::Node(YAML::NodeType::Map);

		// Save array of booleans
		auto booleansYamlNode = root["ArrayOfBooleans"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			booleansYamlNode[i] = mArrayOfBooleans[i];
		}

		// Save array of integers
		auto intsYamlArray = root["ArrayOfInts"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			intsYamlArray[i] = mArrayOfInts[i];
		}

		// Save array of floats
		auto floatsJsonArray = root["ArrayOfFloats"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			floatsJsonArray[i] = mArrayOfFloats[i];
		}

		// Save array of strings
		auto stringsJsonArray = root["ArrayOfStrings"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			stringsJsonArray[i] = mArrayOfStrings[i];
		}

		// Save array of objects
		auto objectsYamlArray = root["ArrayOfObjects"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i)
		{
			const auto& obj = mArrayOfObjects[i];
			auto yamlObj = objectsYamlArray[i];
			yamlObj["TestBoolValue"] = obj.mTestBoolValue;
			yamlObj["TestCharValue"] = static_cast<short>(obj.mTestCharValue);
			yamlObj["TestInt16Value"] = obj.mTestInt16Value;
			yamlObj["TestInt32Value"] = obj.mTestInt32Value;
			yamlObj["TestInt64Value"] = obj.mTestInt64Value;
			yamlObj["TestFloatValue"] = obj.mTestFloatValue;
			yamlObj["TestDoubleValue"] = obj.mTestDoubleValue;
			yamlObj["TestStringValue"] = obj.mTestStringValue;
		}
	
		return YAML::Dump(root);
	}

	void TestLoad(const std::string& json)
	{
		auto root = YAML::Load(json);

		if (root.IsNull())
			throw std::runtime_error("YamlCpp parse error");

		// Load array of booleans
		const auto booleansYamlArray = root["ArrayOfBooleans"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			mArrayOfBooleans[i] = booleansYamlArray[i].as<bool>();
		}

		// Load array of integers
		const auto integersYamlArray = root["ArrayOfInts"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			mArrayOfInts[i] = integersYamlArray[i].as<long long>();
		}

		// Load array of floats
		const auto floatsYamlArray = root["ArrayOfFloats"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			mArrayOfFloats[i] = floatsYamlArray[i].as<double>();
		}

		// Load array of strings
		const auto& stringsYamlArray = root["ArrayOfStrings"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			mArrayOfStrings[i] = stringsYamlArray[i].as<std::string>();
		}

		// Load array of objects
		const auto& objectsYamlArray = root["ArrayOfObjects"];
		for (size_t i = 0; i < ARRAY_SIZE; ++i)
		{
			auto& obj = mArrayOfObjects[i];
			auto yamlVal = objectsYamlArray[i];
			obj.mTestBoolValue = yamlVal["TestBoolValue"].as<bool>();
			obj.mTestCharValue = static_cast<char>(yamlVal["TestCharValue"].as<long long>());
			obj.mTestInt16Value = static_cast<int16_t>(yamlVal["TestInt16Value"].as<long long>());
			obj.mTestInt32Value = static_cast<int32_t>(yamlVal["TestInt32Value"].as<long long>());
			obj.mTestInt64Value = yamlVal["TestInt64Value"].as<long long>();
			obj.mTestFloatValue = static_cast<float>(yamlVal["TestFloatValue"].as<double>());
			obj.mTestDoubleValue = yamlVal["TestDoubleValue"].as<double>();
			obj.mTestStringValue = yamlVal["TestStringValue"].as<std::string>();
		}
	}
};
