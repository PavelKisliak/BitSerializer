/*******************************************************************************
* Copyright (C) 2020 by Artsiom Marozau                                        *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/

#pragma once
#include <stdexcept>
#include "bitserializer_yaml_cpp/yaml_cpp_archive.h"
#include "base_test_models.h"

namespace YAML {
	template <typename TKey>
	struct convert<ModelWithBasicTypes<TKey>> {
		static Node encode(const ModelWithBasicTypes<TKey>& rhs) {
			Node node;
			node["TestBoolValue"] = rhs.mTestBoolValue;
			node["TestCharValue"] = static_cast<int>(rhs.mTestCharValue);
			node["TestInt16Value"] = rhs.mTestInt16Value;
			node["TestInt32Value"] = rhs.mTestInt32Value;
			node["TestInt64Value"] = rhs.mTestInt64Value;
			node["TestFloatValue"] = rhs.mTestFloatValue;
			node["TestDoubleValue"] = rhs.mTestDoubleValue;
			node["TestStringValue"] = rhs.mTestStringValue;
			return node;
		}

		static bool decode(const Node& node, ModelWithBasicTypes<TKey>& rhs) {
			if (!node.IsMap() || node.size() != 8) {
				return false;
			}
			rhs.mTestBoolValue = node["TestBoolValue"].as<bool>();
			rhs.mTestCharValue = static_cast<char>(node["TestCharValue"].as<int>());
			rhs.mTestInt16Value = node["TestInt16Value"].as<int16_t>();
			rhs.mTestInt32Value = node["TestInt32Value"].as<int32_t>();
			rhs.mTestInt64Value = node["TestInt64Value"].as<int64_t>();
			rhs.mTestFloatValue = node["TestFloatValue"].as<float>();
			rhs.mTestDoubleValue = node["TestDoubleValue"].as<double>();
			rhs.mTestStringValue = node["TestStringValue"].as<std::string>();
			return true;
		}
	};
}

class YamlCppPerformanceTestModel final: public BasePerformanceTestModel<char>
{
public:
	~YamlCppPerformanceTestModel() = default;

	const char* GetName() override { return "YamlCpp"; }

	std::string TestSave()
	{
		auto root = YAML::Node(YAML::NodeType::Map);

		// Save array of booleans
		SerializeArray(std::cbegin(mArrayOfBooleans), std::cend(mArrayOfBooleans), root, "ArrayOfBooleans");

		// Save array of integers
		SerializeArray(std::cbegin(mArrayOfInts), std::cend(mArrayOfInts), root, "ArrayOfInts");

		// Save array of floats
		SerializeArray(std::cbegin(mArrayOfFloats), std::cend(mArrayOfFloats), root, "ArrayOfFloats");

		// Save array of strings
		SerializeArray(std::cbegin(mArrayOfStrings), std::cend(mArrayOfStrings), root, "ArrayOfStrings");

		// Save array of objects
		SerializeArray(std::cbegin(mArrayOfObjects), std::cend(mArrayOfObjects), root, "ArrayOfObjects");
	
		return YAML::Dump(root);
	}

	void TestLoad(const std::string& json)
	{
		auto root = YAML::Load(json);

		if (root.IsNull())
			throw std::runtime_error("YamlCpp parse error");

		// Load array of booleans
		DeserializeArray(std::begin(mArrayOfBooleans), std::end(mArrayOfBooleans), root, "ArrayOfBooleans");

		// Load array of integers
		DeserializeArray(std::begin(mArrayOfInts), std::end(mArrayOfInts), root, "ArrayOfInts");
		
		// Load array of floats
		DeserializeArray(std::begin(mArrayOfFloats), std::end(mArrayOfFloats), root, "ArrayOfFloats");

		// Load array of strings
		DeserializeArray(std::begin(mArrayOfStrings), std::end(mArrayOfStrings), root, "ArrayOfStrings");

		// Load array of objects
		DeserializeArray(std::begin(mArrayOfObjects), std::end(mArrayOfObjects), root, "ArrayOfObjects");
	}

private: 
	template <typename TIter>
	void SerializeArray(TIter first, TIter last, YAML::Node& root, const std::string& key)
	{
		auto seqNode = root[key] = YAML::Node(YAML::NodeType::Sequence);
		for (; first != last; ++first)
		{
			seqNode.push_back(*first);
		}
	}

	template <typename TIter>
	void DeserializeArray(TIter first, TIter last, YAML::Node& root, const std::string& key)
	{
		auto seqNode = root[key];
		size_t i = 0;
		for (; first != last; ++first)
		{
			(*first) = seqNode[i].as<typename std::iterator_traits<TIter>::value_type>();
			++i;
		}
	}
};