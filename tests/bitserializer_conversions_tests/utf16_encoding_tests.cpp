/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
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
	static TOutStr EncodeUtf16(const std::wstring& unicodeStr, const char errSym = '?')
	{
		TOutStr result;
		TEncoder::Encode(unicodeStr.begin(), unicodeStr.end(), result, errSym);
		return result;
	}

	template <typename TOutStr = std::u16string>
	static TOutStr EncodeUtf16(const std::u16string& unicodeStr, const char errSym = '?')
	{
		TOutStr result;
		TEncoder::Encode(unicodeStr.begin(), unicodeStr.end(), result, errSym);
		return result;
	}

	template <typename TOutStr = std::u16string>
	static TOutStr EncodeUtf16(const std::u32string& unicodeStr, const char errSym = '?')
	{
		TOutStr result;
		TEncoder::Encode(unicodeStr.begin(), unicodeStr.end(), result, errSym);
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
TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16WithAnsiChars) {
	EXPECT_EQ(u"Hello world!", EncodeUtf16(L"Hello world!"));
	EXPECT_EQ(u"Hello world!", EncodeUtf16(U"Hello world!"));
	EXPECT_EQ(u"Hello world!", EncodeUtf16(u"Hello world!"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16WithUnicodeChars) {
	EXPECT_EQ(u"Привет мир!", EncodeUtf16(U"Привет мир!"));
	EXPECT_EQ(u"世界，您好！", EncodeUtf16(U"世界，您好！"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16WithSurrogates) {
	EXPECT_EQ(u"😀😎🙋", EncodeUtf16(U"😀😎🙋"));
}


//-----------------------------------------------------------------------------
// UTF-16 LE: Tests decoding string
//-----------------------------------------------------------------------------
TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16WithAnsiChars) {
	EXPECT_EQ(L"Hello world!", DecodeUtf16As<std::wstring>(u"Hello world!"));
	EXPECT_EQ(u"Hello world!", DecodeUtf16As<std::u16string>(u"Hello world!"));
	EXPECT_EQ(U"Hello world!", DecodeUtf16As<std::u32string>(u"Hello world!"));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16WithUnicodeChars) {
	EXPECT_EQ(U"Привет мир!", DecodeUtf16As<std::u32string>(u"Привет мир!"));
	EXPECT_EQ(U"世界，您好！", DecodeUtf16As<std::u32string>(u"世界，您好！"));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16WithSurrogates) {
	EXPECT_EQ(U"😀😎🙋", DecodeUtf16As<std::u32string>(u"😀😎🙋"));
}

TEST_F(Utf16LeDecodeTest, ShouldLeaveSurrogatesWhenTargetStringIsUtf16) {
	EXPECT_EQ(L"😀😎🙋", DecodeUtf16As<std::wstring>(u"😀😎🙋"));
	EXPECT_EQ(u"😀😎🙋", DecodeUtf16As<std::u16string>(u"😀😎🙋"));
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
TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16WithAnsiChars) {
	EXPECT_EQ(SwapByteOrder(u"Hello world!"), EncodeUtf16(L"Hello world!"));
	EXPECT_EQ(SwapByteOrder(u"Hello world!"), EncodeUtf16(U"Hello world!"));
	EXPECT_EQ(SwapByteOrder(u"Hello world!"), EncodeUtf16(u"Hello world!"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16WithUnicodeChars) {
	EXPECT_EQ(SwapByteOrder(u"Привет мир!"), EncodeUtf16(U"Привет мир!"));
	EXPECT_EQ(SwapByteOrder(u"世界，您好！"), EncodeUtf16(U"世界，您好！"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16WithSurrogates) {
	EXPECT_EQ(SwapByteOrder(u"😀😎🙋"), EncodeUtf16(U"😀😎🙋"));
}


//-----------------------------------------------------------------------------
// UTF-16 BE: Tests decoding string
//-----------------------------------------------------------------------------
TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16WithAnsiChars) {
	EXPECT_EQ(L"Hello world!", DecodeUtf16As<std::wstring>(SwapByteOrder(u"Hello world!")));
	EXPECT_EQ(u"Hello world!", DecodeUtf16As<std::u16string>(SwapByteOrder(u"Hello world!")));
	EXPECT_EQ(U"Hello world!", DecodeUtf16As<std::u32string>(SwapByteOrder(u"Hello world!")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16WithUnicodeChars) {
	EXPECT_EQ(U"Привет мир!", DecodeUtf16As<std::u32string>(SwapByteOrder(u"Привет мир!")));
	EXPECT_EQ(U"世界，您好！", DecodeUtf16As<std::u32string>(SwapByteOrder(u"世界，您好！")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16WithSurrogates) {
	EXPECT_EQ(U"😀😎🙋", DecodeUtf16As<std::u32string>(SwapByteOrder(u"😀😎🙋")));
}

TEST_F(Utf16BeDecodeTest, ShouldLeaveSurrogatesWhenTargetStringIsUtf16) {
	EXPECT_EQ(L"😀😎🙋", DecodeUtf16As<std::wstring>(SwapByteOrder(u"😀😎🙋")));
	EXPECT_EQ(u"😀😎🙋", DecodeUtf16As<std::u16string>(SwapByteOrder(u"😀😎🙋")));
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
