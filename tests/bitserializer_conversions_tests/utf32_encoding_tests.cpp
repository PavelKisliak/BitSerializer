/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"

using namespace BitSerializer;

// UTF-32 encode test fixture
template <class TEncoder>
class Utf32EncodeBaseFixture : public testing::Test
{
protected:
	template <typename TOutStr = std::u32string>
	static TOutStr EncodeUtf32(const std::string& utf8string, const char errSym = '?')
	{
		TOutStr result;
		TEncoder::Encode(utf8string.begin(), utf8string.end(), result, errSym);
		return result;
	}

	template <typename TOutStr = std::u32string>
	static TOutStr EncodeUtf32(const std::wstring& unicodeStr, const char errSym = '?')
	{
		TOutStr result;
		TEncoder::Encode(unicodeStr.begin(), unicodeStr.end(), result, errSym);
		return result;
	}

	template <typename TOutStr = std::u32string>
	static TOutStr EncodeUtf32(const std::u16string& unicodeStr, const char errSym = '?')
	{
		TOutStr result;
		TEncoder::Encode(unicodeStr.begin(), unicodeStr.end(), result, errSym);
		return result;
	}

	template <typename TOutStr = std::u32string>
	static TOutStr EncodeUtf32(const std::u32string& unicodeStr, const char errSym = '?')
	{
		TOutStr result;
		TEncoder::Encode(unicodeStr.begin(), unicodeStr.end(), result, errSym);
		return result;
	}
};

class Utf32LeEncodeTest : public Utf32EncodeBaseFixture<Convert::Utf32Le> {};
class Utf32BeEncodeTest : public Utf32EncodeBaseFixture<Convert::Utf32Be> {};


// UTF-32 decode test fixture
template <class TEncoder>
class Utf32DecodeBaseFixture : public testing::Test
{
protected:
	template <typename TOutputString>
	static TOutputString DecodeUtf32As(const std::u32string& Utf32Str, const char errSym = '?')
	{
		TOutputString result;
		TEncoder::Decode(Utf32Str.begin(), Utf32Str.end(), result, errSym);
		return result;
	}
};

class Utf32LeDecodeTest : public Utf32DecodeBaseFixture<Convert::Utf32Le> {};
class Utf32BeDecodeTest : public Utf32DecodeBaseFixture<Convert::Utf32Be> {};

namespace 
{
	template <typename TOutStr = std::u32string>
	TOutStr SwapByteOrder(const std::u32string& str)
	{
		TOutStr result;
		std::transform(std::cbegin(str), std::cend(str), std::back_inserter(result), [](auto sym) -> char32_t {
			return (sym >> 24) | ((sym << 8) & 0x00FF0000) | ((sym >> 8) & 0x0000FF00) | (sym << 24);
		});
		return result;
	}
}

#pragma warning(push)
#pragma warning(disable: 4566)
//-----------------------------------------------------------------------------
// UTF-32 LE: Tests for encoding string
//-----------------------------------------------------------------------------
TEST_F(Utf32LeEncodeTest, ShouldEncodeUtf32FromAnsi) {
	EXPECT_EQ(U"Hello world!", EncodeUtf32("Hello world!"));
}

TEST_F(Utf32LeEncodeTest, ShouldEncodeUtf32FromUtf8) {
	EXPECT_EQ(U"Привет мир!", EncodeUtf32(u8"Привет мир!"));
}

TEST_F(Utf32LeEncodeTest, ShouldEncodeUtf32FromUtf16) {
	EXPECT_EQ(U"Привет мир!", EncodeUtf32(L"Привет мир!"));
	EXPECT_EQ(U"世界，您好！", EncodeUtf32(u"世界，您好！"));
}

TEST_F(Utf32LeEncodeTest, ShouldEncodeUtf32FromUtf16Surrogates) {
	EXPECT_EQ(U"😀😎🙋", EncodeUtf32(L"😀😎🙋"));
}

