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
// UTF-16 LE: Tests for encoding string
//-----------------------------------------------------------------------------
TEST(Utf16LeEncodeTest, ShouldEncodeUtf16LeFromAnsi)
{
	// Arrange
	std::u16string outString;
	const std::string source = "Hello world!";

	// Act
	const auto result = Convert::Utf::Utf16Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(u"Hello world!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeEncodeTest, ShouldEncodeUtf16LeFromUtf8)
{
	// Arrange
	std::u16string outString;
	const std::string source = UTF8("Привет мир!");

	// Act
	const auto result = Convert::Utf::Utf16Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(u"Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeEncodeTest, ShouldEncodeUtf16LeFromUtf8Surrogates)
{
	// Arrange
	std::u16string outString;
	const std::string source = UTF8("😀😎🙋");

	// Act
	const auto result = Convert::Utf::Utf16Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(u"😀😎🙋"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeEncodeTest, ShouldEncodeUtf16LeFromUtf16)
{
	// Arrange
	std::u16string outString;
	const std::u16string source = u"Привет мир!";

	// Act
	const auto result = Convert::Utf::Utf16Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(u"Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeEncodeTest, ShouldEncodeUtf16LeFromUtf16Surrogates)
{
	// Arrange
	std::u16string outString;
	const std::u16string source = u"😀😎🙋";

	// Act
	const auto result = Convert::Utf::Utf16Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(u"😀😎🙋"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeEncodeTest, ShouldEncodeUtf16LeFromUtf32)
{
	// Arrange
	std::u16string outString;
	const std::u32string source = U"世界，您好！";

	// Act
	const auto result = Convert::Utf::Utf16Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(u"世界，您好！"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeEncodeTest, ShouldEncodeUtf16LeSurrogatesFromUtf32)
{
	// Arrange
	std::u16string outString;
	const std::u32string source = U"😀😎🙋";

	// Act
	const auto result = Convert::Utf::Utf16Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(NativeStringToLittleEndian(u"😀😎🙋"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeEncodeTest, ShouldHandleUnexpectedEndWhenCroppedSurrogatePair)
{
	// Arrange
	std::u16string outString;
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string source = u"test" + croppedSequence;
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf16Le::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ(NativeStringToLittleEndian(u"test"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(expectedPos), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}


//-----------------------------------------------------------------------------
// UTF-16 LE: Tests decoding string
//-----------------------------------------------------------------------------
TEST(Utf16LeDecodeTest, ShouldDecodeUtf16LeToAnsi)
{
	// Arrange
	std::string outString;
	const std::u16string source = NativeStringToLittleEndian(u"Hello world!");

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ("Hello world!", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldDecodeUtf16LeToUtf8)
{
	// Arrange
	std::string outString;
	const std::u16string source = NativeStringToLittleEndian(u"Привет мир!");

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldDecodeUtf16LeToUtf16)
{
	// Arrange
	std::u16string outString;
	const std::u16string source = NativeStringToLittleEndian(u"世界，您好！");

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"世界，您好！", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf16LeWithSurrogates)
{
	// Arrange
	std::u16string outString;
	const std::u16string source = NativeStringToLittleEndian(u"😀😎🙋");

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"😀😎🙋", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldDecodeUtf16LeToUtf32)
{
	// Arrange
	std::u32string outString;
	const std::u16string source = NativeStringToLittleEndian(u"世界，您好！");

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"世界，您好！", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf32LeWithSurrogates)
{
	// Arrange
	std::u32string outString;
	const std::u16string source = NativeStringToLittleEndian(u"😀😎🙋");

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"😀😎🙋", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldWriteErrorMarkWhenSurrogateStartsWithWrongCode)
{
	// Arrange
	std::u32string outString;
	const std::u16string wrongStartCodes({ Convert::Utf::UnicodeTraits::LowSurrogatesEnd, Convert::Utf::UnicodeTraits::LowSurrogatesStart });
	const std::u16string source = NativeStringToLittleEndian(wrongStartCodes + u"test" + wrongStartCodes);

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"☐☐test☐☐", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(4U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldWriteErrorMarkWhenNoSecondCodeInSurrogate)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = NativeStringToLittleEndian(notFullSurrogatePair + u"test");

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"☐test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldWriteCustomErrorMarkWhenError)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = NativeStringToLittleEndian(notFullSurrogatePair + u"test");

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip, U"<ERROR>");

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"<ERROR>test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldSkipWrongSequenceWhenErrorMarkIsEmpty)
{
	// Arrange
	std::u32string outString;
	const std::u16string notFullSurrogatePair({ Convert::Utf::UnicodeTraits::HighSurrogatesStart });
	const std::u16string source = NativeStringToLittleEndian(notFullSurrogatePair + u"test");

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip, U"");

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldHandleUnexpectedEndWhenCroppedSurrogatePairAtEndWhenDecodeToUtf8)
{
	// Arrange
	std::string outString;
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string source = NativeStringToLittleEndian(u"test" + croppedSequence);
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ("test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(expectedPos), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldHandleUnexpectedEndWhenCroppedSurrogatePairAtEndWhenDecodeToUtf16)
{
	// Arrange
	std::u16string outString;
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string source = NativeStringToLittleEndian(u"test" + croppedSequence);
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ(u"test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(expectedPos), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(Utf16LeDecodeTest, ShouldHandleUnexpectedEndWhenCroppedSurrogatePairAtEndWhenDecodeToUtf32)
{
	// Arrange
	std::u32string outString;
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string source = NativeStringToLittleEndian(u"test" + croppedSequence);
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf16Le::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ(U"test", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(expectedPos), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

#pragma warning(pop)
