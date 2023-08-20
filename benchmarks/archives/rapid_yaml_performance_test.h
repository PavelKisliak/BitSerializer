/*******************************************************************************
* Copyright (C) 2020-2023 by Artsiom Marozau, Pavel Kisliak                    *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/rapidyaml_archive.h"
#include "bitserializer/types/std/array.h"
#include "base_perf_test.h"
#include "base_test_models.h"


using RapidYamlTestModel = std::array<TestModelWithBasicTypes<char>, TestArraySize>;
using RapidYamlBasePerfTest = CArchiveBasePerfTest<BitSerializer::Yaml::RapidYaml::YamlArchive, RapidYamlTestModel, char>;

class CRapidYamlPerformanceTest final : public RapidYamlBasePerfTest
{
public:
	using model_t = RapidYamlTestModel;
	using base_class_t = RapidYamlBasePerfTest;

	[[nodiscard]] std::string GetArchiveName() const override { return "RapidYaml"; }
	[[nodiscard]] bool IsUseNativeLib() const override { return true; }

	size_t SaveModelViaNativeLib() override
	{
		ryml::Tree tree;
		auto root = tree.rootref();
		root |= ryml::SEQ;

		// Save array of objects
		for (size_t i = 0; i < TestArraySize; ++i)
		{
			const auto& obj = mSourceTestModel[i];
			auto yamlObj = root.append_child();
			yamlObj |= ryml::MAP;
			yamlObj.append_child() << ryml::key("TestBoolValue") << obj.mTestBoolValue;
			yamlObj.append_child() << ryml::key("TestCharValue") << static_cast<int16_t>(obj.mTestCharValue);
			yamlObj.append_child() << ryml::key("TestInt64Value") << obj.mTestInt64Value;
			yamlObj.append_child() << ryml::key("TestFloatValue")
				<< c4::fmt::real(obj.mTestFloatValue, std::numeric_limits<float>::max_digits10, c4::RealFormat_e::FTOA_SCIENT);
			yamlObj.append_child() << ryml::key("TestDoubleValue") 
				<< c4::fmt::real(obj.mTestDoubleValue, std::numeric_limits<double>::max_digits10, c4::RealFormat_e::FTOA_SCIENT);
			yamlObj.append_child() << ryml::key("TestString1") << obj.mTestString1;
			yamlObj.append_child() << ryml::key("TestString2") << obj.mTestString2;
			yamlObj.append_child() << ryml::key("TestString3") << obj.mTestString3;
			yamlObj.append_child() << ryml::key("StringWithQuotes") << obj.mStringWithQuotes;
			yamlObj.append_child() << ryml::key("MultiLineString") << obj.mMultiLineString;
		}

		mNativeLibOutputData = ryml::emitrs<std::string>(tree);
		return mNativeLibOutputData.size();
	}

	size_t LoadModelViaNativeLib() override
	{
		ryml::Tree tree = ryml::parse_in_arena(c4::to_csubstr(mNativeLibOutputData));
		auto root = tree.rootref();

		// Load array of objects
		int16_t temp;
		for (size_t i = 0; i < TestArraySize; ++i)
		{
			auto& obj = mNativeLibModel[i];
			auto yamlVal = root[i];
			yamlVal["TestBoolValue"] >> obj.mTestBoolValue;
			yamlVal["TestCharValue"] >> temp;
			obj.mTestCharValue = static_cast<char>(temp);
			yamlVal["TestCharValue"] >> reinterpret_cast<int16_t&>(obj.mTestCharValue);
			yamlVal["TestInt64Value"] >> obj.mTestInt64Value;
			yamlVal["TestFloatValue"] >> obj.mTestFloatValue;
			yamlVal["TestDoubleValue"] >> obj.mTestDoubleValue;
			yamlVal["TestString1"] >> obj.mTestString1;
			yamlVal["TestString2"] >> obj.mTestString2;
			yamlVal["TestString3"] >> obj.mTestString3;
			yamlVal["StringWithQuotes"] >> obj.mStringWithQuotes;
			yamlVal["MultiLineString"] >> obj.mMultiLineString;
		}

		return mNativeLibOutputData.size();
	}

	void Assert() const override
	{
		base_class_t::Assert();

		for (size_t i = 0; i < mSourceTestModel.size(); i++)
		{
			mSourceTestModel[i].Assert(mBitSerializerModel[i]);
		}
		// Output YAML from BitSerializer and base library should be equal
		assert(mNativeLibOutputData == mBitSerializerOutputData);
	}

	private:
		model_t mNativeLibModel;
		output_format_t mNativeLibOutputData;
};
