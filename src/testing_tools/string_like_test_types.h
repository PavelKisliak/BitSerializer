/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <ostream>
#include <string>
#include <string_view>
#include "auto_fixture.h"
#include "bitserializer/conversion_detail/string_traits.h"

/**
 * @brief A test class that wraps a `std::string` and is declared as a string-like type.
 *
 * It is registered via `BITSERIALIZER_DECLARE_STRING_TYPE_EXPLICIT`, which provides a zero-copy
 * `std::string_view` conversion on save and an assignment on load. The class is defined in a shared
 * header (included by many translation units) to verify that the macro-based registration
 * (`StringViewOf` / `AssignString`) works correctly across translation units.
 *
 * The member functions are deliberately named `Data()`, `Size()` and `FromString()` (instead of the
 * standard `data()`, `size()` and `assign()`) to cover the explicit registration form.
 */
class TestStringLikeType
{
public:
	static void BuildFixture(TestStringLikeType& fixture)
	{
		::BuildFixture(fixture.mString);
	}

	TestStringLikeType() = default;
	explicit TestStringLikeType(std::string_view str) : mString(str) {}

	[[nodiscard]] const char* Data() const noexcept { return mString.data(); }
	[[nodiscard]] size_t Size() const noexcept { return mString.size(); }
	void FromString(std::string_view str) { mString = str; }

	bool operator==(const TestStringLikeType& rhs) const { return mString == rhs.mString; }
	bool operator<(const TestStringLikeType& rhs) const { return mString < rhs.mString; }
	[[nodiscard]] const std::string& GetValue() const { return mString; }

private:
	std::string mString;
};

inline std::ostream& operator<<(std::ostream& stream, const TestStringLikeType& value)
{
	return stream << value.GetValue();
}

BITSERIALIZER_DECLARE_STRING_TYPE_EXPLICIT(TestStringLikeType, &TestStringLikeType::Data, &TestStringLikeType::Size, &TestStringLikeType::FromString)

/**
 * @brief A template for a string-like type that follows the standard `std::basic_string` convention.
 *
 * Registered via the short `BITSERIALIZER_DECLARE_STRING_TYPE` macro, which relies on `data()`,
 * `size()` and `assign(std::basic_string_view<TChar>)`. Registrations are provided for `char` and
 * `wchar_t` (see the `TestStdStringLikeType` / `TestWStringLikeType` aliases), so this also verifies
 * that string-like types work for any character type, not only `char`.
 *
 * @tparam TChar Character type.
 */
template <typename TChar>
class TestStdBasicStringLikeType
{
public:
	static void BuildFixture(TestStdBasicStringLikeType& fixture)
	{
		::BuildFixture(fixture.mString);
	}

	TestStdBasicStringLikeType() = default;
	explicit TestStdBasicStringLikeType(std::basic_string_view<TChar> str) : mString(str) {}

	[[nodiscard]] const TChar* data() const noexcept { return mString.data(); }
	[[nodiscard]] size_t size() const noexcept { return mString.size(); }
	void assign(std::basic_string_view<TChar> str) { mString = str; }

	bool operator==(const TestStdBasicStringLikeType& rhs) const { return mString == rhs.mString; }
	bool operator<(const TestStdBasicStringLikeType& rhs) const { return mString < rhs.mString; }
	[[nodiscard]] const std::basic_string<TChar>& GetValue() const { return mString; }

private:
	std::basic_string<TChar> mString;
};

template <typename TChar>
std::basic_ostream<TChar>& operator<<(std::basic_ostream<TChar>& stream, const TestStdBasicStringLikeType<TChar>& value)
{
	return stream << value.GetValue();
}

BITSERIALIZER_DECLARE_STRING_TYPE(TestStdBasicStringLikeType<char>)
BITSERIALIZER_DECLARE_STRING_TYPE(TestStdBasicStringLikeType<wchar_t>)

/// A string-like type with `char` characters and the standard `std::basic_string` method names.
using TestStdStringLikeType = TestStdBasicStringLikeType<char>;

/// A string-like type with `wchar_t` characters and the standard `std::basic_string` method names.
using TestWStringLikeType = TestStdBasicStringLikeType<wchar_t>;
