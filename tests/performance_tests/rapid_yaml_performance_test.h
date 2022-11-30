/*******************************************************************************
* Copyright (C) 2020-2022 by Artsiom Marozau                                   *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include "bitserializer/rapidyaml_archive.h"
#include "archive_base_perf_test.h"
#include "base_test_models.h"


using RapidYamlTestModel = TestModelWithSubArrays<char>;
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

		root |= ryml::MAP;

		// Save array of booleans
		auto booleansYamlNode = root.append_child();
		booleansYamlNode << ryml::key("ArrayOfBooleans");
		booleansYamlNode |= ryml::SEQ;
		for (size_t i = 0; i < model_t::ARRAY_SIZE; ++i) {
			booleansYamlNode.append_child() << mSourceTestModel.mArrayOfBooleans[i];
		}

		// Save array of integers
		auto intsYamlArray = root.append_child();
		intsYamlArray << ryml::key("ArrayOfInts");
		intsYamlArray |= ryml::SEQ;
		for (size_t i = 0; i < model_t::ARRAY_SIZE; ++i) {
			intsYamlArray.append_child() << mSourceTestModel.mArrayOfInts[i];
		}

		// Save array of strings
		auto stringsYamlArray = root.append_child();
		stringsYamlArray << ryml::key("ArrayOfStrings");
		stringsYamlArray |= ryml::SEQ;
		for (size_t i = 0; i < model_t::ARRAY_SIZE; ++i) {
			stringsYamlArray.append_child() << mSourceTestModel.mArrayOfStrings[i];
		}

		// Save array of objects
		auto objectsYamlArray = root.append_child();
		objectsYamlArray << ryml::key("ArrayOfObjects");
		objectsYamlArray |= ryml::SEQ;
		for (size_t i = 0; i < model_t::ARRAY_SIZE; ++i)
		{
			const auto& obj = mSourceTestModel.mArrayOfObjects[i];
			auto yamlObj = objectsYamlArray.append_child();
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

		// Load array of booleans
		const auto booleansYamlArray = root["ArrayOfBooleans"];
		for (size_t i = 0; i < model_t::ARRAY_SIZE; ++i) {
			booleansYamlArray[i] >> mNativeLibModel.mArrayOfBooleans[i];
		}

		// Load array of integers
		const auto integersYamlArray = root["ArrayOfInts"];
		for (size_t i = 0; i < model_t::ARRAY_SIZE; ++i) {
			integersYamlArray[i] >> mNativeLibModel.mArrayOfInts[i];
		}

		// Load array of strings
		const auto& stringsYamlArray = root["ArrayOfStrings"];
		for (size_t i = 0; i < model_t::ARRAY_SIZE; ++i) {
			stringsYamlArray[i] >> mNativeLibModel.mArrayOfStrings[i];
		}

		// Load array of objects
		int16_t temp;
		const auto& objectsYamlArray = root["ArrayOfObjects"];
		for (size_t i = 0; i < model_t::ARRAY_SIZE; ++i)
		{
			auto& obj = mNativeLibModel.mArrayOfObjects[i];
			auto yamlVal = objectsYamlArray[i];
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

		mSourceTestModel.Assert(mNativeLibModel);
		// Output YAML from BitSerializer and base library should be equal
		assert(mNativeLibOutputData == mBitSerializerOutputData);
	}

	private:
		model_t mNativeLibModel;
		output_format_t mNativeLibOutputData;
};
