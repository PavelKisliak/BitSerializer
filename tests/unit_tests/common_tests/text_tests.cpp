/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/common/text.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test IsWhitespace()
//-----------------------------------------------------------------------------
TEST(IsWhitespaceTest, CheckAsciiWhitespace)
{
    EXPECT_TRUE(Text::IsWhitespace(' '));
    EXPECT_TRUE(Text::IsWhitespace('\t'));
    EXPECT_TRUE(Text::IsWhitespace('\n'));
    EXPECT_TRUE(Text::IsWhitespace('\r'));
}

TEST(IsWhitespaceTest, CheckNonWhitespace)
{
    EXPECT_FALSE(Text::IsWhitespace('A'));
    EXPECT_FALSE(Text::IsWhitespace('1'));
    EXPECT_FALSE(Text::IsWhitespace('@'));
}

TEST(IsWhitespaceTest, CheckWideCharacters)
{
    EXPECT_TRUE(Text::IsWhitespace(L' '));
    EXPECT_TRUE(Text::IsWhitespace(u'\t'));
    EXPECT_FALSE(Text::IsWhitespace(U'A'));
}

//-----------------------------------------------------------------------------
// Test TrimWhitespace() for mutable strings
//-----------------------------------------------------------------------------
TEST(TrimWhitespaceTest, ShouldTrimUtf8String)
{
	std::string testValue = "\t test \t\n";
	Text::TrimWhitespace(testValue);
	EXPECT_EQ("test", testValue);
}

TEST(TrimWhitespaceTest, ShouldTrimUtf16String)
{
	std::u16string testValue = u"Hello world!\t\n";
	Text::TrimWhitespace(testValue);
	EXPECT_EQ(u"Hello world!", testValue);
}

TEST(TrimWhitespaceTest, ShouldTrimUtf32String)
{
	std::u32string testValue = U"\t t \t\n";
	Text::TrimWhitespace(testValue);
	EXPECT_EQ(U"t", testValue);
}

TEST(TrimWhitespaceTest, ShouldHandleSingleCharacterString)
{
	std::string testValue = "A";
	Text::TrimWhitespace(testValue);
	EXPECT_EQ("A", testValue);
}

TEST(TrimWhitespaceTest, ShouldHandleAllWhitespaceString)
{
	std::string testValue = " \t\n\r ";
	Text::TrimWhitespace(testValue);
	EXPECT_EQ("", testValue);
}

TEST(TrimWhitespaceTest, ShouldIgnoreEmptyString)
{
	std::string testValue;
	Text::TrimWhitespace(testValue);
	EXPECT_EQ("", testValue);
}

//-----------------------------------------------------------------------------
// Test TrimWhitespace() for std::basic_string_view
//-----------------------------------------------------------------------------
TEST(TrimWhitespaceTest, ShouldTrimUtf8StringView)
{
	std::string_view testValue = "\t test \t\n";
	Text::TrimWhitespace(testValue);
	EXPECT_EQ("test", testValue);
}

TEST(TrimWhitespaceTest, ShouldTrimUtf16StringView)
{
	std::u16string_view testValue = u"Hello world!\t\n";
	Text::TrimWhitespace(testValue);
	EXPECT_EQ(u"Hello world!", testValue);
}

TEST(TrimWhitespaceTest, ShouldTrimUtf32StringView)
{
	std::u32string_view testValue = U"\t t \t\n";
	Text::TrimWhitespace(testValue);
	EXPECT_EQ(U"t", testValue);
}

TEST(TrimWhitespaceTest, ShouldHandleSingleCharacterStringView)
{
	std::string_view testValue = "A";
	Text::TrimWhitespace(testValue);
	EXPECT_EQ("A", testValue);
}

TEST(TrimWhitespaceTest, ShouldHandleAllWhitespaceStringView)
{
	std::string_view testValue = " \t\n\r ";
	Text::TrimWhitespace(testValue);
	EXPECT_EQ("", testValue);
}

TEST(TrimWhitespaceTest, ShouldIgnoreEmptyStringView)
{
	std::string_view testValue;
	Text::TrimWhitespace(testValue);
	EXPECT_EQ("", testValue);
}
