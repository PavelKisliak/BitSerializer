﻿/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"
#include "testing_tools/string_utils.h"

using namespace BitSerializer;

#pragma warning(push)
#pragma warning(disable: 4566)
//-----------------------------------------------------------------------------
// UTF-32 LE: Tests for encoding string
//-----------------------------------------------------------------------------
TEST(Utf32LeEncodeTest, ShouldEncodeUtf32FromAnsi)
{
	// Arrange
	std::u32string outString;
	const std::string source = "Hello world!";

	// Act
	const auto result = Convert::Utf::Utf32Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(U"Hello world!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32LeEncodeTest, ShouldEncodeUtf32FromUtf8)
{
	// Arrange
	std::u32string outString;
	const std::string source = UTF8("Привет мир!");

	// Act
	const auto result = Convert::Utf::Utf32Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(U"Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32LeEncodeTest, ShouldEncodeUtf32FromUtf16)
{
	// Arrange
	std::u32string outString;
	const std::u16string source = u"Привет мир!";

	// Act
	const auto result = Convert::Utf::Utf32Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(U"Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32LeEncodeTest, ShouldEncodeUtf32FromUtf16Surrogates)
{
	// Arrange
	std::u32string outString;
	const std::u16string source = u"😀😎🙋";

	// Act
	const auto result = Convert::Utf::Utf32Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(U"😀😎🙋"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32LeEncodeTest, ShouldEncodeUtf32FromUtf32AsIs)
{
	// Arrange
	std::u32string outString;
	const std::u32string source = U"世界，您好！";

	// Act
	const auto result = Convert::Utf::Utf32Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(U"世界，您好！"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32LeEncodeTest, ShouldWriteErrorMarkWhenSurrogateStartsWithWrongCode)
{
	// Arrange
	std::u32string outString;
	const std::u16string wrongStartCodes({ Convert::Utf::UnicodeTraits::LowSurrogatesStart });
	const std::u16string source = wrongStartCodes + u"test" + wrongStartCodes;

	// Act
	const auto result = Convert::Utf::Utf32Le::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(U"☐test☐"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(2U, result.InvalidSequencesCount);
}

TEST(Utf32LeEncodeTest, ShouldWriteErrorMarkWhenNoSecondCodeInSurrogate)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = notFullSurrogatePair + u"test";

	// Act
	const auto result = Convert::Utf::Utf32Le::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(U"☐test"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

TEST(Utf32LeEncodeTest, ShouldSkipWrongSequenceWhenErrorMarkIsEmpty)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = u"test" + notFullSurrogatePair + u"123";

	// Act
	const auto result = Convert::Utf::Utf32Le::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip, U"");

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(U"test123"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

TEST(Utf32LeEncodeTest, ShouldHandlePolicyThrowError)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = u"test" + notFullSurrogatePair + u"test";

	// Act
	const auto result = Convert::Utf::Utf32Le::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::ThrowError);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(NativeStringToLittleEndian(U"test"), outString);
	EXPECT_EQ(4, std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

//-----------------------------------------------------------------------------
// UTF-32 LE: Tests decoding string
//-----------------------------------------------------------------------------
TEST(Utf32LeDecodeTest, ShouldDecodeUtf32ToAnsi)
{
	// Arrange
	std::string outString;
	const std::u32string source = NativeStringToLittleEndian(U"Hello world!");

	// Act
	const auto result = Convert::Utf::Utf32Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ("Hello world!", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32LeDecodeTest, ShouldDecodeUtf32ToUtf8)
{
	// Arrange
	std::string outString;
	const std::u32string source = NativeStringToLittleEndian(U"Привет мир!");

	// Act
	const auto result = Convert::Utf::Utf32Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32LeDecodeTest, ShouldDecodeUtf32ToUtf16)
{
	// Arrange
	std::u16string outString;
	const std::u32string source = NativeStringToLittleEndian(U"世界，您好！");

	// Act
	const auto result = Convert::Utf::Utf32Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"世界，您好！", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32LeDecodeTest, ShouldDecodeUtf32ToUtf16WithSurrogates)
{
	// Arrange
	std::u16string outString;
	const std::u32string source = NativeStringToLittleEndian(U"😀😎🙋");

	// Act
	const auto result = Convert::Utf::Utf32Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"😀😎🙋", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32LeDecodeTest, ShouldDecodeUtf32ToUtf32AsIs)
{
	// Arrange
	std::u32string outString;
	const std::u32string source = NativeStringToLittleEndian(U"世界，您好！");

	// Act
	const auto result = Convert::Utf::Utf32Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"世界，您好！", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

#pragma warning(pop)
