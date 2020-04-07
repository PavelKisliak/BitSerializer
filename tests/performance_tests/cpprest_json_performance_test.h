/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include "bitserializer_cpprest_json/cpprest_json_archive.h"
#include "base_test_models.h"


class CppRestJsonPerformanceTestModel final : public BasePerformanceTestModel<utility::char_t>
{
public:
	~CppRestJsonPerformanceTestModel() = default;

	const char* GetName() override { return "CppRestJson"; }

	std::string TestSave()
	{
		web::json::value rootJson = web::json::value::object();
		auto& rootObj = rootJson.as_object();

		// Save array of booleans
		web::json::value& booleansJsonArray = rootObj[_XPLATSTR("ArrayOfBooleans")] = web::json::value::array(ARRAY_SIZE);
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			booleansJsonArray[i] = web::json::value(mArrayOfBooleans[i]);
		}

		// Save array of integers
		web::json::value& intsJsonArray = rootObj[_XPLATSTR("ArrayOfInts")] = web::json::value::array(ARRAY_SIZE);
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			intsJsonArray[i] = web::json::value(mArrayOfInts[i]);
		}

		// Save array of floats
		web::json::value& floatsJsonArray = rootObj[_XPLATSTR("ArrayOfFloats")] = web::json::value::array(ARRAY_SIZE);
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			floatsJsonArray[i] = web::json::value(mArrayOfFloats[i]);
		}

		// Save array of strings
		web::json::value& stringsJsonArray = rootObj[_XPLATSTR("ArrayOfStrings")] = web::json::value::array(ARRAY_SIZE);
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			stringsJsonArray[i] = web::json::value(mArrayOfStrings[i]);
		}

		// Save array of objects
		web::json::value& objectsJsonArray = rootObj[_XPLATSTR("ArrayOfObjects")] = web::json::value::array(ARRAY_SIZE);
		for (size_t i = 0; i < ARRAY_SIZE; ++i)
		{
			const auto& obj = mArrayOfObjects[i];
			auto& jObj = objectsJsonArray[i] = web::json::value::object();
			jObj[_XPLATSTR("TestBoolValue")] = web::json::value(obj.mTestBoolValue);
			jObj[_XPLATSTR("TestCharValue")] = web::json::value(obj.mTestCharValue);
			jObj[_XPLATSTR("TestInt16Value")] = web::json::value(obj.mTestInt16Value);
			jObj[_XPLATSTR("TestInt32Value")] = web::json::value(obj.mTestInt32Value);
			jObj[_XPLATSTR("TestInt64Value")] = web::json::value(obj.mTestInt64Value);
			jObj[_XPLATSTR("TestFloatValue")] = web::json::value(obj.mTestFloatValue);
			jObj[_XPLATSTR("TestDoubleValue")] = web::json::value(obj.mTestDoubleValue);
			jObj[_XPLATSTR("TestStringValue")] = web::json::value(obj.mTestStringValue);
		}

#ifdef _UTF16_STRINGS
		// Encode to UTF-8 (CppRestSDK does not have native methods on Windows platform)
		return utility::conversions::to_utf8string(rootJson.serialize());
#else
		return rootJson.serialize();
#endif
	}

	void TestLoad(const std::string& json)
	{
#ifdef _UTF16_STRINGS
		auto rootJson = web::json::value::parse(utility::conversions::to_string_t(json));
#else
		auto rootJson = web::json::value::parse(json);
#endif
		if (rootJson.is_null())
			throw std::runtime_error("CppRestJson parse error");
		auto& rootObj = rootJson.as_object();

		// Load array of booleans
		const auto& booleansJsonArray = rootObj.find(_XPLATSTR("ArrayOfBooleans"))->second.as_array();
		int i = 0;
		for (const auto& jVal : booleansJsonArray) {
			mArrayOfBooleans[i] = jVal.as_bool();
			++i;
		}

		// Load array of integers
		const auto& integersJsonArray = rootObj.find(_XPLATSTR("ArrayOfInts"))->second.as_array();
		i = 0;
		for (const auto& jVal : integersJsonArray) {
			mArrayOfInts[i] = jVal.as_number().to_int64();
			++i;
		}

		// Load array of floats
		const auto& floatsJsonArray = rootObj.find(_XPLATSTR("ArrayOfFloats"))->second.as_array();
		i = 0;
		for (const auto& jVal : floatsJsonArray) {
			mArrayOfFloats[i] = jVal.as_double();
			++i;
		}

		// Load array of strings
		const auto& stringsJsonArray = rootObj.find(_XPLATSTR("ArrayOfStrings"))->second.as_array();
		i = 0;
		for (const auto& jVal : stringsJsonArray) {
			mArrayOfStrings[i] = jVal.as_string();
			++i;
		}

		// Load array of objects
		const auto& objectsJsonArray = rootObj.find(_XPLATSTR("ArrayOfObjects"))->second.as_array();
		i = 0;
		for (const auto& jVal : objectsJsonArray)
		{
			auto& obj = mArrayOfObjects[i];
			const auto& jObj = jVal.as_object();
			obj.mTestBoolValue = jObj.find(_XPLATSTR("TestBoolValue"))->second.as_bool();
			obj.mTestCharValue = static_cast<char>(jObj.find(_XPLATSTR("TestCharValue"))->second.as_integer());
			obj.mTestInt16Value = static_cast<int16_t>(jObj.find(_XPLATSTR("TestInt16Value"))->second.as_integer());
			obj.mTestInt32Value = jObj.find(_XPLATSTR("TestInt32Value"))->second.as_integer();
			obj.mTestInt64Value = jObj.find(_XPLATSTR("TestInt64Value"))->second.as_number().to_int64();
			obj.mTestFloatValue = static_cast<float>(jObj.find(_XPLATSTR("TestFloatValue"))->second.as_double());
			obj.mTestDoubleValue = jObj.find(_XPLATSTR("TestDoubleValue"))->second.as_double();
			obj.mTestStringValue = jObj.find(_XPLATSTR("TestStringValue"))->second.as_string();
			++i;
		}
	}
};