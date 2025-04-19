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
// UTF-16 BE: Tests for encoding string
//-----------------------------------------------------------------------------
TEST(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromAnsi)
{
	// Arrange
	std::u16string outString;
	const std::string source = "Hello world!";

	// Act
	const auto result = Convert::Utf::Utf16Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(u"Hello world!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf8)
{
	// Arrange
	std::u16string outString;
	const std::string source = UTF8("Привет мир!");

	// Act
	const auto result = Convert::Utf::Utf16Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(u"Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf8Surrogates)
{
	// Arrange
	std::u16string outString;
	const std::string source = UTF8("😀😎🙋");

	// Act
	const auto result = Convert::Utf::Utf16Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(u"😀😎🙋"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf16)
{
	// Arrange
	std::u16string outString;
	const std::u16string source = u"Привет мир!";

	// Act
	const auto result = Convert::Utf::Utf16Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(u"Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf16Surrogates)
{
	// Arrange
	std::u16string outString;
	const std::u16string source = u"😀😎🙋";

	// Act
	const auto result = Convert::Utf::Utf16Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(u"😀😎🙋"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf32)
{
	// Arrange
	std::u16string outString;
	const std::u32string source = U"世界，您好！";

	// Act
	const auto result = Convert::Utf::Utf16Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(u"世界，您好！"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeEncodeTest, ShouldEncodeUtf16BeSurrogatesFromUtf32)
{
	// Arrange
	std::u16string outString;
	const std::u32string source = U"😀😎🙋";

	// Act
	const auto result = Convert::Utf::Utf16Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToBigEndian(u"😀😎🙋"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeEncodeTest, ShouldHandleUnexpectedEndWhenCroppedSurrogatePair)
{
	// Arrange
	std::u16string outString;
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string source = u"test" + croppedSequence;
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf16Be::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ(NativeStringToBigEndian(u"test"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(expectedPos), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

//-----------------------------------------------------------------------------
// UTF-16 BE: Tests decoding string
//-----------------------------------------------------------------------------
TEST(Utf16BeDecodeTest, ShouldDecodeUtf16BeToAnsi)
{
	// Arrange
	std::string outString;
	const std::u16string source = NativeStringToBigEndian(u"Hello world!");

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ("Hello world!", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf8)
{
	// Arrange
	std::string outString;
	const std::u16string source = NativeStringToBigEndian(u"Привет мир!");

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf16)
{
	// Arrange
	std::u16string outString;
	const std::u16string source = NativeStringToBigEndian(u"世界，您好！");

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"世界，您好！", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf16WithSurrogates)
{
	// Arrange
	std::u16string outString;
	const std::u16string source = NativeStringToBigEndian(u"😀😎🙋");

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"😀😎🙋", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf32)
{
	// Arrange
	std::u32string outString;
	const std::u16string source = NativeStringToBigEndian(u"世界，您好！");

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"世界，您好！", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf32WithSurrogates)
{
	// Arrange
	std::u32string outString;
	const std::u16string source = NativeStringToBigEndian(u"😀😎🙋");

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"😀😎🙋", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldWriteErrorMarkWhenSurrogateStartsWithWrongCode)
{
	// Arrange
	std::u32string outString;
	const std::u16string wrongStartCodes({ Convert::Utf::UnicodeTraits::LowSurrogatesEnd, Convert::Utf::UnicodeTraits::LowSurrogatesStart });
	const std::u16string source = NativeStringToBigEndian(wrongStartCodes + u"test" + wrongStartCodes);

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"☐☐test☐☐", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(4U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldWriteErrorMarkWhenNoSecondCodeInSurrogate)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = NativeStringToBigEndian(notFullSurrogatePair + u"test");

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"☐test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldWriteCustomErrorMarkWhenError)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = NativeStringToBigEndian(notFullSurrogatePair + u"test");

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip, U"<ERROR>");

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"<ERROR>test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldSkipWrongSequenceWhenErrorMarkIsEmpty)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = NativeStringToBigEndian(notFullSurrogatePair + u"test");

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip, U"");

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldHandleUnexpectedEndWhenCroppedSurrogatePairAtEndWhenDecodeToUtf8)
{
	// Arrange
	std::string outString;
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string source = NativeStringToBigEndian(u"test" + croppedSequence);
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ("test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(expectedPos), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldHandleUnexpectedEndWhenCroppedSurrogatePairAtEndWhenDecodeToUtf16)
{
	// Arrange
	std::u16string outString;
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string source = NativeStringToBigEndian(u"test" + croppedSequence);
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ(u"test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(expectedPos), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16BeDecodeTest, ShouldHandleUnexpectedEndWhenCroppedSurrogatePairAtEndWhenDecodeToUtf32)
{
	// Arrange
	std::u32string outString;
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string source = NativeStringToBigEndian(u"test" + croppedSequence);
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf16Be::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ(U"test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(expectedPos), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

#pragma warning(pop)
