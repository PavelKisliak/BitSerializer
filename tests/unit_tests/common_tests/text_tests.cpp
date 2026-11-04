/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <string>
#include <string_view>

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
// Test ToAsciiLower() / ToAsciiUpper()
//-----------------------------------------------------------------------------
TEST(AsciiCaseConversionTest, ShouldConvertLetters)
{
	EXPECT_EQ('a', Text::ToAsciiLower('A'));
	EXPECT_EQ('z', Text::ToAsciiLower('Z'));
	EXPECT_EQ('A', Text::ToAsciiUpper('a'));
	EXPECT_EQ('Z', Text::ToAsciiUpper('z'));
}

TEST(AsciiCaseConversionTest, ShouldLeaveOtherCharactersUnchanged)
{
	EXPECT_EQ('1', Text::ToAsciiLower('1'));
	EXPECT_EQ('@', Text::ToAsciiLower('@'));
	EXPECT_EQ('1', Text::ToAsciiUpper('1'));
	EXPECT_EQ('@', Text::ToAsciiUpper('@'));

	const auto nonAscii = static_cast<unsigned char>(0xC0);
	EXPECT_EQ(nonAscii, Text::ToAsciiLower(nonAscii));
	EXPECT_EQ(nonAscii, Text::ToAsciiUpper(nonAscii));
}

