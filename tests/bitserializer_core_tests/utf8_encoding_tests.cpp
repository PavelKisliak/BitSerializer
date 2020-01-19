/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "test_helpers/common_test_entities.h"
#include "bitserializer/conversion_detail/convert_utf.h"

using namespace BitSerializer;

// UTF-8 encode test fixture
class Utf8EncodeTest : public testing::Test
{
protected:
	static std::string EncodeUtf8(const std::wstring& unicodeStr, const char errSym = '?')
	{
		std::string result;
		Convert::Utf8::Encode(unicodeStr.begin(), unicodeStr.end(), result, errSym);
		return result;
	}

	static std::string EncodeUtf8(const std::u32string& unicodeStr, const char errSym = '?')
	{
		std::string result;
		Convert::Utf8::Encode(unicodeStr.begin(), unicodeStr.end(), result, errSym);
		return result;
	}
};

// UTF-8 decode test fixture
class Utf8DecodeTest : public testing::Test
{
protected:
	static std::wstring DecodeUtf8ToWString(const std::string& utf8Str, const std::wstring::value_type errSym = '?')
	{
		std::wstring result;
		Convert::Utf8::Decode(utf8Str.begin(), utf8Str.end(), result, errSym);
		return result;
	}

	static std::u32string DecodeUtf8ToU32string(const std::string& utf8Str, const std::u32string::value_type errSym = '?')
	{
		std::u32string result;
		Convert::Utf8::Decode(utf8Str.begin(), utf8Str.end(), result, errSym);
		return result;
	}

	static std::u16string DecodeUtf8ToU16string(const std::string& utf8Str, const std::u16string::value_type errSym = '?')
	{
		std::u16string result;
		Convert::Utf8::Decode(utf8Str.begin(), utf8Str.end(), result, errSym);
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
	const std::wstring surrogatePair = { wchar_t(0xD83D), wchar_t(0xDE00) };
	EXPECT_EQ(u8"😀test😀", EncodeUtf8(surrogatePair + L"test" + surrogatePair));
}

TEST_F(Utf8EncodeTest, ShouldEncodeInvalidSurrogatePairsAsErrSym) {
	EXPECT_EQ(u8"test?", EncodeUtf8(std::wstring(L"test") + wchar_t(0xDE00)));
	EXPECT_EQ(u8"test?", EncodeUtf8(std::wstring(L"test") + wchar_t(0xD83D)));
	EXPECT_EQ(u8"test_string", EncodeUtf8(std::wstring(L"test") + wchar_t(0xD83D) + L"string", '_'));
}

//-----------------------------------------------------------------------------
// Tests for decoding string from UTF-8
//-----------------------------------------------------------------------------
TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedOneOctet) {
	EXPECT_EQ(L"Hello world!", DecodeUtf8ToWString(u8"Hello world!"));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedTwoOctets) {
	EXPECT_EQ(L"Привет мир!", DecodeUtf8ToWString(u8"Привет мир!"));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedThreeOctets) {
	EXPECT_EQ(L"世界，您好！", DecodeUtf8ToWString(u8"世界，您好！"));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenUsedFourOctets) {
	EXPECT_EQ(std::u32string(U"😀😎🙋"), DecodeUtf8ToU32string(u8"😀😎🙋"));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenDeprecatedFiveOctets) {
	const std::string fiveOctets({ char(0b11111000), char(0b10000001), char(0b10000001), char(0b10000001), char(0b10000001) });
	EXPECT_EQ(std::u32string(U"_test_"), DecodeUtf8ToU32string(fiveOctets + u8"test" + fiveOctets, '_'));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenDeprecatedSixOctets) {
	const std::string sixOctets({ char(0b11111100), char(0b10000001), char(0b10000001), char(0b10000001), char(0b10000001), char(0b10000001) });
	EXPECT_EQ(std::u32string(U"_test_"), DecodeUtf8ToU32string(sixOctets + u8"test" + sixOctets, '_'));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenInvalidStartCode) {
	const std::string wrongStartCodes({ char(0b11111110), char(0b11111111) });
	EXPECT_EQ(std::u32string(U"__test__"), DecodeUtf8ToU32string(wrongStartCodes + u8"test" + wrongStartCodes, '_'));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenWrongTail2InSequence) {
	const std::string wrongSequence({ char(0b11110111), char(0b11111111), char(0b10111111), char(0b10111111) });
	EXPECT_EQ(std::u32string(U"_test_"), DecodeUtf8ToU32string(wrongSequence + u8"test" + wrongSequence, '_'));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenWrongTail3InSequence) {
	const std::string wrongSequence({ char(0b11110111), char(0b10111111), char(0b11111111), char(0b10111111) });
	EXPECT_EQ(std::u32string(U"_test_"), DecodeUtf8ToU32string(wrongSequence + u8"test" + wrongSequence, '_'));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenWrongTail4InSequence) {
	const std::string wrongSequence({ char(0b11110111), char(0b10111111), char(0b10111111), char(0b11111111) });
	EXPECT_EQ(std::u32string(U"_test_"), DecodeUtf8ToU32string(wrongSequence + u8"test" + wrongSequence, '_'));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenCroppedTwoOctetsAtEnd) {
	const std::string croppedSequence({ char(0b11011111) });
	EXPECT_EQ(std::u32string(U"test_"), DecodeUtf8ToU32string(u8"test" + croppedSequence, '_'));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenCroppedThreeOctetsAtEnd) {
	const std::string croppedSequence({ char(0b11101111) });
	EXPECT_EQ(std::u32string(U"test_"), DecodeUtf8ToU32string(u8"test" + croppedSequence, '_'));
}

TEST_F(Utf8DecodeTest, ShouldDecodeUtf8WhenCroppedFourOctetsAtEnd) {
	const std::string croppedSequence({ char(0b11110111) });
	EXPECT_EQ(std::u32string(U"test_"), DecodeUtf8ToU32string(u8"test" + croppedSequence, '_'));
}

TEST_F(Utf8DecodeTest, ShouldNotDecodeSurrogatePairs) {
	const std::string encodedSurrogatePair = { char(0xED), char(0xA1), char(0x8C), char(0xED), char(0xBE), char(0xB4) };
	EXPECT_EQ(std::u32string(U"test__"), DecodeUtf8ToU32string(u8"test" + encodedSurrogatePair, '_'));
}

TEST_F(Utf8DecodeTest, ShouldDecodeAsSurrogatePairsWhenCharExceedsUtf16Range) {
	EXPECT_EQ(u"😀test🙋", DecodeUtf8ToU16string(u8"😀test🙋"));
}

#pragma warning(pop)
