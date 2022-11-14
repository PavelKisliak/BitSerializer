/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdint>
#include <string>
#include "tests/test_helpers/common_test_methods.h"


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
		::BuildFixture(fixture.mTestInt16Value);
		::BuildFixture(fixture.mTestInt32Value);
		::BuildFixture(fixture.mTestInt64Value);
		::BuildFixture(fixture.mTestFloatValue);
		::BuildFixture(fixture.mTestDoubleValue);
		::BuildFixture(fixture.mTestStringValue);
	}

	void Assert(const TestModelWithBasicTypes& rhs) const
	{
		assert(mTestBoolValue == rhs.mTestBoolValue);
		assert(mTestCharValue == rhs.mTestCharValue);
		assert(mTestInt16Value == rhs.mTestInt16Value);
		assert(mTestInt32Value == rhs.mTestInt32Value);
		assert(mTestInt64Value == rhs.mTestInt64Value);
		assert(mTestFloatValue == rhs.mTestFloatValue);
		assert(mTestDoubleValue == rhs.mTestDoubleValue);
		assert(mTestStringValue == rhs.mTestStringValue);
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		if constexpr (std::is_same_v<TKeyCharType, char>)
		{
			archive << BitSerializer::MakeKeyValue("TestBoolValue", mTestBoolValue);
			archive << BitSerializer::MakeKeyValue("TestCharValue", mTestCharValue);
			archive << BitSerializer::MakeKeyValue("TestInt16Value", mTestInt16Value);
			archive << BitSerializer::MakeKeyValue("TestInt32Value", mTestInt32Value);
			archive << BitSerializer::MakeKeyValue("TestInt64Value", mTestInt64Value);
			archive << BitSerializer::MakeKeyValue("TestFloatValue", mTestFloatValue);
			archive << BitSerializer::MakeKeyValue("TestDoubleValue", mTestDoubleValue);
			archive << BitSerializer::MakeKeyValue("TestStringValue", mTestStringValue);
		}
		else
		{
			archive << BitSerializer::MakeKeyValue(L"TestBoolValue", mTestBoolValue);
			archive << BitSerializer::MakeKeyValue(L"TestCharValue", mTestCharValue);
			archive << BitSerializer::MakeKeyValue(L"TestInt16Value", mTestInt16Value);
			archive << BitSerializer::MakeKeyValue(L"TestInt32Value", mTestInt32Value);
			archive << BitSerializer::MakeKeyValue(L"TestInt64Value", mTestInt64Value);
			archive << BitSerializer::MakeKeyValue(L"TestFloatValue", mTestFloatValue);
			archive << BitSerializer::MakeKeyValue(L"TestDoubleValue", mTestDoubleValue);
			archive << BitSerializer::MakeKeyValue(L"TestStringValue", mTestStringValue);
		}
	}

	bool mTestBoolValue = false;
	char mTestCharValue = 0;
	int16_t mTestInt16Value = 0;
	int32_t mTestInt32Value = 0;
	int64_t mTestInt64Value = 0;
	float mTestFloatValue = 0.f;
	double mTestDoubleValue = 0.0;
	string_t mTestStringValue;
};


/// <summary>
/// Complex test model with sub arrays of basic and object types.
/// </summary>
template <typename TKeyCharType>
class TestModelWithSubArrays
{
public:
	static constexpr size_t ARRAY_SIZE = 30;

	using key_char_t = TKeyCharType;
	using string_t = std::basic_string<TKeyCharType, std::char_traits<TKeyCharType>>;

	static void BuildFixture(TestModelWithSubArrays& fixture)
	{
		::BuildFixture(fixture.mArrayOfBooleans);
		::BuildFixture(fixture.mArrayOfInts);
		::BuildFixture(fixture.mArrayOfDoubles);
		::BuildFixture(fixture.mArrayOfStrings);
		::BuildFixture(fixture.mArrayOfObjects);
	}

	void Assert(const TestModelWithSubArrays& rhs) const
	{
		for (size_t i=0; i<ARRAY_SIZE; ++i)
		{
			assert(mArrayOfBooleans[i] == rhs.mArrayOfBooleans[i]);
			assert(mArrayOfInts[i] == rhs.mArrayOfInts[i]);
			assert(mArrayOfDoubles[i] == rhs.mArrayOfDoubles[i]);
			assert(mArrayOfStrings[i] == rhs.mArrayOfStrings[i]);
			mArrayOfObjects[i].Assert(rhs.mArrayOfObjects[i]);
		}
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		if constexpr (std::is_same_v<TKeyCharType, char>)
		{
			archive << BitSerializer::MakeKeyValue("ArrayOfBooleans", mArrayOfBooleans);
			archive << BitSerializer::MakeKeyValue("ArrayOfInts", mArrayOfInts);
			archive << BitSerializer::MakeKeyValue("ArrayOfDoubles", mArrayOfDoubles);
			archive << BitSerializer::MakeKeyValue("ArrayOfStrings", mArrayOfStrings);
			archive << BitSerializer::MakeKeyValue("ArrayOfObjects", mArrayOfObjects);
		}
		else
		{
			archive << BitSerializer::MakeKeyValue(L"ArrayOfBooleans", mArrayOfBooleans);
			archive << BitSerializer::MakeKeyValue(L"ArrayOfInts", mArrayOfInts);
			archive << BitSerializer::MakeKeyValue(L"ArrayOfDoubles", mArrayOfDoubles);
			archive << BitSerializer::MakeKeyValue(L"ArrayOfStrings", mArrayOfStrings);
			archive << BitSerializer::MakeKeyValue(L"ArrayOfObjects", mArrayOfObjects);
		}
	}

	bool mArrayOfBooleans[ARRAY_SIZE] = {};
	int64_t mArrayOfInts[ARRAY_SIZE] = {};
	double mArrayOfDoubles[ARRAY_SIZE] = {};
	string_t mArrayOfStrings[ARRAY_SIZE];
	TestModelWithBasicTypes<TKeyCharType> mArrayOfObjects[ARRAY_SIZE] = {};
};