TEST(AsciiCaseConversionTest, ShouldConvertWideCharacters)
{
	EXPECT_EQ(u'a', Text::ToAsciiLower(u'A'));
	EXPECT_EQ(U'z', Text::ToAsciiLower(U'Z'));
	EXPECT_EQ(L'A', Text::ToAsciiUpper(L'a'));
	EXPECT_EQ(U'Z', Text::ToAsciiUpper(U'z'));
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

//-----------------------------------------------------------------------------
// Test Equal()
//-----------------------------------------------------------------------------
TEST(TextEqualTest, ShouldCompareStringsForAllLengthsAndMismatchPositions)
{
	constexpr size_t maxLength = 40;
	for (size_t length = 0; length <= maxLength; ++length)
	{
		const std::string lhs(length, 'a');
		const std::string rhs(length, 'a');
		EXPECT_TRUE(Text::Equal(lhs, rhs)) << "length " << length;

		for (size_t pos = 0; pos < length; ++pos)
		{
			std::string different = rhs;
			different[pos] = 'b';
			EXPECT_FALSE(Text::Equal(lhs, different)) << "length " << length << ", pos " << pos;
		}

		const std::string longer = rhs + 'a';
		EXPECT_FALSE(Text::Equal(lhs, longer)) << "length " << length;
	}
}

TEST(TextEqualTest, ShouldCompareStringLiteralsWithCompileTimeSize)
{
	const std::string_view empty;
	EXPECT_TRUE(Text::Equal(empty, ""));
	EXPECT_FALSE(Text::Equal(std::string_view("x"), ""));

	// 8 bytes: a single uint64 compare
	EXPECT_TRUE(Text::Equal(std::string_view("Member_0"), "Member_0"));
	EXPECT_FALSE(Text::Equal(std::string_view("Member_0"), "Member_1"));
	EXPECT_FALSE(Text::Equal(std::string_view("Member_0"), "Member_00"));

	// 12 bytes: 8 + 4
	EXPECT_TRUE(Text::Equal(std::string_view("BooleanValue"), "BooleanValue"));
	EXPECT_FALSE(Text::Equal(std::string_view("BooleanValue"), "BooleanValuE"));

	// 16 bytes: 8 + 8
	EXPECT_TRUE(Text::Equal(std::string_view("UnsignedIntValue"), "UnsignedIntValue"));
	EXPECT_FALSE(Text::Equal(std::string_view("UnsignedIntValue"), "UnsignedIntValuE"));

	// 26 bytes: 8 + 8 + 8 + 2
	EXPECT_TRUE(Text::Equal(std::string_view("StringWithLongKeyAndValue"), "StringWithLongKeyAndValue"));
	EXPECT_FALSE(Text::Equal(std::string_view("StringWithLongKeyAndValue"), "StringWithLongKeyAndValuE"));

	// 7 and 9 bytes: the 4/2/1 and 8+1 paths
	EXPECT_TRUE(Text::Equal(std::string_view("Member7"), "Member7"));
	EXPECT_FALSE(Text::Equal(std::string_view("Member7"), "Member8"));
	EXPECT_TRUE(Text::Equal(std::string_view("Member_01"), "Member_01"));
	EXPECT_FALSE(Text::Equal(std::string_view("Member_01"), "Member_02"));

	// Mutable character arrays use the same overload
	char mutableKey[] = "Member_0";
	EXPECT_TRUE(Text::Equal(std::string_view("Member_0"), mutableKey));
	EXPECT_FALSE(Text::Equal(std::string_view("Member_1"), mutableKey));

	// Reversed order: literal first, view/string second
	EXPECT_TRUE(Text::Equal("Member_0", std::string_view("Member_0")));
	EXPECT_FALSE(Text::Equal("Member_0", std::string_view("Member_1")));
	EXPECT_TRUE(Text::Equal("StringWithLongKeyAndValue", std::string("StringWithLongKeyAndValue")));
	EXPECT_FALSE(Text::Equal("StringWithLongKeyAndValue", std::string("StringWithLongKeyAndValuE")));

	// Both arguments are literals: the array/array overload resolves the call unambiguously
	EXPECT_TRUE(Text::Equal("Member_0", "Member_0"));
	EXPECT_FALSE(Text::Equal("Member_0", "Member_1"));
	EXPECT_FALSE(Text::Equal("Member_0", "Member_00"));
}

TEST(TextEqualTest, ShouldCompareStringLiteralsAtCompileTime)
{
	// Only the array/array overload is constexpr; the mixed (string_view, array) overloads
	// operate on runtime data and cannot be used in constant expressions.
	static_assert(Text::Equal("", ""));
	static_assert(!Text::Equal("", "x"));

	// 7, 8 and 9 bytes are the fixed-size load boundaries.
	static_assert(Text::Equal("1234567", "1234567"));
	static_assert(!Text::Equal("1234567", "1234560"));
	static_assert(!Text::Equal("1234567", "12345678"));
	static_assert(Text::Equal("12345678", "12345678"));
	static_assert(!Text::Equal("12345678", "12345670"));
	static_assert(Text::Equal("123456789", "123456789"));
	static_assert(!Text::Equal("123456789", "123456788"));

	static_assert(Text::Equal("Member_0", "Member_0"));
	static_assert(!Text::Equal("Member_0", "Member_1"));
	static_assert(!Text::Equal("Member_0", "Member_00"));
	static_assert(Text::Equal("StringWithLongKeyAndValue", "StringWithLongKeyAndValue"));
	SUCCEED();
}

TEST(TextEqualTest, ShouldCompareBoundaryLengthsSevenEightNine)
{
	// 7 bytes: 4 + 2 + 1 — mismatch in each fixed-size block.
	EXPECT_TRUE(Text::Equal(std::string_view("1234567"), "1234567"));
	EXPECT_FALSE(Text::Equal(std::string_view("1234567"), "0234567"));
	EXPECT_FALSE(Text::Equal(std::string_view("1234567"), "1230567"));
	EXPECT_FALSE(Text::Equal(std::string_view("1234567"), "1234067"));
	EXPECT_FALSE(Text::Equal(std::string_view("1234567"), "1234560"));

	// 8 bytes: one 8-byte compare.
	EXPECT_TRUE(Text::Equal(std::string_view("12345678"), "12345678"));
	EXPECT_FALSE(Text::Equal(std::string_view("12345678"), "02345678"));
	EXPECT_FALSE(Text::Equal(std::string_view("12345678"), "12345670"));

	// 9 bytes: 8 + 1.
	EXPECT_TRUE(Text::Equal(std::string_view("123456789"), "123456789"));
	EXPECT_FALSE(Text::Equal(std::string_view("123456789"), "023456789"));
	EXPECT_FALSE(Text::Equal(std::string_view("123456789"), "123456780"));
	EXPECT_FALSE(Text::Equal(std::string_view("123456789"), "123456788"));

	// Length mismatch around the 7/8/9 boundaries.
	EXPECT_FALSE(Text::Equal(std::string_view("1234567"), "12345678"));
	EXPECT_FALSE(Text::Equal(std::string_view("12345678"), "1234567"));
	EXPECT_FALSE(Text::Equal(std::string_view("12345678"), "123456789"));
	EXPECT_FALSE(Text::Equal(std::string_view("123456789"), "12345678"));

	// Reversed and literal/literal forms use the same fixed-size path.
	EXPECT_FALSE(Text::Equal("1234567", std::string_view("1234560")));
	EXPECT_TRUE(Text::Equal("123456789", std::string_view("123456789")));
	EXPECT_FALSE(Text::Equal("1234567", "1234560"));
	EXPECT_TRUE(Text::Equal("123456789", "123456789"));
}

//-----------------------------------------------------------------------------
// Test EqualIgnoreCase()
//-----------------------------------------------------------------------------
TEST(EqualIgnoreCaseTest, ShouldCompareStringViews)
{
	EXPECT_TRUE(Text::EqualIgnoreCase(std::string_view("HeLLo"), std::string_view("hello")));
	EXPECT_TRUE(Text::EqualIgnoreCase(std::string("ABC"), std::string("abc")));
	EXPECT_FALSE(Text::EqualIgnoreCase(std::string_view("Hello"), std::string_view("world")));
	EXPECT_FALSE(Text::EqualIgnoreCase(std::string_view("Hello"), std::string_view("hell")));
}

TEST(EqualIgnoreCaseTest, ShouldCompareWithStringLiterals)
{
	EXPECT_TRUE(Text::EqualIgnoreCase(std::string_view("HeLLo"), "hello"));
	EXPECT_TRUE(Text::EqualIgnoreCase("hello", std::string_view("HeLLo")));
	EXPECT_TRUE(Text::EqualIgnoreCase("HeLLo", "hello"));
	EXPECT_FALSE(Text::EqualIgnoreCase("Hello", "world"));
}

TEST(EqualIgnoreCaseTest, ShouldOnlyFoldAsciiLetters)
{
	EXPECT_TRUE(Text::EqualIgnoreCase("a1-b2", "A1-B2"));
	EXPECT_FALSE(Text::EqualIgnoreCase("abc", "abC "));
	EXPECT_FALSE(Text::EqualIgnoreCase("a@", "a`"));
	EXPECT_FALSE(Text::EqualIgnoreCase("a[", "a{"));

	const std::string nonAsciiA = "\xC3\x84";	// UTF-8 "Ä"
	const std::string nonAsciiB = "\xC3\xA4";	// UTF-8 "ä"
	EXPECT_TRUE(Text::EqualIgnoreCase(nonAsciiA, nonAsciiA));
	EXPECT_FALSE(Text::EqualIgnoreCase(nonAsciiA, nonAsciiB));
}

TEST(EqualIgnoreCaseTest, ShouldWorkAtCompileTime)
{
	static_assert(Text::EqualIgnoreCase("HeLLo", "hello"));
	static_assert(!Text::EqualIgnoreCase("Hello", "world"));
	static_assert(!Text::EqualIgnoreCase("Hello", "hell"));
}

TEST(EqualIgnoreCaseTest, ShouldSupportWideCharacterTypes)
{
	EXPECT_TRUE(Text::EqualIgnoreCase(std::wstring_view(L"HeLLo"), std::wstring_view(L"hello")));
	EXPECT_FALSE(Text::EqualIgnoreCase(std::wstring_view(L"Hello"), std::wstring_view(L"world")));

	EXPECT_TRUE(Text::EqualIgnoreCase(std::u16string_view(u"ABC"), u"abc"));
	EXPECT_TRUE(Text::EqualIgnoreCase(u"abc", std::u16string_view(u"ABC")));
	EXPECT_TRUE(Text::EqualIgnoreCase(U"Hello", U"hello"));
}

TEST(EqualIgnoreCaseTest, ShouldSupportWideCompileTimeComparison)
{
	static_assert(Text::EqualIgnoreCase(u"HeLLo", u"hello"));
	static_assert(Text::EqualIgnoreCase(U"HELLO", U"hello"));
	static_assert(!Text::EqualIgnoreCase(L"Hello", L"world"));
}
