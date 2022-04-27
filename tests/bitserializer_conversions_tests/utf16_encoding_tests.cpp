/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"

using namespace BitSerializer;

// UTF-16 encode test fixture
template <class TEncoder>
class Utf16EncodeBaseFixture : public testing::Test
{
protected:
	template <typename TOutStr = std::u16string>
	static TOutStr EncodeUtf16(const std::string& srcStr, const char errSym = '?')
	{
		TOutStr result;
		TEncoder::Encode(srcStr.begin(), srcStr.end(), result, errSym);
		return result;
	}

	template <typename TOutStr = std::u16string>
	static TOutStr EncodeUtf16(const std::wstring& srcStr, const char errSym = '?')
	{
		TOutStr result;
		TEncoder::Encode(srcStr.begin(), srcStr.end(), result, errSym);
		return result;
	}

	template <typename TOutStr = std::u16string>
	static TOutStr EncodeUtf16(const std::u16string& srcStr, const char errSym = '?')
	{
		TOutStr result;
		TEncoder::Encode(srcStr.begin(), srcStr.end(), result, errSym);
		return result;
	}

	template <typename TOutStr = std::u16string>
	static TOutStr EncodeUtf16(const std::u32string& srcStr, const char errSym = '?')
	{
		TOutStr result;
		TEncoder::Encode(srcStr.begin(), srcStr.end(), result, errSym);
		return result;
	}
};

class Utf16LeEncodeTest : public Utf16EncodeBaseFixture<Convert::Utf16Le> {};
class Utf16BeEncodeTest : public Utf16EncodeBaseFixture<Convert::Utf16Be> {};


// UTF-16 decode test fixture
template <class TEncoder>
class Utf16DecodeBaseFixture : public testing::Test
{
protected:
	template <typename TOutputString>
	static TOutputString DecodeUtf16As(const std::u16string& utf16Str, const char errSym = '?')
	{
		TOutputString result;
		TEncoder::Decode(utf16Str.begin(), utf16Str.end(), result, errSym);
		return result;
	}
};

class Utf16LeDecodeTest : public Utf16DecodeBaseFixture<Convert::Utf16Le> {};
class Utf16BeDecodeTest : public Utf16DecodeBaseFixture<Convert::Utf16Be> {};

namespace 
{
	template <typename TOutputString = std::u16string>
	TOutputString SwapByteOrder(const std::u16string& str)
	{
		TOutputString result;
		std::transform(std::cbegin(str), std::cend(str), std::back_inserter(result), [](auto sym) -> char16_t {
			return (sym >> 8) | (sym << 8);
		});
		return result;
	}
}

