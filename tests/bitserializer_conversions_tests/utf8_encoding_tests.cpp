/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "testing_tools/string_utils.h"
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;

#pragma warning(push)
#pragma warning(disable: 4566)
//-----------------------------------------------------------------------------
// Tests for encoding string to UTF-8
//-----------------------------------------------------------------------------
TEST(Utf8EncodeTest, ShouldEncodeUtf8WhenUsedOneOctet)
{
	// Arrange
	std::string outString;
	const std::wstring source = L"Hello world!";

	// Act
	const auto result = Convert::Utf::Utf8::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("Hello world!"), outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8EncodeTest, ShouldEncodeUtf8WhenUsedTwoOctets)
{
	// Arrange
	std::string outString;
	const std::wstring source = L"Привет мир!";

	// Act
	const auto result = Convert::Utf::Utf8::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("Привет мир!"), outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8EncodeTest, ShouldEncodeUtf8WhenUsedThreeOctets)
{
	// Arrange
	std::string outString;
	const std::u32string source = U"世界，您好！";

	// Act
	const auto result = Convert::Utf::Utf8::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("世界，您好！"), outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8EncodeTest, ShouldEncodeUtf8WhenUsedFourOctets)
{
	// Arrange
	std::string outString;
	const std::u32string source = U"😀😎🙋";

	// Act
	const auto result = Convert::Utf::Utf8::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("😀😎🙋"), outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8EncodeTest, ShouldEncodeUtf8WithDecodingSurrogatePairs)
{
	// Arrange
	std::string outString;
	const std::u16string surrogatePair = u"\xD83D\xDE00";
	const std::u16string source = surrogatePair + u"test" + surrogatePair;

	// Act
	const auto result = Convert::Utf::Utf8::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("😀test😀"), outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8EncodeTest, ShouldEncodeInvalidSurrogatePairsAsErrorMark)
{
	// Arrange
	std::string outString;
	const std::u16string source = u"\xDE00test\xDE00";

	// Act
	const auto result = Convert::Utf::Utf8::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("☐test☐"), outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(2, result.InvalidSequencesCount);
}

TEST(Utf8EncodeTest, ShouldWriteCustomErrorMarkWhenError)
{
	// Arrange
	std::string outString;
	const std::u16string source = u"test\xDE00";

	// Act
	const auto result = Convert::Utf::Utf8::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip, "<ERROR>");

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("test<ERROR>"), outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1, result.InvalidSequencesCount);
}

TEST(Utf8EncodeTest, ShouldSkipWrongSequenceWhenErrorMarkIsEmpty)
{
	// Arrange
	std::string outString;
	const std::u16string source = u"test\xDE00";

	// Act
	const auto result = Convert::Utf::Utf8::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip, "");

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("test"), outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1, result.InvalidSequencesCount);
}

TEST(Utf8EncodeTest, ShouldHandlePolicyThrowError)
{
	// Arrange
	std::string outString;
	const std::u16string source = u"test\xDE00test";

	// Act
	const auto result = Convert::Utf::Utf8::Encode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::ThrowError);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(UTF8("test"), outString);
	EXPECT_EQ(4, std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1, result.InvalidSequencesCount);
}

