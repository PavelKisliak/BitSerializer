/*******************************************************************************
* Copyright (C) 2018-2025 by Artsiom Marozau, Pavel Kisliak                    *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
// Benchmark headers
#include "benchmark_base.h"

// RapidYaml library
#include "c4/format.hpp"
#include "ryml/ryml.hpp"


/**
 * @brief Benchmark implementation for the RapidYaml library.
 */
class CRapidYamlBenchmark final : public CBenchmarkBase
{
public:
	[[nodiscard]] std::string GetLibraryName() const override
	{
		return "RapidYaml";
	}

	[[nodiscard]] std::vector<TestStage> GetStagesList() const override
	{
		// Exclude serialization tests to streams (not supported by RapidYaml)
		return { TestStage::SaveToMemory, TestStage::LoadFromMemory };
	}

protected:
	using RapidJsonDocument = rapidjson::GenericDocument<rapidjson::UTF8<>>;
	using RapidJsonNode = rapidjson::GenericValue<rapidjson::UTF8<>>;
	using StringBuffer = rapidjson::GenericStringBuffer<rapidjson::UTF8<>>;

	void BenchmarkSaveToMemory(const CCommonTestModel& sourceTestModel, std::string& outputData) override
	{
		ryml::Tree tree;
		auto root = tree.rootref();
		root |= ryml::SEQ;

		// Save array of objects
		for (size_t i = 0; i < sourceTestModel.size(); ++i)
		{
			const auto& obj = sourceTestModel[i];
			auto yamlObj = root.append_child();
			yamlObj |= ryml::MAP;
			yamlObj.append_child() << ryml::key("BooleanValue") << c4::fmt::boolalpha(obj.BooleanValue);
			yamlObj.append_child() << ryml::key("SignedIntValue") << obj.SignedIntValue;
			yamlObj.append_child() << ryml::key("UnsignedIntValue") << obj.UnsignedIntValue;
			yamlObj.append_child() << ryml::key("FloatValue")
				<< c4::fmt::real(obj.FloatValue, std::numeric_limits<float>::max_digits10, c4::RealFormat_e::FTOA_SCIENT);
			yamlObj.append_child() << ryml::key("DoubleValue")
				<< c4::fmt::real(obj.DoubleValue, std::numeric_limits<double>::max_digits10, c4::RealFormat_e::FTOA_SCIENT);
			yamlObj.append_child() << ryml::key("ShortString") << obj.ShortString;
			yamlObj.append_child() << ryml::key("StringWithLongKeyAndValue") << obj.StringWithLongKeyAndValue;
			yamlObj.append_child() << ryml::key("UnicodeString") << obj.UnicodeString;
			yamlObj.append_child() << ryml::key("StringWithEscapedChars") << obj.StringWithEscapedChars;
			yamlObj.append_child() << ryml::key("MultiLineString") << obj.MultiLineString;
		}

		outputData = ryml::emitrs_yaml<std::string>(tree);
	}

	void BenchmarkLoadFromMemory(CCommonTestModel& targetTestModel, const std::string& sourceData) override
	{
		ryml::Tree tree = ryml::parse_in_arena(c4::to_csubstr(sourceData));
		auto root = tree.rootref();

		// Load array of objects
		for (size_t i = 0; i < targetTestModel.size(); ++i)
		{
			auto& obj = targetTestModel[i];
			auto yamlVal = root[i];
			yamlVal["BooleanValue"] >> obj.BooleanValue;
			yamlVal["SignedIntValue"] >> obj.SignedIntValue;
			yamlVal["UnsignedIntValue"] >> obj.UnsignedIntValue;
			yamlVal["FloatValue"] >> obj.FloatValue;
			yamlVal["DoubleValue"] >> obj.DoubleValue;
			yamlVal["ShortString"] >> obj.ShortString;
			yamlVal["StringWithLongKeyAndValue"] >> obj.StringWithLongKeyAndValue;
			yamlVal["UnicodeString"] >> obj.UnicodeString;
			yamlVal["StringWithEscapedChars"] >> obj.StringWithEscapedChars;
			yamlVal["MultiLineString"] >> obj.MultiLineString;
		}
	}
};
