/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdint>
#include <string>
#include "testing_tools/common_test_methods.h"

// Array size of test models
constexpr size_t TestArraySize = 30;

/// <summary>
/// Test model with set of basic types.
/// </summary>
template <typename TKeyCharType>
class TestModelWithBasicTypes
{
public:
	using key_char_t = TKeyCharType;
	using string_t = std::basic_string<TKeyCharType, std::char_traits<TKeyCharType>>;

	static void BuildFixture(TestModelWithBasicTypes& fixture)
	{
		::BuildFixture(fixture.mTestBoolValue);
		::BuildFixture(fixture.mTestCharValue);
		::BuildFixture(fixture.mTestInt64Value);
		::BuildFixture(fixture.mTestFloatValue);
		::BuildFixture(fixture.mTestDoubleValue);
		::BuildFixture(fixture.mTestString1);
		::BuildFixture(fixture.mTestString2);
		::BuildFixture(fixture.mTestString3);
		if constexpr (std::is_same_v<TKeyCharType, char>)
		{
			fixture.mStringWithQuotes = "Test \"<quoted>\" string";
			fixture.mMultiLineString = "Test\nmulti\nline\nstring";
		}
		else
		{
			fixture.mStringWithQuotes = L"Test \"<quoted>\" string";
			fixture.mMultiLineString = L"Test\nmulti\nline\nstring";
		}
	}

	static constexpr size_t GetTotalFieldsCount() noexcept
	{
		return 10;
	}

	void Assert(const TestModelWithBasicTypes& rhs) const
	{
		assert(mTestBoolValue == rhs.mTestBoolValue);
		assert(mTestCharValue == rhs.mTestCharValue);
		assert(mTestInt64Value == rhs.mTestInt64Value);
		assert(mTestFloatValue == rhs.mTestFloatValue);
		assert(mTestDoubleValue == rhs.mTestDoubleValue);
		assert(mTestString1 == rhs.mTestString1);
		assert(mTestString2 == rhs.mTestString2);
		assert(mTestString3 == rhs.mTestString3);
		assert(mStringWithQuotes == rhs.mStringWithQuotes);
		assert(mMultiLineString == rhs.mMultiLineString);
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		if constexpr (std::is_same_v<TKeyCharType, char>)
		{
			archive << BitSerializer::MakeKeyValue("TestBoolValue", mTestBoolValue);
			archive << BitSerializer::MakeKeyValue("TestCharValue", mTestCharValue);
			archive << BitSerializer::MakeKeyValue("TestInt64Value", mTestInt64Value);
			archive << BitSerializer::MakeKeyValue("TestFloatValue", mTestFloatValue);
			archive << BitSerializer::MakeKeyValue("TestDoubleValue", mTestDoubleValue);
			archive << BitSerializer::MakeKeyValue("TestString1", mTestString1);
			archive << BitSerializer::MakeKeyValue("TestString2", mTestString2);
			archive << BitSerializer::MakeKeyValue("TestString3", mTestString3);
			archive << BitSerializer::MakeKeyValue("StringWithQuotes", mStringWithQuotes);
			archive << BitSerializer::MakeKeyValue("MultiLineString", mMultiLineString);
		}
		else
		{
			archive << BitSerializer::MakeKeyValue(L"TestBoolValue", mTestBoolValue);
			archive << BitSerializer::MakeKeyValue(L"TestCharValue", mTestCharValue);
			archive << BitSerializer::MakeKeyValue(L"TestInt64Value", mTestInt64Value);
			archive << BitSerializer::MakeKeyValue(L"TestFloatValue", mTestFloatValue);
			archive << BitSerializer::MakeKeyValue(L"TestDoubleValue", mTestDoubleValue);
			archive << BitSerializer::MakeKeyValue(L"TestString1", mTestString1);
			archive << BitSerializer::MakeKeyValue(L"TestString2", mTestString2);
			archive << BitSerializer::MakeKeyValue(L"TestString3", mTestString3);
			archive << BitSerializer::MakeKeyValue(L"StringWithQuotes", mStringWithQuotes);
			archive << BitSerializer::MakeKeyValue(L"MultiLineString", mMultiLineString);
		}
	}

	bool mTestBoolValue = false;
	char mTestCharValue = 0;
	int64_t mTestInt64Value = 0;
	float mTestFloatValue = 0.f;
	double mTestDoubleValue = 0.0;
	string_t mTestString1;
	string_t mTestString2;
	string_t mTestString3;
	string_t mStringWithQuotes;
	string_t mMultiLineString;
};
