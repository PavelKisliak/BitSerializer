/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "test_helpers/common_test_entities.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test To<>()
//-----------------------------------------------------------------------------
TEST(ConvertApi, ShouldConvertFromRawCString) {
	EXPECT_EQ(-100500, Convert::To<int32_t>(std::string_view("  -100500  ")));
	EXPECT_EQ(-100500, Convert::To<int32_t>(std::wstring_view(L"  -100500  ")));
	EXPECT_EQ(-100500, Convert::To<int32_t>(std::u16string_view(u"  -100500  ")));
	EXPECT_EQ(-100500, Convert::To<int32_t>(std::u32string_view(U"  -100500  ")));
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

TEST(ConvertApi, ShouldReturnTheSamePointerWhenConvertToSameType) {
	const char* expectedPtr = "test";
	EXPECT_EQ(expectedPtr, Convert::To<const char*>(expectedPtr));
}

TEST(ConvertApi, ShouldReturnTheSameValueWhenConvertToSameType) {
	EXPECT_EQ(500, Convert::To<int>(500));
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
// Test registration of stream operations for Convert::UtfType
//-----------------------------------------------------------------------------
TEST(ConvertApi, ConvertUtfTypeToStream) {
	std::ostringstream oss;
	oss << BitSerializer::Convert::UtfType::Utf16le;
	EXPECT_EQ("UTF-16LE", oss.str());
}

TEST(ConvertApi, ConvertUtfTypeFromStream) {
	std::stringstream stream("UTF-32LE");
	BitSerializer::Convert::UtfType actual;
	stream >> actual;
	EXPECT_EQ(BitSerializer::Convert::UtfType::Utf32le, actual);
}
