/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;

/// <summary>
/// Test class without any conversions methods (internal or external).
/// </summary>
class NotConvertibleFixture { };

//-----------------------------------------------------------------------------
// Test IsConvertible<>()
//-----------------------------------------------------------------------------
TEST(ConvertApi, ShouldDetectWhetherTypeIsConvertible)
{
	EXPECT_TRUE((Convert::IsConvertible<int, std::string>()));
	EXPECT_TRUE((Convert::IsConvertible<std::u16string, int>()));
	EXPECT_TRUE((Convert::IsConvertible<const char16_t*, float>()));
	EXPECT_TRUE((Convert::IsConvertible<std::u32string_view, double>()));

	// Test convert with internal string conversion methods (FromString(), ToString())
	EXPECT_TRUE((Convert::IsConvertible<std::string, TestPointClass>()));
	EXPECT_TRUE((Convert::IsConvertible<std::string_view, TestPointClass>()));
	EXPECT_TRUE((Convert::IsConvertible<const char*, TestPointClass>()));
	EXPECT_TRUE((Convert::IsConvertible<TestPointClass, std::string>()));

	// Test convert with externally overloaded conversion methods
	EXPECT_TRUE((Convert::IsConvertible<std::chrono::nanoseconds, Detail::CBinTimestamp>()));
	EXPECT_TRUE((Convert::IsConvertible<Detail::CBinTimestamp, std::chrono::nanoseconds>()));

	// Test non-convertible classes
	EXPECT_FALSE((Convert::IsConvertible<std::string_view, NotConvertibleFixture>()));
	EXPECT_FALSE((Convert::IsConvertible<NotConvertibleFixture, std::string>()));
}


//-----------------------------------------------------------------------------
// Test To<>()
//-----------------------------------------------------------------------------
TEST(ConvertApi, ShouldConvertFromRawCString) {
	EXPECT_EQ(-100500, Convert::To<int32_t>("  -100500  "));
}

TEST(ConvertApi, ShouldConvertFromStringView) {
	EXPECT_EQ(-100500, Convert::To<int32_t>(std::string_view("  -100500  ")));
	EXPECT_EQ(-100500, Convert::To<int32_t>(std::wstring_view(L"  -100500  ")));
	EXPECT_EQ(-100500, Convert::To<int32_t>(std::u16string_view(u"  -100500  ")));
	EXPECT_EQ(-100500, Convert::To<int32_t>(std::u32string_view(U"  -100500  ")));
}

TEST(ConvertApi, ShouldConvertStdString) {
	EXPECT_EQ(100500, Convert::To<int32_t>(std::string("  100500  ")));
	EXPECT_EQ(100500, Convert::To<int32_t>(std::wstring(L"  100500  ")));
	EXPECT_EQ(100500, Convert::To<int32_t>(std::u16string(u"  100500  ")));
	EXPECT_EQ(100500, Convert::To<int32_t>(std::u32string(U"  100500  ")));
}

#pragma warning(push)
#pragma warning(disable: 4566)
TEST(ConvertApi, ShouldConvertUtf8ToAnyStringType) {
	EXPECT_EQ(UTF8("Привет мир!"), Convert::ToString(UTF8("Привет мир!")));
	EXPECT_EQ(L"😀😎🙋", Convert::ToWString(UTF8("😀😎🙋")));
	EXPECT_EQ(u"Привет мир!", Convert::To<std::u16string>(UTF8("Привет мир!")));
	EXPECT_EQ(U"Привет мир!", Convert::To<std::u32string>(UTF8("Привет мир!")));
}

TEST(ConvertApi, ShouldConvertUtf16ToAnyStringType) {
	EXPECT_EQ(UTF8("Привет мир!"), Convert::ToString(u"Привет мир!"));
	EXPECT_EQ(L"😀😎🙋", Convert::ToWString(u"😀😎🙋"));
	EXPECT_EQ(u"Привет мир!", Convert::To<std::u16string>(u"Привет мир!"));
	EXPECT_EQ(U"Привет мир!", Convert::To<std::u32string>(u"Привет мир!"));
}

