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
		name.reserve(32);
		detailedProductDescription.reserve(128);
		localizedName.reserve(128);
		note.reserve(32);
		specifications.reserve(32);
	}

	bool active = false;
	int32_t id = 0;
	uint64_t stock = 0;
	float price = 0.f;
	double rating = 0.0;
	std::string name;
	std::string detailedProductDescription;
	std::string localizedName;
	std::string note;
	std::string specifications;

	/**
	 * @brief Initializes the test fixture with known values.
	 *
	 * @param fixture Reference to the model object to populate.
	 */
	static void BuildFixture(BasicTestModel& fixture)
	{
		fixture.active = true;
		fixture.id = -100;
		fixture.stock = 123456789;
		fixture.price = 3.141592f;
		fixture.rating = -3.141592654;
		fixture.name = "Short string";
		fixture.detailedProductDescription = "A string whose purpose is to test the performance of working with a long key and value";
		fixture.localizedName = UTF8("Съешь ещё этих мягких французских булок, да выпей чаю");
		fixture.note = "Test \"escaped\" chars";
		fixture.specifications = "Test\nmulti\nline\nstring";
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
		Assert("active", active, rhs.active);
		Assert("id", id, rhs.id);
		Assert("stock", stock, rhs.stock);
		Assert("price", price, rhs.price);
		Assert("rating", rating, rhs.rating);
		Assert("name", name, rhs.name);
		Assert("detailedProductDescription", detailedProductDescription, rhs.detailedProductDescription);
		Assert("localizedName", localizedName, rhs.localizedName);
		Assert("note", note, rhs.note);
		Assert("specifications", specifications, rhs.specifications);
	}
};

// Common test model representing an array of basic models for archive testing
using CommonTestModel = std::array<BasicTestModel, 30>;
