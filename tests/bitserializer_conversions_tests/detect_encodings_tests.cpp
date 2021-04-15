/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of UTF encodings detection by BOM (Byte Order Mark)
//-----------------------------------------------------------------------------
// Common tests for different encoding types
//-----------------------------------------------------------------------------
TEST(UtfDetectEncodingsTest, ShouldReturnFalseWhenInputDataIsEmpty) {
	EXPECT_FALSE(Convert::StartsWithBom<Convert::Utf8>(""));
	EXPECT_FALSE(Convert::StartsWithBom<Convert::Utf32Be>(""));
	EXPECT_FALSE(Convert::StartsWithBom<Convert::Utf32Le>(""));
}

TEST(UtfDetectEncodingsTest, ShouldReturnUtf8WhenNoBom) {
	std::stringstream strStream;
	strStream << "test_text";
	EXPECT_EQ(Convert::UtfType::Utf8, Convert::DetectEncoding(strStream));
}

TEST(UtfDetectEncodingsTest, ShouldSkipBomWhenFoundWhenPassedTrue) {
	const std::string expectedText = "test_text";
	std::stringstream strStream;
	strStream << "\xEF\xBB\xBF" << expectedText;
	Convert::DetectEncoding(strStream, true);

	const std::string actualText(std::istreambuf_iterator<char>(strStream), {});
	EXPECT_EQ(expectedText, actualText);
}

TEST(UtfDetectEncodingsTest, ShouldNotSkipBomWhenPassedFalse) {
	const std::string expectedText = "\xEF\xBB\xBFtest_text";
	std::stringstream strStream;
	strStream << expectedText;
	Convert::DetectEncoding(strStream, false);

	const std::string actualText(std::istreambuf_iterator<char>(strStream), {});
	EXPECT_EQ(expectedText, actualText);
}

//-----------------------------------------------------------------------------
// UTF-8 detect
//-----------------------------------------------------------------------------
TEST(UtfDetectEncodingsTest, Utf8_ShouldReturnTrueWhenStartsWithValidBom) {
	static const std::string testStr = "\xEF\xBB\xBF";
	EXPECT_TRUE(Convert::StartsWithBom<Convert::Utf8>(testStr));
}

TEST(UtfDetectEncodingsTest, Utf8_ShouldReturnFalseWhenBomIsNotFull) {
	static const std::string testStr = "\xEF\xBB_test";
	EXPECT_FALSE(Convert::StartsWithBom<Convert::Utf8>(testStr));
}

TEST(UtfDetectEncodingsTest, Utf8_ShouldDetectEncoding) {
	std::stringstream strStream;
	strStream << "\xEF\xBB\xBF_test";
	EXPECT_EQ(Convert::UtfType::Utf8, Convert::DetectEncoding(strStream));
}

//-----------------------------------------------------------------------------
// UTF-16 LE detect
//-----------------------------------------------------------------------------
TEST(UtfDetectEncodingsTest, Utf16Le_ShouldReturnTrueWhenStartsWithValidBom) {
	static const std::string testStr = "\xFF\xFE";
	EXPECT_TRUE(Convert::StartsWithBom<Convert::Utf16Le>(testStr));
}

TEST(UtfDetectEncodingsTest, Utf16Le_ShouldReturnFalseWhenBomIsNotFull) {
	static const std::string testStr = "\xFF-";
	EXPECT_FALSE(Convert::StartsWithBom<Convert::Utf16Le>(testStr));
}

//-----------------------------------------------------------------------------
// UTF-16 BE detect
//-----------------------------------------------------------------------------
TEST(UtfDetectEncodingsTest, Utf16Be_ShouldReturnTrueWhenStartsWithValidBom) {
	static const std::string testStr = "\xFE\xFF";
	EXPECT_TRUE(Convert::StartsWithBom<Convert::Utf16Be>(testStr));
}

TEST(UtfDetectEncodingsTest, Utf16Be_ShouldReturnFalseWhenBomIsNotFull) {
	static const std::string testStr = "\xFE-";
	EXPECT_FALSE(Convert::StartsWithBom<Convert::Utf16Be>(testStr));
}

//-----------------------------------------------------------------------------
// UTF-32 LE detect
//-----------------------------------------------------------------------------
TEST(UtfDetectEncodingsTest, Utf32Le_ShouldReturnTrueWhenStartsWithValidBom) {
	static const std::string testStr = { char(0xFF), char(0xFE), char(0x00), char(0x00) };
	EXPECT_TRUE(Convert::StartsWithBom<Convert::Utf32Le>(testStr));
}

TEST(UtfDetectEncodingsTest, Utf32Le_ShouldReturnFalseWhenBomIsNotFull) {
	static const std::string testStr = { char(0xFF), char(0xFE), char(0x00), '-' };
	EXPECT_FALSE(Convert::StartsWithBom<Convert::Utf32Le>(testStr));
}

//-----------------------------------------------------------------------------
// UTF-32 BE detect
//-----------------------------------------------------------------------------
TEST(UtfDetectEncodingsTest, Utf32Be_ShouldReturnTrueWhenStartsWithValidBom) {
	static const std::string testStr = { char(0x00), char(0x00), char(0xFE), char(0xFF) };
	EXPECT_TRUE(Convert::StartsWithBom<Convert::Utf32Be>(testStr));
}

TEST(UtfDetectEncodingsTest, Utf32Be_ShouldReturnFalseWhenBomIsNotFull) {
	static const std::string testStr = { char(0x00), char(0x00), char(0xFE) };
	EXPECT_FALSE(Convert::StartsWithBom<Convert::Utf32Be>(testStr));
}
