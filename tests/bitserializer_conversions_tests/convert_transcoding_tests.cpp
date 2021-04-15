/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "test_helpers/common_test_entities.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test conversion std::string to std::wstring and vice versa
//-----------------------------------------------------------------------------
TEST(TranscodeStrings, StringToWString) {
	EXPECT_EQ(L"Test", Convert::ToWString("Test"));
}

TEST(TranscodeStrings, WStringToString) {
	EXPECT_EQ("Test", Convert::ToString(L"Test"));
}

TEST(TranscodeStrings, StringFromWString) {
	EXPECT_EQ("Test", Convert::To<std::string>(L"Test"));
}

TEST(TranscodeStrings, WStringFromString) {
	EXPECT_EQ(L"Test", Convert::To<std::wstring>("Test"));
}

TEST(TranscodeStrings, WStringToUtf8) {
	EXPECT_EQ(u8"Привет мир!", Convert::ToString(L"Привет мир!"));
}

TEST(TranscodeStrings, Utf8ToString) {
	EXPECT_EQ(L"Привет мир!", Convert::ToWString(u8"Привет мир!"));
}
