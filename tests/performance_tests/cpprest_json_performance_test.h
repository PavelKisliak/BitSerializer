/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include "bitserializer/cpprestjson_archive.h"
#include "bitserializer/types/std/array.h"
#include "archive_base_perf_test.h"
#include "base_test_models.h"


// Char type in the CppRestSdk depends from the platform
using CppRestCharType = utility::char_t;

using CppRestJsonTestModel = std::array<TestModelWithBasicTypes<CppRestCharType>, TestArraySize>;
using CppRestJsonBasePerfTest = CArchiveBasePerfTest<BitSerializer::Json::CppRest::JsonArchive, CppRestJsonTestModel, CppRestCharType>;

class CCppRestJsonPerformanceTest final : public CppRestJsonBasePerfTest
{
public:
	using model_t = CppRestJsonTestModel;
	using base_class_t = CppRestJsonBasePerfTest;

	[[nodiscard]] std::string GetArchiveName() const override { return "CppRestJson"; }
	[[nodiscard]] bool IsUseNativeLib() const override { return true; }

	size_t SaveModelViaNativeLib() override
	{
		web::json::value rootJson = web::json::value::array(mSourceTestModel.size());

		// Save array of objects
		int i = 0;
		auto& arrayOfObjects = rootJson.as_array();
		for (auto& jObj : arrayOfObjects)
		{
			const auto& obj = mSourceTestModel[i];
			jObj[_XPLATSTR("TestBoolValue")] = web::json::value(obj.mTestBoolValue);
			jObj[_XPLATSTR("TestCharValue")] = web::json::value(obj.mTestCharValue);
			jObj[_XPLATSTR("TestInt64Value")] = web::json::value(obj.mTestInt64Value);
			jObj[_XPLATSTR("TestFloatValue")] = web::json::value(obj.mTestFloatValue);
			jObj[_XPLATSTR("TestDoubleValue")] = web::json::value(obj.mTestDoubleValue);
			jObj[_XPLATSTR("TestString1")] = web::json::value(obj.mTestString1);
			jObj[_XPLATSTR("TestString2")] = web::json::value(obj.mTestString2);
			jObj[_XPLATSTR("TestString3")] = web::json::value(obj.mTestString3);
			jObj[_XPLATSTR("StringWithQuotes")] = web::json::value(obj.mStringWithQuotes);
			jObj[_XPLATSTR("MultiLineString")] = web::json::value(obj.mMultiLineString);
			++i;
		}

#ifdef _UTF16_STRINGS
		// Encode to UTF-8 (CppRestSDK does not have native methods on Windows platform)
		mNativeLibOutputData = utility::conversions::to_utf8string(rootJson.serialize());
#else
		mNativeLibOutputData = rootJson.serialize();
#endif
		return mNativeLibOutputData.size();
	}

	size_t LoadModelViaNativeLib() override
	{
#ifdef _UTF16_STRINGS
		auto rootJson = web::json::value::parse(utility::conversions::to_string_t(mNativeLibOutputData));
#else
		auto rootJson = web::json::value::parse(mNativeLibOutputData);
#endif
		if (rootJson.is_null())
			throw std::runtime_error("CppRestJson parse error");

		// Load array of objects
		int i = 0;
		const auto& arrayOfObjects = rootJson.as_array();
		for (const auto& jVal : arrayOfObjects)
		{
			auto& obj = mNativeLibModel[i];
			const auto& jObj = jVal.as_object();
			obj.mTestBoolValue = jObj.find(_XPLATSTR("TestBoolValue"))->second.as_bool();
			obj.mTestCharValue = static_cast<char>(jObj.find(_XPLATSTR("TestCharValue"))->second.as_integer());
			obj.mTestInt64Value = jObj.find(_XPLATSTR("TestInt64Value"))->second.as_number().to_int64();
			obj.mTestFloatValue = static_cast<float>(jObj.find(_XPLATSTR("TestFloatValue"))->second.as_double());
			obj.mTestDoubleValue = jObj.find(_XPLATSTR("TestDoubleValue"))->second.as_double();
			obj.mTestString1 = jObj.find(_XPLATSTR("TestString1"))->second.as_string();
			obj.mTestString2 = jObj.find(_XPLATSTR("TestString2"))->second.as_string();
			obj.mTestString3 = jObj.find(_XPLATSTR("TestString3"))->second.as_string();
			obj.mStringWithQuotes = jObj.find(_XPLATSTR("StringWithQuotes"))->second.as_string();
			obj.mMultiLineString = jObj.find(_XPLATSTR("MultiLineString"))->second.as_string();
			++i;
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
		// Output JSON from BitSerializer and base library should be equal
		assert(mNativeLibOutputData == mBitSerializerOutputData);
	}

	private:
		model_t mNativeLibModel;
		output_format_t mNativeLibOutputData;
};
