/*******************************************************************************
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
// UTF-32 BE: Tests for encoding string
//-----------------------------------------------------------------------------
TEST(Utf32BeEncodeTest, ShouldEncodeUtf32LeFromAnsi)
{
	// Arrange
	std::u32string outString;
	const std::string source = "Hello world!";

	// Act
	const auto result = Convert::Utf::Utf32Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(U"Hello world!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32BeEncodeTest, ShouldEncodeUtf32LeFromUtf8)
{
	// Arrange
	std::u32string outString;
	const std::string source = UTF8("Привет мир!");

	// Act
	const auto result = Convert::Utf::Utf32Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(U"Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32BeEncodeTest, ShouldEncodeUtf32LeFromUtf16)
{
	// Arrange
	std::u32string outString;
	const std::u16string source = u"Привет мир!";

	// Act
	const auto result = Convert::Utf::Utf32Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(U"Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32BeEncodeTest, ShouldEncodeUtf32LeFromUtf16Surrogates)
{
	// Arrange
	std::u32string outString;
	const std::u16string source = u"😀😎🙋";

	// Act
	const auto result = Convert::Utf::Utf32Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(U"😀😎🙋"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32BeEncodeTest, ShouldEncodeUtf32LeFromUtf32)
{
	// Arrange
	std::u32string outString;
	const std::u32string source = U"世界，您好！";

	// Act
	const auto result = Convert::Utf::Utf32Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(U"世界，您好！"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32BeEncodeTest, ShouldWriteErrorMarkWhenSurrogateStartsWithWrongCode)
{
	// Arrange
	std::u32string outString;
	const std::u16string wrongStartCodes({ Convert::Utf::UnicodeTraits::LowSurrogatesStart });
	const std::u16string source = wrongStartCodes + u"test" + wrongStartCodes;

	// Act
	const auto result = Convert::Utf::Utf32Be::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(U"☐test☐"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(2U, result.InvalidSequencesCount);
}

TEST(Utf32BeEncodeTest, ShouldWriteErrorMarkWhenNoSecondCodeInSurrogate)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = notFullSurrogatePair + u"test";

	// Act
	const auto result = Convert::Utf::Utf32Be::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(U"☐test"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

TEST(Utf32BeEncodeTest, ShouldSkipWrongSequenceWhenErrorMarkIsEmpty)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = u"test" + notFullSurrogatePair + u"123";

	// Act
	const auto result = Convert::Utf::Utf32Be::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip, U"");

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(U"test123"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

TEST(Utf32BeEncodeTest, ShouldHandlePolicyThrowError)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = u"test" + notFullSurrogatePair + u"test";

	// Act
	const auto result = Convert::Utf::Utf32Be::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::ThrowError);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(NativeStringToBigEndian(U"test"), outString);
	EXPECT_EQ(4, std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

//-----------------------------------------------------------------------------
// UTF-32 BE: Tests decoding string
//-----------------------------------------------------------------------------
TEST(Utf32BeDecodeTest, ShouldDecodeUtf32BeToAnsi)
{
	// Arrange
	std::string outString;
	const std::u32string source = NativeStringToBigEndian(U"Hello world!");

	// Act
	const auto result = Convert::Utf::Utf32Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ("Hello world!", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32BeDecodeTest, ShouldDecodeUtf32BeToUtf8)
{
	// Arrange
	std::string outString;
	const std::u32string source = NativeStringToBigEndian(U"Привет мир!");

	// Act
	const auto result = Convert::Utf::Utf32Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32BeDecodeTest, ShouldDecodeUtf32BeToUtf16)
{
	// Arrange
	std::u16string outString;
	const std::u32string source = NativeStringToBigEndian(U"世界，您好！");

	// Act
	const auto result = Convert::Utf::Utf32Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"世界，您好！", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32BeDecodeTest, ShouldDecodeUtf32ToUtf16WithSurrogates)
{
	// Arrange
	std::u16string outString;
	const std::u32string source = NativeStringToBigEndian(U"😀😎🙋");

	// Act
	const auto result = Convert::Utf::Utf32Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"😀😎🙋", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf32BeDecodeTest, ShouldDecodeUtf32BeToUtf32Le)
{
	// Arrange
	std::u32string outString;
	const std::u32string source = NativeStringToBigEndian(U"世界，您好！");

	// Act
	const auto result = Convert::Utf::Utf32Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"世界，您好！", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

#pragma warning(pop)
