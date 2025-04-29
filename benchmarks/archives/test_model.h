/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <cstdint>
#include "testing_tools/common_test_methods.h"

/// <summary>
/// Test model with set of simple types.
/// </summary>
class CBasicTestModel
{
public:
	bool BooleanValue = false;
	int8_t SignedIntValue = 0;
	uint64_t UnsignedIntValue = 0;
	float FloatValue = 0.f;
	double DoubleValue = 0.0;
	std::string ShortString;
	std::string StringWithLongKeyAndValue;
	std::string UnicodeString;
	std::string StringWithEscapedChars;
	std::string MultiLineString;

	static void BuildFixture(CBasicTestModel& fixture)
	{
		fixture.BooleanValue = true;
		fixture.SignedIntValue = -100;
		fixture.UnsignedIntValue = 123456789;
		fixture.FloatValue = 3.141592f;
		fixture.DoubleValue = -3.141592654;
		fixture.ShortString = "Short string";
		fixture.StringWithLongKeyAndValue = "A string whose purpose is to test the performance of working with a long key and value";
		fixture.UnicodeString = UTF8("Съешь ещё этих мягких французских булок, да выпей чаю");
		fixture.StringWithEscapedChars = "Test \"<quoted>\" string";
		fixture.MultiLineString = "Test\nmulti\nline\nstring";
	}

	static constexpr size_t GetTotalFieldsCount() noexcept
	{
		return 10;
	}

	template <typename T>
	static void Assert(const char* fieldName, const T& val1, const T& val2)
	{
		if constexpr (std::is_floating_point_v<T>)
		{
			// Approximately compare floating point numbers due to possible loss precision when store in the text formats
			if (ApproximatelyEqual(val1, val2, std::numeric_limits<T>::epsilon() * 5))
			{
				return;
			}
		}
		else
		{
			if (val1 == val2)
			{
				return;
			}
		}
		throw std::runtime_error(std::string(" - Failed verification on field '") + fieldName + "': " + BitSerializer::Convert::ToString(val1) + " != " + BitSerializer::Convert::ToString(val2));
	}

	void Assert(const CBasicTestModel& rhs) const
	{
		Assert("BooleanValue", BooleanValue, rhs.BooleanValue);
		Assert("SignedIntValue", SignedIntValue, rhs.SignedIntValue);
		Assert("UnsignedIntValue", UnsignedIntValue, rhs.UnsignedIntValue);
		Assert("FloatValue", FloatValue, rhs.FloatValue);
		Assert("DoubleValue", DoubleValue, rhs.DoubleValue);
		Assert("ShortString", ShortString, rhs.ShortString);
		Assert("StringWithLongKeyAndValue", StringWithLongKeyAndValue, rhs.StringWithLongKeyAndValue);
		Assert("UnicodeString", UnicodeString, rhs.UnicodeString);
		Assert("StringWithEscapedChars", StringWithEscapedChars, rhs.StringWithEscapedChars);
		Assert("MultiLineString", MultiLineString, rhs.MultiLineString);
	}
};

// Common test model for all archives (represents an array of objects)
using CommonTestModel = std::array<CBasicTestModel, 30>;
