/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
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
		::BuildFixture(fixture.mTestWStringValue);
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
		assert(mTestWStringValue == rhs.mTestWStringValue);
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
			archive << BitSerializer::MakeKeyValue("TestWStringValue", mTestWStringValue);
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
			archive << BitSerializer::MakeKeyValue(L"TestWStringValue", mTestWStringValue);
		}
	}

	bool mTestBoolValue;
	char mTestCharValue;
	int16_t mTestInt16Value;
	int32_t mTestInt32Value;
	int64_t mTestInt64Value;
	float mTestFloatValue;
	double mTestDoubleValue;
	std::string mTestStringValue;
	std::wstring mTestWStringValue;
};

template <typename TKey>
class BasePerformanceTestModel
{
public:
	static constexpr size_t ARRAY_SIZE = 20;

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
	std::wstring mArrayOfStrings[ARRAY_SIZE];
	ModelWithBasicTypes<TKey> mArrayOfObjects[ARRAY_SIZE] = {};
};
