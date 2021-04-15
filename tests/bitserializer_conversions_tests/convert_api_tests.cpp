/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
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
}

TEST(ConvertApi, ShouldConvertFromStringView) {
	EXPECT_EQ(-100500, Convert::To<int32_t>(std::string_view("  -100500  ")));
	EXPECT_EQ(-100500, Convert::To<int32_t>(std::wstring_view(L"  -100500  ")));
}

TEST(ConvertApi, ShouldConvertStdString) {
	EXPECT_EQ(100500, Convert::To<int32_t>(std::string("  100500  ")));
	EXPECT_EQ(100500, Convert::To<int32_t>(std::wstring(L"  100500  ")));
}

TEST(ConvertApi, ShouldReturnTheSamePointerWhenConvertToSameType) {
	const char* expectedPtr = "test";
	EXPECT_EQ(expectedPtr, Convert::To<const char*>(expectedPtr));
}

TEST(ConvertApi, ShouldReturnTheSameValueWhenConvertToSameType) {
	EXPECT_EQ(500, Convert::To<int>(500));
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
// Test conversion of classes and enums to ostringstream
//-----------------------------------------------------------------------------
TEST(ConvertApi, ConvertClassToStream) {
	std::ostringstream oss;
	oss << TestPointClass(543, -345);
	EXPECT_EQ("543 -345", oss.str());
}

TEST(ConvertApi, ConvertClassToWStream) {
	std::wostringstream oss;
	oss << TestPointClass(543, -345);
	EXPECT_EQ(L"543 -345", oss.str());
}

TEST(ConvertApi, ConvertEnumToStream) {
	std::ostringstream oss;
	oss << TestEnum::Five;
	EXPECT_EQ("Five", oss.str());
}

TEST(ConvertApi, ConvertEnumToWStream) {
	std::wostringstream oss;
	oss << TestEnum::Five;
	EXPECT_EQ(L"Five", oss.str());
}
