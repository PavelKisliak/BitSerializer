/*******************************************************************************
* Copyright (C) 2018-2026 by Artsiom Marozau, Pavel Kisliak                    *
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
class RapidYamlBenchmark final : public BenchmarkBase
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
	void BenchmarkSaveToMemory(const CommonTestModel& sourceTestModel, std::string& outputData) override
	{
		ryml::Tree tree;
		auto root = tree.rootref();
		root |= ryml::SEQ;

		// Save array of objects
		for (const auto& obj : sourceTestModel)
		{
			auto yamlObj = root.append_child();
			yamlObj |= ryml::MAP;
			yamlObj.append_child() << ryml::key("BooleanValue") << c4::fmt::boolalpha(obj.booleanValue);
			yamlObj.append_child() << ryml::key("SignedIntValue") << obj.signedIntValue;
			yamlObj.append_child() << ryml::key("UnsignedIntValue") << obj.unsignedIntValue;
			yamlObj.append_child() << ryml::key("FloatValue")
				<< c4::fmt::real(obj.floatValue, std::numeric_limits<float>::max_digits10, c4::RealFormat_e::FTOA_SCIENT);
			yamlObj.append_child() << ryml::key("DoubleValue")
				<< c4::fmt::real(obj.doubleValue, std::numeric_limits<double>::max_digits10, c4::RealFormat_e::FTOA_SCIENT);
			yamlObj.append_child() << ryml::key("ShortString") << obj.shortString;
			yamlObj.append_child() << ryml::key("StringWithLongKeyAndValue") << obj.stringWithLongKeyAndValue;
			yamlObj.append_child() << ryml::key("UnicodeString") << obj.unicodeString;
			yamlObj.append_child() << ryml::key("StringWithEscapedChars") << obj.stringWithEscapedChars;
			yamlObj.append_child() << ryml::key("MultiLineString") << obj.multiLineString;
		}

		outputData = ryml::emitrs_yaml<std::string>(tree);
	}

	void BenchmarkLoadFromMemory(CommonTestModel& targetTestModel, const std::string& sourceData) override
	{
		ryml::Tree tree = ryml::parse_in_arena(c4::to_csubstr(sourceData));
		auto root = tree.rootref();

		// Load array of objects
		for (size_t i = 0; i < targetTestModel.size(); ++i)
		{
			auto& obj = targetTestModel[i];
			auto yamlVal = root[i];
			yamlVal["BooleanValue"] >> obj.booleanValue;
			yamlVal["SignedIntValue"] >> obj.signedIntValue;
			yamlVal["UnsignedIntValue"] >> obj.unsignedIntValue;
			yamlVal["FloatValue"] >> obj.floatValue;
			yamlVal["DoubleValue"] >> obj.doubleValue;
			yamlVal["ShortString"] >> obj.shortString;
			yamlVal["StringWithLongKeyAndValue"] >> obj.stringWithLongKeyAndValue;
			yamlVal["UnicodeString"] >> obj.unicodeString;
			yamlVal["StringWithEscapedChars"] >> obj.stringWithEscapedChars;
			yamlVal["MultiLineString"] >> obj.multiLineString;
		}
	}
};
