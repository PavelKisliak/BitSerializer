/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;

// NOLINTBEGIN(readability-convert-member-functions-to-static)

/// <summary>
/// Test class with internal string conversions methods.
/// </summary>
class InternalConversionFixture
{
public:
	[[nodiscard]] std::string ToString() const { return {}; }
	[[nodiscard]] std::u16string ToU16String() const { return {}; }
	[[nodiscard]] std::u32string ToU32String() const { return {}; }

	void FromString(std::string_view) { }
	void FromString(std::wstring_view) { }
};

// NOLINTEND(readability-convert-member-functions-to-static)

/// <summary>
/// Test class with external string conversions methods.
/// </summary>
class ExternalConversionFixture {};

std::string to_string(const ExternalConversionFixture&) { return {}; }
std::wstring to_wstring(const ExternalConversionFixture&) { return {}; }

/// <summary>
/// Test class without any conversions methods (internal or external).
/// </summary>
class NotConvertibleFixture { };

//-----------------------------------------------------------------------------

TEST(ConvertTraits, ShouldDetectClassToStringMethod) {
	constexpr bool convertibleResult = Convert::Detail::has_internal_ToString_v<InternalConversionFixture, std::string>;
	EXPECT_TRUE(convertibleResult);
	constexpr bool notConvertibleResult = Convert::Detail::has_internal_ToString_v<NotConvertibleFixture, std::string>;
	EXPECT_FALSE(notConvertibleResult);
}

TEST(ConvertTraits, ShouldDetectClassToU16StringMethod) {
	constexpr bool convertibleResult = Convert::Detail::has_internal_ToString_v<InternalConversionFixture, std::u16string>;
	EXPECT_TRUE(convertibleResult);
	constexpr bool notConvertibleResult = Convert::Detail::has_internal_ToString_v<NotConvertibleFixture, std::u16string>;
	EXPECT_FALSE(notConvertibleResult);
}

TEST(ConvertTraits, ShouldDetectClassToU32StringMethod) {
	constexpr bool convertibleResult = Convert::Detail::has_internal_ToString_v<InternalConversionFixture, std::u32string>;
	EXPECT_TRUE(convertibleResult);
	constexpr bool notConvertibleResult = Convert::Detail::has_internal_ToString_v<NotConvertibleFixture, std::u32string>;
	EXPECT_FALSE(notConvertibleResult);
}

TEST(ConvertTraits, ShouldDetectClassFromStringMethod) {
	constexpr bool convertibleResult = Convert::Detail::has_internal_FromString_v<InternalConversionFixture, std::string>;
	EXPECT_TRUE(convertibleResult);
	constexpr bool notConvertibleResult = Convert::Detail::has_internal_FromString_v<NotConvertibleFixture, std::string>;
	EXPECT_FALSE(notConvertibleResult);
}

//-----------------------------------------------------------------------------

TEST(ConvertTraits, ShouldDetectGlobalTestToStringFunc) {
	constexpr bool convertibleResult = Convert::Detail::has_global_to_string_v<ExternalConversionFixture, std::string>;
	EXPECT_TRUE(convertibleResult);
	constexpr bool notConvertibleResult = Convert::Detail::has_global_to_string_v<NotConvertibleFixture, std::string>;
	EXPECT_FALSE(notConvertibleResult);
}

TEST(ConvertTraits, ShouldDetectGlobalTestToWStringFunc) {
	constexpr bool convertibleResult = Convert::Detail::has_global_to_string_v<ExternalConversionFixture, std::wstring>;
	EXPECT_TRUE(convertibleResult);
	constexpr bool notConvertibleResult = Convert::Detail::has_global_to_string_v<NotConvertibleFixture, std::wstring>;
	EXPECT_FALSE(notConvertibleResult);
}

//-----------------------------------------------------------------------------

TEST(ConvertTraits, ShouldDetectConversibilityToStringView) {
	EXPECT_TRUE(Convert::Detail::is_convertible_to_string_view_v<const std::string&>);
	EXPECT_TRUE(Convert::Detail::is_convertible_to_string_view_v<std::u16string&>);
	EXPECT_TRUE(Convert::Detail::is_convertible_to_string_view_v<std::u32string&>);
	
	EXPECT_TRUE(Convert::Detail::is_convertible_to_string_view_v<const char*>);
	EXPECT_TRUE(Convert::Detail::is_convertible_to_string_view_v<const char16_t*>);
	EXPECT_TRUE(Convert::Detail::is_convertible_to_string_view_v<const char32_t*>);

	// string_view must not be convertible (for avoid redundant conversion)
	EXPECT_FALSE(Convert::Detail::is_convertible_to_string_view_v<std::string_view>);
	EXPECT_FALSE(Convert::Detail::is_convertible_to_string_view_v<int>);
	EXPECT_FALSE(Convert::Detail::is_convertible_to_string_view_v<NotConvertibleFixture>);
}

//-----------------------------------------------------------------------------
TEST(ConvertTraits, ShouldDetectWhetherConversionIsPossible)
{
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<int, std::string>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<float, std::u16string>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<double, std::u32string>));

	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<std::string, int>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<std::u16string, float>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<std::u32string, double>));

	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<std::string_view, int>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<std::u16string_view, float>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<std::u32string_view, double>));

	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<const char*, int>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<const char16_t*, float>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<const char32_t*, double>));

	// Test convert with internal string conversion methods (FromString(), ToString())
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<std::string, InternalConversionFixture>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<const char*, InternalConversionFixture>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<std::string_view, InternalConversionFixture>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<InternalConversionFixture, std::string>));

	// Test convert with externally overloaded conversion methods
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<ExternalConversionFixture, std::string>));

	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<std::chrono::seconds, Detail::CBinTimestamp>));
	EXPECT_TRUE((Convert::Detail::is_convert_supported_v<Detail::CBinTimestamp, std::chrono::seconds>));

	// Test non-convertible classes
	EXPECT_FALSE((Convert::Detail::is_convert_supported_v<std::string_view, NotConvertibleFixture>));
	EXPECT_FALSE((Convert::Detail::is_convert_supported_v<NotConvertibleFixture, std::string>));
}