#pragma warning(push)
#pragma warning(disable: 4566)
//-----------------------------------------------------------------------------
// UTF-16 LE: Tests for encoding string
//-----------------------------------------------------------------------------
TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromAnsi) {
	EXPECT_EQ(u"Hello world!", EncodeUtf16("Hello world!"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf8) {
	EXPECT_EQ(u"Привет мир!", EncodeUtf16(u8"Привет мир!"));
	EXPECT_EQ(u"世界，您好！", EncodeUtf16(u8"世界，您好！"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf8Surrogates) {
	EXPECT_EQ(u"😀😎🙋", EncodeUtf16(u8"😀😎🙋"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf16) {
	EXPECT_EQ(u"Привет мир!", EncodeUtf16(u"Привет мир!"));
	EXPECT_EQ(u"世界，您好！", EncodeUtf16(u"世界，您好！"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf16Surrogates) {
	EXPECT_EQ(u"😀😎🙋", EncodeUtf16(u"😀😎🙋"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromWString) {
	EXPECT_EQ(u"Привет мир!", EncodeUtf16(L"Привет мир!"));
	EXPECT_EQ(u"世界，您好！", EncodeUtf16(L"世界，您好！"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf32) {
	EXPECT_EQ(u"Привет мир!", EncodeUtf16(U"Привет мир!"));
	EXPECT_EQ(u"世界，您好！", EncodeUtf16(U"世界，您好！"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf32Surrogates) {
	EXPECT_EQ(u"😀😎🙋", EncodeUtf16(U"😀😎🙋"));
}


//-----------------------------------------------------------------------------
// UTF-16 LE: Tests decoding string
//-----------------------------------------------------------------------------
TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToAnsi) {
	EXPECT_EQ("Hello world!", DecodeUtf16As<std::string>(u"Hello world!"));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf8) {
	EXPECT_EQ(u8"Привет мир!", DecodeUtf16As<std::string>(u"Привет мир!"));
	EXPECT_EQ(u8"世界，您好！", DecodeUtf16As<std::string>(u"世界，您好！"));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf16) {
	EXPECT_EQ(u"Hello world!", DecodeUtf16As<std::u16string>(u"Hello world!"));
	EXPECT_EQ(u"Привет мир!", DecodeUtf16As<std::u16string>(u"Привет мир!"));
	EXPECT_EQ(u"世界，您好！", DecodeUtf16As<std::u16string>(u"世界，您好！"));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf16WithSurrogates) {
	EXPECT_EQ(u"😀😎🙋", DecodeUtf16As<std::u16string>(u"😀😎🙋"));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToWString) {
	EXPECT_EQ(L"Hello world!", DecodeUtf16As<std::wstring>(u"Hello world!"));
	EXPECT_EQ(L"Привет мир!", DecodeUtf16As<std::wstring>(u"Привет мир!"));
	EXPECT_EQ(L"世界，您好！", DecodeUtf16As<std::wstring>(u"世界，您好！"));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf32) {
	EXPECT_EQ(U"Hello world!", DecodeUtf16As<std::u32string>(u"Hello world!"));
	EXPECT_EQ(U"Привет мир!", DecodeUtf16As<std::u32string>(u"Привет мир!"));
	EXPECT_EQ(U"世界，您好！", DecodeUtf16As<std::u32string>(u"世界，您好！"));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf32WithSurrogates) {
	EXPECT_EQ(U"😀😎🙋", DecodeUtf16As<std::u32string>(u"😀😎🙋"));
}

TEST_F(Utf16LeDecodeTest, ShouldPutErrorSymbolWhenSurrogateStartsWithWrongCode) {
	const std::u16string wrongStartCodes({ Convert::Unicode::LowSurrogatesEnd, Convert::Unicode::LowSurrogatesStart });
	EXPECT_EQ(U"__test__", DecodeUtf16As<std::u32string>(wrongStartCodes + u"test" + wrongStartCodes, '_'));
}

TEST_F(Utf16LeDecodeTest, ShouldPutErrorSymbolWhenNoSecondCodeInSurrogate) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(U"test_", DecodeUtf16As<std::u32string>(u"test" + notFullSurrogatePair, '_'));
}


//-----------------------------------------------------------------------------
// UTF-16 BE: Tests for encoding string
//-----------------------------------------------------------------------------
TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromAnsi) {
	EXPECT_EQ(SwapByteOrder(u"Hello world!"), EncodeUtf16("Hello world!"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf8) {
	EXPECT_EQ(SwapByteOrder(u"Привет мир!"), EncodeUtf16(u8"Привет мир!"));
	EXPECT_EQ(SwapByteOrder(u"世界，您好！"), EncodeUtf16(u8"世界，您好！"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf8Surrogates) {
	EXPECT_EQ(SwapByteOrder(u"😀😎🙋"), EncodeUtf16(u8"😀😎🙋"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf16) {
	EXPECT_EQ(SwapByteOrder(u"Привет мир!"), EncodeUtf16(u"Привет мир!"));
	EXPECT_EQ(SwapByteOrder(u"世界，您好！"), EncodeUtf16(u"世界，您好！"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf16Surrogates) {
	EXPECT_EQ(SwapByteOrder(u"😀😎🙋"), EncodeUtf16(u"😀😎🙋"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromWString) {
	EXPECT_EQ(SwapByteOrder(u"Привет мир!"), EncodeUtf16(L"Привет мир!"));
	EXPECT_EQ(SwapByteOrder(u"世界，您好！"), EncodeUtf16(L"世界，您好！"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf32) {
	EXPECT_EQ(SwapByteOrder(u"Привет мир!"), EncodeUtf16(U"Привет мир!"));
	EXPECT_EQ(SwapByteOrder(u"世界，您好！"), EncodeUtf16(U"世界，您好！"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf32Surrogates) {
	EXPECT_EQ(SwapByteOrder(u"😀😎🙋"), EncodeUtf16(U"😀😎🙋"));
}


//-----------------------------------------------------------------------------
// UTF-16 BE: Tests decoding string
//-----------------------------------------------------------------------------
TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToAnsi) {
	EXPECT_EQ("Hello world!", DecodeUtf16As<std::string>(SwapByteOrder(u"Hello world!")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf8) {
	EXPECT_EQ(u8"Привет мир!", DecodeUtf16As<std::string>(SwapByteOrder(u"Привет мир!")));
	EXPECT_EQ(u8"世界，您好！", DecodeUtf16As<std::string>(SwapByteOrder(u"世界，您好！")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf16Le) {
	EXPECT_EQ(u"Hello world!", DecodeUtf16As<std::u16string>(SwapByteOrder(u"Hello world!")));
	EXPECT_EQ(u"Привет мир!", DecodeUtf16As<std::u16string>(SwapByteOrder(u"Привет мир!")));
	EXPECT_EQ(u"世界，您好！", DecodeUtf16As<std::u16string>(SwapByteOrder(u"世界，您好！")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf16LeWithSurrogates) {
	EXPECT_EQ(u"😀😎🙋", DecodeUtf16As<std::u16string>(SwapByteOrder(u"😀😎🙋")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToWString) {
	EXPECT_EQ(L"Hello world!", DecodeUtf16As<std::wstring>(SwapByteOrder(u"Hello world!")));
	EXPECT_EQ(L"Привет мир!", DecodeUtf16As<std::wstring>(SwapByteOrder(u"Привет мир!")));
	EXPECT_EQ(L"世界，您好！", DecodeUtf16As<std::wstring>(SwapByteOrder(u"世界，您好！")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf32) {
	EXPECT_EQ(U"Hello world!", DecodeUtf16As<std::u32string>(SwapByteOrder(u"Hello world!")));
	EXPECT_EQ(U"Привет мир!", DecodeUtf16As<std::u32string>(SwapByteOrder(u"Привет мир!")));
	EXPECT_EQ(U"世界，您好！", DecodeUtf16As<std::u32string>(SwapByteOrder(u"世界，您好！")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf32WithSurrogates) {
	EXPECT_EQ(U"😀😎🙋", DecodeUtf16As<std::u32string>(SwapByteOrder(u"😀😎🙋")));
}

TEST_F(Utf16BeDecodeTest, ShouldPutErrorSymbolWhenSurrogateStartsWithWrongCode) {
	const std::u16string wrongStartCodes({ Convert::Unicode::LowSurrogatesEnd, Convert::Unicode::LowSurrogatesStart });
	EXPECT_EQ(U"__test__", DecodeUtf16As<std::u32string>(SwapByteOrder(wrongStartCodes + u"test" + wrongStartCodes), '_'));
}

TEST_F(Utf16BeDecodeTest, ShouldPutErrorSymbolWhenNoSecondCodeInSurrogate) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(U"test_", DecodeUtf16As<std::u32string>(SwapByteOrder(u"test" + notFullSurrogatePair), '_'));
}

#pragma warning(pop)
