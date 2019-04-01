/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <exception>
#include "bitserializer/string_conversion.h"
#include "bitserializer_cpprest_json/cpprest_json_archive.h"
#include "base_test_models.h"


class CppRestJsonPerformanceTestModel : public BasePerformanceTestModel
{
public:
	~CppRestJsonPerformanceTestModel() = default;

	const char* GetName() override
	{
		if constexpr (std::is_same_v<wchar_t, utility::string_t::value_type>)
			return "CppRestJson (std::wstring)";
		else
			return "CppRestJson (std::string)";
	}

	utility::string_t TestSave()
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
#ifdef _UTF16_STRINGS
			stringsJsonArray[i] = web::json::value(mArrayOfStrings[i]);
#else
			stringsJsonArray[i] = web::json::value(BitSerializer::Convert::To<utility::string_t>(mArrayOfStrings[i]));
#endif
		}

		// Save array of objects
		web::json::value& objectsJsonArray = rootObj[_XPLATSTR("ArrayOfObjects")] = web::json::value::array(ARRAY_SIZE);
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			const auto& obj = mArrayOfObjects[i];
			auto& jObj = objectsJsonArray[i] = web::json::value::object();
			jObj[_XPLATSTR("TestBoolValue")] = web::json::value(obj.mTestBoolValue);
			jObj[_XPLATSTR("TestCharValue")] = web::json::value(obj.mTestCharValue);
			jObj[_XPLATSTR("TestInt16Value")] = web::json::value(obj.mTestInt16Value);
			jObj[_XPLATSTR("TestInt32Value")] = web::json::value(obj.mTestInt32Value);
			jObj[_XPLATSTR("TestInt64Value")] = web::json::value(obj.mTestInt64Value);
			jObj[_XPLATSTR("TestFloatValue")] = web::json::value(obj.mTestFloatValue);
			jObj[_XPLATSTR("TestDoubleValue")] = web::json::value(obj.mTestDoubleValue);
#ifdef _UTF16_STRINGS
			jObj[_XPLATSTR("TestStringValue")] = web::json::value(BitSerializer::Convert::To<utility::string_t>(obj.mTestStringValue));
			jObj[_XPLATSTR("TestWStringValue")] = web::json::value(obj.mTestWStringValue);
#else
			jObj[_XPLATSTR("TestStringValue")] = web::json::value(obj.mTestStringValue);
			jObj[_XPLATSTR("TestWStringValue")] = web::json::value(BitSerializer::Convert::To<utility::string_t>(obj.mTestWStringValue));
#endif
		}

		// Build
		return rootJson.serialize();
	}

	void TestLoad(const utility::string_t& json)
	{
		auto rootJson = web::json::value::parse(json);
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
			LoadString(jVal, mArrayOfStrings[i]);
			++i;
		}

		// Load array of objects
		const auto& objectsJsonArray = rootObj.find(_XPLATSTR("ArrayOfObjects"))->second.as_array();
		i = 0;
		for (const auto& jVal : objectsJsonArray) {
			auto& obj = mArrayOfObjects[i];
			const auto& jObj = jVal.as_object();
			obj.mTestBoolValue = jObj.find(_XPLATSTR("TestBoolValue"))->second.as_bool();
			obj.mTestCharValue = static_cast<char>(jObj.find(_XPLATSTR("TestCharValue"))->second.as_number().to_int32());
			obj.mTestInt16Value = static_cast<int16_t>(jObj.find(_XPLATSTR("TestInt16Value"))->second.as_number().to_int32());
			obj.mTestInt32Value = jObj.find(_XPLATSTR("TestInt32Value"))->second.as_number().to_int32();
			obj.mTestInt64Value = jObj.find(_XPLATSTR("TestInt64Value"))->second.as_number().to_int64();
			obj.mTestFloatValue = static_cast<float>(jObj.find(_XPLATSTR("TestFloatValue"))->second.as_double());
			obj.mTestDoubleValue = jObj.find(_XPLATSTR("TestDoubleValue"))->second.as_double();
			LoadString(jObj.find(_XPLATSTR("TestStringValue"))->second, obj.mTestStringValue);
			LoadString(jObj.find(_XPLATSTR("TestWStringValue"))->second, obj.mTestWStringValue);
			++i;
		}
	}

	template <typename TSym, typename TAllocator>
	inline web::json::value StringToJsonValue(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
			return web::json::value(value);
		else
			return web::json::value(BitSerializer::Convert::To<utility::string_t>(value));
	}

	template <typename TSym, typename TAllocator>
	inline bool LoadString(const web::json::value& jsonValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
			value = jsonValue.as_string();
		else
			value = BitSerializer::Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(jsonValue.as_string());
		return true;
	}
};