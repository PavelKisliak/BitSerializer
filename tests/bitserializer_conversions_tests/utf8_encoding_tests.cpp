/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;

// UTF-8 encode test fixture
class Utf8EncodeTest : public testing::Test
{
protected:
	template <typename TOutStr = std::string>
	static TOutStr EncodeUtf8(const std::wstring& unicodeStr, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutStr::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutStr::value_type>())
	{
		TOutStr result;
		Convert::Utf8::Encode(unicodeStr.begin(), unicodeStr.end(), result, encodePolicy, errorMark);
		return result;
	}

	template <typename TOutStr = std::string>
	static TOutStr EncodeUtf8(const std::u16string& unicodeStr, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutStr::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutStr::value_type>())
	{
		TOutStr result;
		Convert::Utf8::Encode(unicodeStr.begin(), unicodeStr.end(), result, encodePolicy, errorMark);
		return result;
	}

	template <typename TOutStr = std::string>
	static TOutStr EncodeUtf8(const std::u32string& unicodeStr, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutStr::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutStr::value_type>())
	{
		TOutStr result;
		Convert::Utf8::Encode(unicodeStr.begin(), unicodeStr.end(), result, encodePolicy, errorMark);
		return result;
	}
};

// UTF-8 decode test fixture
class Utf8DecodeTest : public testing::Test
{
protected:
	template <typename TOutputString>
	static TOutputString DecodeUtf8As(const std::string& utf8Str, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutputString::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutputString::value_type>())
	{
		TOutputString result;
		Convert::Utf8::Decode(utf8Str.begin(), utf8Str.end(), result, encodePolicy, errorMark);
		return result;
	}
};


#pragma warning(push)
#pragma warning(disable: 4566)
//-----------------------------------------------------------------------------
// Tests for encoding string to UTF-8
//-----------------------------------------------------------------------------
TEST_F(Utf8EncodeTest, ShouldEncodeUtf8WhenUsedOneOctet) {
	EXPECT_EQ(1, EncodeUtf8(std::wstring({ 0x7f })).size());
	EXPECT_EQ(u8"Hello world!", EncodeUtf8(L"Hello world!"));
}

TEST_F(Utf8EncodeTest, ShouldEncodeUtf8WhenUsedTwoOctets) {
	EXPECT_EQ(2, EncodeUtf8(std::wstring({ 0x7ff })).size());
	EXPECT_EQ(u8"Привет мир!", EncodeUtf8(L"Привет мир!"));
}

TEST_F(Utf8EncodeTest, ShouldEncodeUtf8WhenUsedThreeOctets) {
	EXPECT_EQ(3, EncodeUtf8(std::wstring({ 0xffff })).size());
	EXPECT_EQ(u8"世界，您好！", EncodeUtf8(L"世界，您好！"));
}

TEST_F(Utf8EncodeTest, ShouldEncodeUtf8WhenUsedFourOctets) {
	EXPECT_EQ(4, EncodeUtf8(std::u32string({ 0x10FFFF })).size());
	EXPECT_EQ(u8"😀😎🙋", EncodeUtf8(U"😀😎🙋"));
}

TEST_F(Utf8EncodeTest, ShouldEncodeUtf8WithDecodingSurrogatePairs) {
	const std::u16string surrogatePair = u"\xD83D\xDE00";
	EXPECT_EQ(u8"😀test😀", EncodeUtf8(surrogatePair + u"test" + surrogatePair));
}

