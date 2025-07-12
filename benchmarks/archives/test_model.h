/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <cstdint>
#include "testing_tools/common_test_methods.h"

/**
 * @brief A test model containing a set of basic data types.
 */
class CBasicTestModel
{
public:
	/**
	 * @brief Constructs a new instance of the test model.
	 *
	 * Pre-allocates internal buffers for strings to minimize memory allocation overhead during tests.
	 */
	CBasicTestModel()
	{
		ShortString.reserve(32);
		StringWithLongKeyAndValue.reserve(128);
		UnicodeString.reserve(128);
		StringWithEscapedChars.reserve(32);
		MultiLineString.reserve(32);
	}

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

	/**
	 * @brief Initializes the test fixture with known values.
	 *
	 * @param fixture Reference to the model object to populate.
	 */
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
		fixture.StringWithEscapedChars = "Test \"escaped\" chars";
		fixture.MultiLineString = "Test\nmulti\nline\nstring";
	}

	/**
	 * @brief Returns the number of fields in this model.
	 *
	 * @return constexpr size_t Number of serializable fields.
	 */
	static constexpr size_t GetTotalFieldsCount() noexcept
	{
		return 10;
	}

	/**
	 * @brief Compares two values of any type and throws an exception if they do not match.
	 *
	 * For floating-point numbers, performs approximate comparison due to potential precision loss.
	 *
	 * @tparam T Type of the values being compared.
	 * @param fieldName Name of the field for error reporting.
	 * @param val1 First value to compare.
	 * @param val2 Second value to compare.
	 */
	template <typename T>
	static void Assert(const char* fieldName, const T& val1, const T& val2)
	{
		if constexpr (std::is_floating_point_v<T>)
		{
			// Use approximate comparison for floating points
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

		throw std::runtime_error(std::string(" - Field verification failed on '") + fieldName + "': "
			+ BitSerializer::Convert::ToString(val1) + " != "
			+ BitSerializer::Convert::ToString(val2));
	}

	/**
	 * @brief Verifies that all fields of this object match those of another instance.
	 *
	 * Throws an exception with detailed failure message if any field does not match.
	 *
	 * @param rhs The object to compare against.
	 */
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

// Common test model representing an array of basic models for archive testing
using CCommonTestModel = std::array<CBasicTestModel, 30>;
