/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "testing_tools/common_test_methods.h"

/**
 * @brief A test model containing a set of basic data types.
 */
class BasicTestModel
{
public:
	/**
	 * @brief Constructs a new instance of the test model.
	 *
	 * Pre-allocates internal buffers for strings to minimize memory allocation overhead during tests.
	 */
	BasicTestModel()
	{
		shortString.reserve(32);
		stringWithLongKeyAndValue.reserve(128);
		unicodeString.reserve(128);
		stringWithEscapedChars.reserve(32);
		multiLineString.reserve(32);
	}

	bool booleanValue = false;
	int8_t signedIntValue = 0;
	uint64_t unsignedIntValue = 0;
	float floatValue = 0.f;
	double doubleValue = 0.0;
	std::string shortString;
	std::string stringWithLongKeyAndValue;
	std::string unicodeString;
	std::string stringWithEscapedChars;
	std::string multiLineString;

	/**
	 * @brief Initializes the test fixture with known values.
	 *
	 * @param fixture Reference to the model object to populate.
	 */
	static void BuildFixture(BasicTestModel& fixture)
	{
		fixture.booleanValue = true;
		fixture.signedIntValue = -100;
		fixture.unsignedIntValue = 123456789;
		fixture.floatValue = 3.141592f;
		fixture.doubleValue = -3.141592654;
		fixture.shortString = "Short string";
		fixture.stringWithLongKeyAndValue = "A string whose purpose is to test the performance of working with a long key and value";
		fixture.unicodeString = UTF8("Съешь ещё этих мягких французских булок, да выпей чаю");
		fixture.stringWithEscapedChars = "Test \"escaped\" chars";
		fixture.multiLineString = "Test\nmulti\nline\nstring";
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
	void Assert(const BasicTestModel& rhs) const
	{
		Assert("BooleanValue", booleanValue, rhs.booleanValue);
		Assert("SignedIntValue", signedIntValue, rhs.signedIntValue);
		Assert("UnsignedIntValue", unsignedIntValue, rhs.unsignedIntValue);
		Assert("FloatValue", floatValue, rhs.floatValue);
		Assert("DoubleValue", doubleValue, rhs.doubleValue);
		Assert("ShortString", shortString, rhs.shortString);
		Assert("StringWithLongKeyAndValue", stringWithLongKeyAndValue, rhs.stringWithLongKeyAndValue);
		Assert("UnicodeString", unicodeString, rhs.unicodeString);
		Assert("StringWithEscapedChars", stringWithEscapedChars, rhs.stringWithEscapedChars);
		Assert("MultiLineString", multiLineString, rhs.multiLineString);
	}
};

// Common test model representing an array of basic models for archive testing
using CommonTestModel = std::array<BasicTestModel, 30>;
