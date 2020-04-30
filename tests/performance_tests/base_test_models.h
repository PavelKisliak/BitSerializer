/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdint>
#include <string>
#include "test_helpers/common_test_methods.h"

template <typename TKey>
class ModelWithBasicTypes
{
public:
	using char_t = TKey;
	using string_t = std::basic_string<TKey, std::char_traits<TKey>>;

	static void BuildFixture(ModelWithBasicTypes& fixture)
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

	void Assert(const ModelWithBasicTypes& rhs) const
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
		if constexpr (std::is_same_v<TKey, char>)
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

template <typename TKey>
class BasePerformanceTestModel
{
public:
	static constexpr size_t ARRAY_SIZE = 30;

	using char_t = TKey;
	using string_t = std::basic_string<TKey, std::char_traits<TKey>>;
	using out_string_stream_t = std::basic_ostringstream<TKey, std::char_traits<TKey>, std::allocator<TKey>>;
	using in_string_stream_t = std::basic_ostringstream<TKey, std::char_traits<TKey>, std::allocator<TKey>>;

	virtual ~BasePerformanceTestModel() = default;

	virtual const char* GetName() = 0;

	static void BuildFixture(BasePerformanceTestModel& fixture)
	{
		::BuildFixture(fixture.mArrayOfBooleans);
		::BuildFixture(fixture.mArrayOfInts);
		::BuildFixture(fixture.mArrayOfFloats);
		::BuildFixture(fixture.mArrayOfStrings);
		::BuildFixture(fixture.mArrayOfObjects);
	}

	void Assert(const BasePerformanceTestModel& rhs) const
	{
		for (size_t i=0; i<ARRAY_SIZE; ++i)
		{
			assert(mArrayOfBooleans[i] == rhs.mArrayOfBooleans[i]);
			assert(mArrayOfInts[i] == rhs.mArrayOfInts[i]);
			assert(mArrayOfFloats[i] == rhs.mArrayOfFloats[i]);
			assert(mArrayOfStrings[i] == rhs.mArrayOfStrings[i]);
			mArrayOfObjects[i].Assert(rhs.mArrayOfObjects[i]);
		}
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		if constexpr (std::is_same_v<TKey, char>)
		{
			archive << BitSerializer::MakeKeyValue("ArrayOfBooleans", mArrayOfBooleans);
			archive << BitSerializer::MakeKeyValue("ArrayOfInts", mArrayOfInts);
			archive << BitSerializer::MakeKeyValue("ArrayOfFloats", mArrayOfFloats);
			archive << BitSerializer::MakeKeyValue("ArrayOfStrings", mArrayOfStrings);
			archive << BitSerializer::MakeKeyValue("ArrayOfObjects", mArrayOfObjects);
		}
		else
		{
			archive << BitSerializer::MakeKeyValue(L"ArrayOfBooleans", mArrayOfBooleans);
			archive << BitSerializer::MakeKeyValue(L"ArrayOfInts", mArrayOfInts);
			archive << BitSerializer::MakeKeyValue(L"ArrayOfFloats", mArrayOfFloats);
			archive << BitSerializer::MakeKeyValue(L"ArrayOfStrings", mArrayOfStrings);
			archive << BitSerializer::MakeKeyValue(L"ArrayOfObjects", mArrayOfObjects);
		}
	}

protected:
	bool mArrayOfBooleans[ARRAY_SIZE] = {};
	int64_t mArrayOfInts[ARRAY_SIZE] = {};
	double mArrayOfFloats[ARRAY_SIZE] = {};
	string_t mArrayOfStrings[ARRAY_SIZE];
	ModelWithBasicTypes<TKey> mArrayOfObjects[ARRAY_SIZE] = {};
};