TEST(Utf8EncodeTest, ShouldHandleUnexpectedEndWhenCroppedSurrogatePair)
{
	// Arrange
	std::string outString;
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string source = u"test_тест" + croppedSequence;
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf8::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ(UTF8("test_тест"), outString);
	EXPECT_EQ(expectedPos, std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8EncodeTest, ShouldAppendToExistingString)
{
	// Arrange
	std::string outString = "Hello";
	const std::wstring source = L" world!";

	// Act
	const auto result = Convert::Utf::Utf8::Encode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("Hello world!"), outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

//-----------------------------------------------------------------------------
// Tests for decoding string from UTF-8
//-----------------------------------------------------------------------------
TEST(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedOneOctet)
{
	// Arrange
	std::wstring outString;
	constexpr std::string_view source = "Hello world!";

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(L"Hello world!", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedTwoOctets)
{
	// Arrange
	std::u16string outString;
	const std::string_view source = UTF8("Привет мир!");

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"Привет мир!", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedThreeOctets)
{
	// Arrange
	std::u16string outString;
	const std::string_view source = UTF8("世界，您好！");

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"世界，您好！", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedFourOctets)
{
	// Arrange
	std::u32string outString;
	const std::string_view source = UTF8("😀😎🙋");

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"😀😎🙋", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldDecodeUtf8WhenDeprecatedFiveOctets)
{
	// Arrange
	std::u32string outString;
	const std::string fiveOctets(MakeStringFromSequence(0b11111000, 0b10000001, 0b10000001, 0b10000001, 0b10000001));
	const std::string source = fiveOctets + "test" + fiveOctets;

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"☐test☐", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(2, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldDecodeUtf8WhenDeprecatedSixOctets)
{
	// Arrange
	std::u32string outString;
	const std::string sixOctets(MakeStringFromSequence(0b11111100, 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b10000001));
	const std::string source = sixOctets + "test" + sixOctets;

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"☐test☐", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(2, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldDecodeUtf8WhenInvalidStartCode)
{
	// Arrange
	std::u32string outString;
	const std::string wrongStartCodes(MakeStringFromSequence(0b11111110, 0b11111111));
	const std::string source = wrongStartCodes + "test" + wrongStartCodes;

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"☐☐test☐☐", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(4, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldDecodeUtf8WhenWrongTail2InSequence)
{
	// Arrange
	std::u32string outString;
	const std::string wrongSequence(MakeStringFromSequence(0b11110111, 0b11111111, 0b10111111, 0b10111111));
	const std::string source = wrongSequence + "test" + wrongSequence;

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"☐test☐", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(2, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldDecodeUtf8WhenWrongTail3InSequence)
{
	// Arrange
	std::u32string outString;
	const std::string wrongSequence(MakeStringFromSequence(0b11110111, 0b10111111, 0b11111111, 0b10111111));
	const std::string source = wrongSequence + "test" + wrongSequence;

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"☐test☐", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(2, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldDecodeUtf8WhenWrongTail4InSequence)
{
	// Arrange
	std::u32string outString;
	const std::string wrongSequence(MakeStringFromSequence(0b11110111, 0b10111111, 0b10111111, 0b11111111));
	const std::string source = wrongSequence + "test" + wrongSequence;

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"☐test☐", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(2, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldSkipWrongSequenceWhenErrorMarkIsEmpty)
{
	// Arrange
	std::u32string outString;
	const std::string wrongSequence(MakeStringFromSequence(0b11110111, 0b10111111, 0b10111111, 0b11111111));
	const std::string source = wrongSequence + "test" + wrongSequence;

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip, U"");

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"test", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(2, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldHandlePolicyThrowError)
{
	// Arrange
	std::u32string outString;
	const std::string wrongSequence(MakeStringFromSequence(0b11110111, 0b10111111, 0b10111111, 0b11111111));
	const std::string source = "test" + wrongSequence + "test";

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::ThrowError);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(U"test", outString);
	EXPECT_EQ(4, std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldHandleUnexpectedEndWhenMissedTwoOctetsAtEnd)
{
	// Arrange
	std::u32string outString;
	const std::string croppedSequence(MakeStringFromSequence(0b11011111));
	const std::string source = "test" + croppedSequence;
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ(U"test", outString);
	EXPECT_EQ(expectedPos, std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldHandleUnexpectedEndWhenMissedThreeOctetsAtEnd)
{
	// Arrange
	std::u32string outString;
	const std::string croppedSequence(MakeStringFromSequence(0b11101111, 0b10000001));
	const std::string source = "test" + croppedSequence;
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ(U"test", outString);
	EXPECT_EQ(expectedPos, std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldHandleUnexpectedEndWhenMissedFourOctetsAtEnd)
{
	// Arrange
	std::u32string outString;
	const std::string croppedSequence(MakeStringFromSequence(0b11110111, 0b10000001, 0b10000001));
	const std::string source = "test" + croppedSequence;
	const size_t expectedPos = source.size() - croppedSequence.size();

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::UnexpectedEnd, result.ErrorCode);
	EXPECT_EQ(U"test", outString);
	EXPECT_EQ(expectedPos, std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldNotDecodeSurrogatePairs)
{
	// Arrange
	std::u32string outString;
	const std::string source = "test\xED\xA1\x8C\xED\xBE\xB4";

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"test☐☐", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(2, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldPutCustomErrorMarkWhenError)
{
	// Arrange
	std::u32string outString;
	const std::string source = "test\xED\xA1\x8Ctest";

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString, Convert::Utf::UtfEncodingErrorPolicy::Skip, U"<ERROR>");

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::Success, result.ErrorCode);
	EXPECT_EQ(U"test<ERROR>test", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(1, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldDecodeAsSurrogatePairsWhenTargetIsUtf16)
{
	// Arrange
	std::u16string outString;
	const std::string source = UTF8("😀test🙋");

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::Success, result.ErrorCode);
	EXPECT_EQ(u"😀test🙋", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

TEST(Utf8DecodeTest, ShouldAppendToExistingString)
{
	// Arrange
	std::wstring outString = L"Hello";
	constexpr std::string_view source = " world!";

	// Act
	const auto result = Convert::Utf::Utf8::Decode(std::begin(source), std::end(source), outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(L"Hello world!", outString);
	EXPECT_EQ(source.size(), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0, result.InvalidSequencesCount);
}

#pragma warning(pop)