TEST_F(Utf32LeEncodeTest, ShouldPutErrorSymbolWhenSurrogateStartsWithWrongCode) {
	const std::u16string wrongStartCodes({ Convert::Unicode::LowSurrogatesEnd, Convert::Unicode::LowSurrogatesStart });
	EXPECT_EQ(U"__test__", EncodeUtf32(wrongStartCodes + u"test" + wrongStartCodes, '_'));
}

TEST_F(Utf32LeEncodeTest, ShouldPutErrorSymbolWhenNoSecondCodeInSurrogate) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(U"test_", EncodeUtf32(u"test" + notFullSurrogatePair, '_'));
}


//-----------------------------------------------------------------------------
// UTF-32 LE: Tests decoding string
//-----------------------------------------------------------------------------
TEST_F(Utf32LeDecodeTest, ShouldDecodeUtf32WithAnsiChars) {
	EXPECT_EQ(L"Hello world!", DecodeUtf32As<std::wstring>(U"Hello world!"));
	EXPECT_EQ(u"Hello world!", DecodeUtf32As<std::u16string>(U"Hello world!"));
	EXPECT_EQ(U"Hello world!", DecodeUtf32As<std::u32string>(U"Hello world!"));
}

TEST_F(Utf32LeDecodeTest, ShouldDecodeUtf32WithUnicodeChars) {
	EXPECT_EQ(u8"Привет мир!", DecodeUtf32As<std::string>(U"Привет мир!"));
	EXPECT_EQ(L"Привет мир!", DecodeUtf32As<std::wstring>(U"Привет мир!"));
	EXPECT_EQ(u"世界，您好！", DecodeUtf32As<std::u16string>(U"世界，您好！"));
}

TEST_F(Utf32LeDecodeTest, ShouldDecodeUtf32WithSurrogates) {
	EXPECT_EQ(u"😀😎🙋", DecodeUtf32As<std::u16string>(U"😀😎🙋"));
}


//-----------------------------------------------------------------------------
// UTF-32 BE: Tests for encoding string
//-----------------------------------------------------------------------------
TEST_F(Utf32BeEncodeTest, ShouldEncodeUtf32FromUtf16) {
	EXPECT_EQ(SwapByteOrder(U"Привет мир!"), EncodeUtf32(L"Привет мир!"));
	EXPECT_EQ(SwapByteOrder(U"世界，您好！"), EncodeUtf32(u"世界，您好！"));
}

TEST_F(Utf32BeEncodeTest, ShouldEncodeUtf32FromUtf16Surrogates) {
	EXPECT_EQ(SwapByteOrder(U"😀😎🙋"), EncodeUtf32(L"😀😎🙋"));
}


//-----------------------------------------------------------------------------
// UTF-32 BE: Tests decoding string
//-----------------------------------------------------------------------------
TEST_F(Utf32BeDecodeTest, ShouldDecodeUtf32WithAnsiChars) {
	EXPECT_EQ(L"Hello world!", DecodeUtf32As<std::wstring>(SwapByteOrder(U"Hello world!")));
	EXPECT_EQ(u"Hello world!", DecodeUtf32As<std::u16string>(SwapByteOrder(U"Hello world!")));
	EXPECT_EQ(U"Hello world!", DecodeUtf32As<std::u32string>(SwapByteOrder(U"Hello world!")));
}

TEST_F(Utf32BeDecodeTest, ShouldDecodeUtf32WithUnicodeChars) {
	EXPECT_EQ(L"Привет мир!", DecodeUtf32As<std::wstring>(SwapByteOrder(U"Привет мир!")));
	EXPECT_EQ(u"世界，您好！", DecodeUtf32As<std::u16string>(SwapByteOrder(U"世界，您好！")));
}

TEST_F(Utf32BeDecodeTest, ShouldDecodeUtf32WithSurrogates) {
	EXPECT_EQ(u"😀😎🙋", DecodeUtf32As<std::u16string>(SwapByteOrder(U"😀😎🙋")));
}

#pragma warning(pop)
