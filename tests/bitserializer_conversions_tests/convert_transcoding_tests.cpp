/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "tests/test_helpers/common_test_entities.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test conversions any UTF string to any other UTF format
//-----------------------------------------------------------------------------
TEST(TranscodeStrings, StringToTheSameStringType) {
	EXPECT_EQ("Hello world!", Convert::ToString("Hello world!"));
	EXPECT_EQ(U"Hello world!", Convert::To<std::u32string>(U"Hello world!"));
}

TEST(TranscodeStrings, DecodeUtf8ToAnyStringType) {
	EXPECT_EQ(L"Привет мир!", Convert::ToWString(u8"Привет мир!"));
	EXPECT_EQ(u"Привет мир!", Convert::To<std::u16string>(u8"Привет мир!"));
	EXPECT_EQ(U"Привет мир!", Convert::To<std::u32string>(u8"Привет мир!"));
}

TEST(TranscodeStrings, EncodeUtf8FromAnyStringType) {
	EXPECT_EQ(u8"Привет мир!", Convert::ToString(L"Привет мир!"));
	EXPECT_EQ(u8"Привет мир!", Convert::ToString(u"Привет мир!"));
	EXPECT_EQ(u8"Привет мир!", Convert::ToString(U"Привет мир!"));
}

TEST(TranscodeStrings, DecodeUtf16ToUtf32) {
	EXPECT_EQ(U"Привет мир!", Convert::To<std::u32string>(u"Привет мир!"));
}

TEST(TranscodeStrings, EncodeUtf16FromUtf32) {
	EXPECT_EQ(u"Привет мир!", Convert::To<std::u16string>(U"Привет мир!"));
}