TEST(ConvertApi, ShouldConvertUtf32ToAnyStringType) {
	EXPECT_EQ(UTF8("Привет мир!"), Convert::ToString(U"Привет мир!"));
	EXPECT_EQ(L"😀😎🙋", Convert::ToWString(U"😀😎🙋"));
	EXPECT_EQ(u"Привет мир!", Convert::To<std::u16string>(U"Привет мир!"));
	EXPECT_EQ(U"Привет мир!", Convert::To<std::u32string>(U"Привет мир!"));
}
#pragma warning(pop)

TEST(ConvertApi, ShouldThrowExceptionWhenWrongUtfSequence)
{
	const std::string wrongSequence({ char(0b11110111), char(0b10111111), char(0b10111111), char(0b11111111) });
	const std::string source = "test" + wrongSequence + "test";

	EXPECT_THROW(Convert::To<std::u16string>(source), std::invalid_argument);
}

TEST(ConvertApi, ShouldReturnTheSamePointerWhenConvertToSameType) {
	const char* expectedPtr = "test";
	EXPECT_EQ(expectedPtr, Convert::To<const char*>(expectedPtr));
}

TEST(ConvertApi, ShouldReturnTheSameValueWhenConvertToSameType) {
	EXPECT_EQ(500, Convert::To<int>(500));
}

TEST(ConvertApi, ShouldMoveStringValue)
{
	const auto stringCapacity = std::string().capacity();
	std::string sourceStr(stringCapacity + 1, '*');
	const char* expectedPtr = sourceStr.data();

	auto targetStr = Convert::To<std::string>(std::move(sourceStr));
	EXPECT_EQ(expectedPtr, targetStr.data());
}

TEST(ConvertApi, ShouldThrowExceptionWhenBadArgument) {
	EXPECT_THROW(Convert::To<bool>("test"), std::invalid_argument);
}

TEST(ConvertApi, ShouldThrowExceptionWhenOverflow) {
	EXPECT_THROW(Convert::To<bool>("5"), std::out_of_range);
}


//-----------------------------------------------------------------------------
// Test TryTo<>()
//-----------------------------------------------------------------------------
TEST(ConvertApi, TryToShouldReturnConvertedValue) {
	EXPECT_TRUE(Convert::TryTo<int>("0").has_value());
	EXPECT_EQ(500, Convert::TryTo<int>("500").value());
}

TEST(ConvertApi, TryToShouldReturnEmptyWhenOccurredError) {
	EXPECT_FALSE(Convert::TryTo<bool>("-1"));
}

TEST(ConvertApi, TryToShouldNoThrowExceptions) {
	EXPECT_NO_THROW(Convert::TryTo<bool>("-1"));
	EXPECT_NO_THROW(Convert::TryTo<char>("10000"));
}

//-----------------------------------------------------------------------------
// Test functions ToString/ToWString (syntax sugar functions)
//-----------------------------------------------------------------------------
TEST(ConvertApi, ToString) {
	EXPECT_EQ("500", Convert::ToString(500));
}

TEST(ConvertApi, ToWString) {
	EXPECT_EQ(L"500", Convert::ToWString(500));
}

//-----------------------------------------------------------------------------
// Test registration of stream operations for Convert::Utf::UtfType
//-----------------------------------------------------------------------------
TEST(ConvertApi, ConvertUtfTypeToStream) {
	std::ostringstream oss;
	oss << Convert::Utf::UtfType::Utf16le;
	EXPECT_EQ("UTF-16LE", oss.str());
}

TEST(ConvertApi, ConvertUtfTypeFromStream) {
	std::stringstream stream("UTF-32LE");
	Convert::Utf::UtfType actual;
	stream >> actual;
	EXPECT_EQ(BitSerializer::Convert::Utf::UtfType::Utf32le, actual);
}