TEST_F(Utf8EncodeTest, ShouldEncodeInvalidSurrogatePairsAsErrorMark) {
	EXPECT_EQ(u8"test☐", EncodeUtf8(u"test\xDE00", Convert::EncodeErrorPolicy::WriteErrorMark));
	EXPECT_EQ(u8"test☐тест", EncodeUtf8(u"test\xD83Dтест", Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf8EncodeTest, ShouldPutCustomErrorMarkWhenError) {
	EXPECT_EQ(u8"test<ERROR>", EncodeUtf8(u"test\xDE00", Convert::EncodeErrorPolicy::WriteErrorMark, "<ERROR>"));
}

TEST_F(Utf8EncodeTest, ShouldHandlePolicyThrowException) {
	EXPECT_THROW(EncodeUtf8(u"test\xDE00", Convert::EncodeErrorPolicy::ThrowException), std::runtime_error);
}

TEST_F(Utf8EncodeTest, ShouldHandlePolicySkip) {
	EXPECT_EQ(u8"test", EncodeUtf8(u"test\xDE00", Convert::EncodeErrorPolicy::Skip));
}

TEST_F(Utf8EncodeTest, ShouldReturnIteratorToEnd)
{
	// Arrange
	constexpr std::u16string_view testStr = u"test";

	// Act
	std::string actualStr;
	const auto actualIt = Convert::Utf8::Encode(testStr.cbegin(), testStr.cend(), actualStr);

	// Assert
	EXPECT_TRUE(actualIt == testStr.cend());
}

TEST_F(Utf8EncodeTest, ShouldReturnIteratorToCroppedSurrogatePairAtEnd)
{
	// Arrange
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string testStr = u"test_тест" + croppedSequence;
	const size_t expectedPos = testStr.size() - croppedSequence.size();

	// Act
	std::string actual;
	const size_t actualPos = Convert::Utf8::Encode(testStr.cbegin(), testStr.cend(), actual) - testStr.cbegin();

	// Assert
	EXPECT_EQ(expectedPos, actualPos);
	EXPECT_EQ(u8"test_тест", actual);
}


//-----------------------------------------------------------------------------
// Tests for decoding string from UTF-8
//-----------------------------------------------------------------------------
TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedOneOctet) {
	EXPECT_EQ(L"Hello world!", DecodeUtf8As<std::wstring>(u8"Hello world!"));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedTwoOctets) {
	EXPECT_EQ(L"Привет мир!", DecodeUtf8As<std::wstring>(u8"Привет мир!"));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedThreeOctets) {
	EXPECT_EQ(L"世界，您好！", DecodeUtf8As<std::wstring>(u8"世界，您好！"));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedFourOctets) {
	EXPECT_EQ(U"😀😎🙋", DecodeUtf8As<std::u32string>(u8"😀😎🙋"));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenDeprecatedFiveOctets) {
	const std::string fiveOctets({ char(0b11111000), char(0b10000001), char(0b10000001), char(0b10000001), char(0b10000001) });
	EXPECT_EQ(U"☐test☐", DecodeUtf8As<std::u32string>(fiveOctets + u8"test" + fiveOctets, Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenDeprecatedSixOctets) {
	const std::string sixOctets({ char(0b11111100), char(0b10000001), char(0b10000001), char(0b10000001), char(0b10000001), char(0b10000001) });
	EXPECT_EQ(U"☐test☐", DecodeUtf8As<std::u32string>(sixOctets + u8"test" + sixOctets, Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenInvalidStartCode) {
	const std::string wrongStartCodes({ char(0b11111110), char(0b11111111) });
	EXPECT_EQ(U"☐☐test☐☐", DecodeUtf8As<std::u32string>(wrongStartCodes + u8"test" + wrongStartCodes, Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenWrongTail2InSequence) {
	const std::string wrongSequence({ char(0b11110111), char(0b11111111), char(0b10111111), char(0b10111111) });
	EXPECT_EQ(U"☐test☐", DecodeUtf8As<std::u32string>(wrongSequence + u8"test" + wrongSequence, Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenWrongTail3InSequence) {
	const std::string wrongSequence({ char(0b11110111), char(0b10111111), char(0b11111111), char(0b10111111) });
	EXPECT_EQ(U"☐test☐", DecodeUtf8As<std::u32string>(wrongSequence + u8"test" + wrongSequence, Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenWrongTail4InSequence) {
	const std::string wrongSequence({ char(0b11110111), char(0b10111111), char(0b10111111), char(0b11111111) });
	EXPECT_EQ(U"☐test☐", DecodeUtf8As<std::u32string>(wrongSequence + u8"test" + wrongSequence, Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf8DecodeTest, ShouldHandlePolicyThrowException) {
	const std::string wrongSequence({ char(0b11110111), char(0b10111111), char(0b10111111), char(0b11111111) });
	EXPECT_THROW(DecodeUtf8As<std::u32string>(wrongSequence + u8"test", Convert::EncodeErrorPolicy::ThrowException), std::runtime_error);
}

TEST_F(Utf8DecodeTest, ShouldHandlePolicySkip) {
	const std::string wrongSequence({ char(0b11110111), char(0b10111111), char(0b10111111), char(0b11111111) });
	EXPECT_EQ(U"test", DecodeUtf8As<std::u32string>(wrongSequence + u8"test", Convert::EncodeErrorPolicy::Skip));
}

TEST_F(Utf8DecodeTest, ShouldReturnIteratorToCroppedTwoOctetsAtEnd)
{
	// Arrange
	const std::string croppedSequence({ char(0b11011111) });
	const std::string testStr = u8"test" + croppedSequence;
	const size_t expectedPos = testStr.size() - croppedSequence.size();

	// Act
	std::u32string actual;
	const size_t actualPos = Convert::Utf8::Decode(testStr.cbegin(), testStr.cend(), actual) - testStr.cbegin();

	// Assert
	EXPECT_EQ(expectedPos, actualPos);
	EXPECT_EQ(U"test", actual);
}

TEST_F(Utf8DecodeTest, ShouldReturnIteratorToCroppedThreeOctetsAtEnd)
{
	// Arrange
	const std::string croppedSequence({ char(0b11101111), char(0b10000001) });
	const std::string testStr = u8"test" + croppedSequence;
	const size_t expectedPos = testStr.size() - croppedSequence.size();

	// Act
	std::u32string actual;
	const size_t actualPos = Convert::Utf8::Decode(testStr.cbegin(), testStr.cend(), actual) - testStr.cbegin();

	// Assert
	EXPECT_EQ(expectedPos, actualPos);
	EXPECT_EQ(U"test", actual);
}

TEST_F(Utf8DecodeTest, ShouldReturnIteratorToCroppedFourOctetsAtEnd)
{
	// Arrange
	const std::string croppedSequence({ char(0b11110111), char(0b10000001), char(0b10000001) });
	const std::string testStr = u8"test" + croppedSequence;
	const size_t expectedPos = testStr.size() - croppedSequence.size();

	// Act
	std::u32string actual;
	const size_t actualPos = Convert::Utf8::Decode(testStr.cbegin(), testStr.cend(), actual) - testStr.cbegin();

	// Assert
	EXPECT_EQ(expectedPos, actualPos);
	EXPECT_EQ(U"test", actual);
}

TEST_F(Utf8DecodeTest, ShouldNotDecodeSurrogatePairs) {
	const std::string encodedSurrogatePair = "\xED\xA1\x8C\xED\xBE\xB4";
	EXPECT_EQ(U"test☐☐", DecodeUtf8As<std::u32string>(u8"test" + encodedSurrogatePair, Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf8DecodeTest, ShouldPutCustomErrorMarkWhenError) {
	const std::string wrongSurrogate = "\xED\xA1\x8C\xED";
	EXPECT_EQ(U"test<ERROR>", DecodeUtf8As<std::u32string>(u8"test" + wrongSurrogate,
		Convert::EncodeErrorPolicy::WriteErrorMark, U"<ERROR>"));
}

TEST_F(Utf8DecodeTest, ShouldDecodeAsSurrogatePairsWhenTargetIsUtf16) {
	EXPECT_EQ(u"😀test🙋", DecodeUtf8As<std::u16string>(u8"😀test🙋"));
}

TEST_F(Utf8DecodeTest, ShouldReturnIteratorToEnd)
{
	// Arrange
	constexpr std::string_view testStr = u8"test";

	// Act
	std::u16string actualStr;
	const auto actualIt = Convert::Utf8::Decode(testStr.cbegin(), testStr.cend(), actualStr);

	// Assert
	EXPECT_TRUE(actualIt == testStr.cend());
}

#pragma warning(pop)
