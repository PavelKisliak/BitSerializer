/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"
#include "testing_tools/string_utils.h"

using namespace BitSerializer;

// UTF-32 encode test fixture
template <class TEncoder>
class Utf32EncodeBaseFixture : public testing::Test
{
protected:
	template <typename TOutStr = std::u32string>
	static TOutStr EncodeUtf32(const std::string& utf8string, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutStr::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutStr::value_type>())
	{
		TOutStr result;
		TEncoder::Encode(utf8string.begin(), utf8string.end(), result, encodePolicy, errorMark);
		return result;
	}

	template <typename TOutStr = std::u32string>
	static TOutStr EncodeUtf32(const std::wstring& unicodeStr, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutStr::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutStr::value_type>())
	{
		TOutStr result;
		TEncoder::Encode(unicodeStr.begin(), unicodeStr.end(), result, encodePolicy, errorMark);
		return result;
	}

	template <typename TOutStr = std::u32string>
	static TOutStr EncodeUtf32(const std::u16string& unicodeStr, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutStr::value_type * errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutStr::value_type>())
	{
		TOutStr result;
		TEncoder::Encode(unicodeStr.begin(), unicodeStr.end(), result, encodePolicy, errorMark);
		return result;
	}

	template <typename TOutStr = std::u32string>
	static TOutStr EncodeUtf32(const std::u32string& unicodeStr, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutStr::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutStr::value_type>())
	{
		TOutStr result;
		TEncoder::Encode(unicodeStr.begin(), unicodeStr.end(), result, encodePolicy, errorMark);
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
	static TOutputString DecodeUtf32As(const std::u32string& Utf32Str, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::ThrowException,
		const typename TOutputString::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutputString::value_type>())
	{
		TOutputString result;
		TEncoder::Decode(Utf32Str.begin(), Utf32Str.end(), result, encodePolicy, errorMark);
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
	EXPECT_EQ(U"Привет мир!", EncodeUtf32(UTF8("Привет мир!")));
	EXPECT_EQ(U"世界，您好！", EncodeUtf32(UTF8("世界，您好！")));
}

TEST_F(Utf32LeEncodeTest, ShouldEncodeUtf32FromUtf16) {
	EXPECT_EQ(U"Привет мир!", EncodeUtf32(u"Привет мир!"));
	EXPECT_EQ(U"世界，您好！", EncodeUtf32(u"世界，您好！"));
}

TEST_F(Utf32LeEncodeTest, ShouldEncodeUtf32FromUtf16Surrogates) {
	EXPECT_EQ(U"😀😎🙋", EncodeUtf32(u"😀😎🙋"));
}

TEST_F(Utf32LeEncodeTest, ShouldEncodeUtf32FromWString) {
	EXPECT_EQ(U"Привет мир!", EncodeUtf32(L"Привет мир!"));
	EXPECT_EQ(U"世界，您好！", EncodeUtf32(L"世界，您好！"));
}

TEST_F(Utf32LeEncodeTest, ShouldEncodeUtf32FromUtf32AsIs) {
	EXPECT_EQ(U"Привет мир!", EncodeUtf32(U"Привет мир!"));
	EXPECT_EQ(U"世界，您好！", EncodeUtf32(U"世界，您好！"));
}

TEST_F(Utf32LeEncodeTest, ShouldPutErrorMarkWhenSurrogateStartsWithWrongCode) {
	const std::u16string wrongStartCodes({ Convert::Unicode::LowSurrogatesEnd, Convert::Unicode::LowSurrogatesStart });
	EXPECT_EQ(U"☐☐test☐☐", EncodeUtf32(wrongStartCodes + u"test" + wrongStartCodes, Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf32LeEncodeTest, ShouldPutErrorMarkWhenNoSecondCodeInSurrogate) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(U"☐test", EncodeUtf32(notFullSurrogatePair + u"test", Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf32LeEncodeTest, ShouldHandlePolicyThrowException) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_THROW(EncodeUtf32(notFullSurrogatePair + u"test", Convert::EncodeErrorPolicy::ThrowException), std::runtime_error);
}

TEST_F(Utf32LeEncodeTest, ShouldHandlePolicySkip) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(U"test", EncodeUtf32(notFullSurrogatePair + u"test", Convert::EncodeErrorPolicy::Skip));
}

TEST_F(Utf32LeEncodeTest, ShouldReturnIteratorToEnd)
{
	// Arrange
	constexpr std::u16string_view testStr = u"test";

	// Act
	std::u32string actualStr;
	const auto actualIt = Convert::Utf32Le::Encode(testStr.cbegin(), testStr.cend(), actualStr);

	// Assert
	EXPECT_TRUE(actualIt == testStr.cend());
}


//-----------------------------------------------------------------------------
// UTF-32 LE: Tests decoding string
//-----------------------------------------------------------------------------
TEST_F(Utf32LeDecodeTest, ShouldDecodeUtf32ToAnsi) {
	EXPECT_EQ("Hello world!", DecodeUtf32As<std::string>(U"Hello world!"));
}

TEST_F(Utf32LeDecodeTest, ShouldDecodeUtf32ToUtf8) {
	EXPECT_EQ(UTF8("Привет мир!"), DecodeUtf32As<std::string>(U"Привет мир!"));
	EXPECT_EQ(UTF8("世界，您好！"), DecodeUtf32As<std::string>(U"世界，您好！"));
}

TEST_F(Utf32LeDecodeTest, ShouldDecodeUtf32ToUtf16) {
	EXPECT_EQ(u"Hello world!", DecodeUtf32As<std::u16string>(U"Hello world!"));
	EXPECT_EQ(u"Привет мир!", DecodeUtf32As<std::u16string>(U"Привет мир!"));
	EXPECT_EQ(u"世界，您好！", DecodeUtf32As<std::u16string>(U"世界，您好！"));
}

TEST_F(Utf32LeDecodeTest, ShouldDecodeUtf32ToUtf16WithSurrogates) {
	EXPECT_EQ(u"😀😎🙋", DecodeUtf32As<std::u16string>(U"😀😎🙋"));
}

TEST_F(Utf32LeDecodeTest, ShouldDecodeUtf32ToWString) {
	EXPECT_EQ(L"Hello world!", DecodeUtf32As<std::wstring>(U"Hello world!"));
	EXPECT_EQ(L"Привет мир!", DecodeUtf32As<std::wstring>(U"Привет мир!"));
	EXPECT_EQ(L"世界，您好！", DecodeUtf32As<std::wstring>(U"世界，您好！"));
}

TEST_F(Utf32LeDecodeTest, ShouldDecodeUtf32ToUtf32AsIs) {
	EXPECT_EQ(U"Привет мир!", DecodeUtf32As<std::u32string>(U"Привет мир!"));
	EXPECT_EQ(U"世界，您好！", DecodeUtf32As<std::u32string>(U"世界，您好！"));
}

TEST_F(Utf32LeDecodeTest, ShouldReturnIteratorToEnd)
{
	// Arrange
	constexpr std::u32string_view testStr = U"test";

	// Act
	std::u16string actualStr;
	const auto actualIt = Convert::Utf32Le::Decode(testStr.cbegin(), testStr.cend(), actualStr);

	// Assert
	EXPECT_TRUE(actualIt == testStr.cend());
}

//-----------------------------------------------------------------------------
// UTF-32 BE: Tests for encoding string
//-----------------------------------------------------------------------------
TEST_F(Utf32BeEncodeTest, ShouldEncodeUtf32BeFromAnsi) {
	EXPECT_EQ(SwapByteOrder(U"Hello world!"), EncodeUtf32("Hello world!"));
}

TEST_F(Utf32BeEncodeTest, ShouldEncodeUtf32BeFromUtf8) {
	EXPECT_EQ(SwapByteOrder(U"Привет мир!"), EncodeUtf32(UTF8("Привет мир!")));
	EXPECT_EQ(SwapByteOrder(U"世界，您好！"), EncodeUtf32(UTF8("世界，您好！")));
}

TEST_F(Utf32BeEncodeTest, ShouldEncodeUtf32BeFromUtf16) {
	EXPECT_EQ(SwapByteOrder(U"Hello world!"), EncodeUtf32(u"Hello world!"));
	EXPECT_EQ(SwapByteOrder(U"Привет мир!"), EncodeUtf32(u"Привет мир!"));
	EXPECT_EQ(SwapByteOrder(U"世界，您好！"), EncodeUtf32(u"世界，您好！"));
}

TEST_F(Utf32BeEncodeTest, ShouldEncodeUtf32BeFromUtf16WithSurrogates) {
	EXPECT_EQ(SwapByteOrder(U"😀😎🙋"), EncodeUtf32(u"😀😎🙋"));
}

TEST_F(Utf32BeEncodeTest, ShouldEncodeUtf32BeFromWString) {
	EXPECT_EQ(SwapByteOrder(U"Привет мир!"), EncodeUtf32(L"Привет мир!"));
	EXPECT_EQ(SwapByteOrder(U"世界，您好！"), EncodeUtf32(L"世界，您好！"));
}

TEST_F(Utf32BeEncodeTest, ShouldEncodeUtf32BeFromUtf32Le) {
	EXPECT_EQ(SwapByteOrder(U"Привет мир!"), EncodeUtf32(U"Привет мир!"));
	EXPECT_EQ(SwapByteOrder(U"世界，您好！"), EncodeUtf32(U"世界，您好！"));
}

TEST_F(Utf32BeEncodeTest, ShouldPutErrorMarkWhenSurrogateStartsWithWrongCode) {
	const std::u16string wrongStartCodes({ Convert::Unicode::LowSurrogatesEnd, Convert::Unicode::LowSurrogatesStart });
	EXPECT_EQ(SwapByteOrder(U"☐☐test☐☐"), EncodeUtf32(wrongStartCodes + u"test" + wrongStartCodes, Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf32BeEncodeTest, ShouldPutErrorMarkWhenNoSecondCodeInSurrogate) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(SwapByteOrder(U"☐test"), EncodeUtf32(notFullSurrogatePair + u"test", Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf32BeEncodeTest, ShouldHandlePolicyThrowException) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_THROW(EncodeUtf32(notFullSurrogatePair + u"test", Convert::EncodeErrorPolicy::ThrowException), std::runtime_error);
}

TEST_F(Utf32BeEncodeTest, ShouldHandlePolicySkip) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(SwapByteOrder(U"test"), EncodeUtf32(notFullSurrogatePair + u"test", Convert::EncodeErrorPolicy::Skip));
}

TEST_F(Utf32BeEncodeTest, ShouldReturnIteratorToEnd)
{
	// Arrange
	constexpr std::u16string_view testStr = u"test";

	// Act
	std::u32string actualStr;
	const auto actualIt = Convert::Utf32Be::Encode(testStr.cbegin(), testStr.cend(), actualStr);

	// Assert
	EXPECT_TRUE(actualIt == testStr.cend());
}

//-----------------------------------------------------------------------------
// UTF-32 BE: Tests decoding string
//-----------------------------------------------------------------------------
TEST_F(Utf32BeDecodeTest, ShouldDecodeUtf32BeToAnsi) {
	EXPECT_EQ("Hello world!", DecodeUtf32As<std::string>(SwapByteOrder(U"Hello world!")));
}

TEST_F(Utf32BeDecodeTest, ShouldDecodeUtf32BeToUtf8) {
	EXPECT_EQ(UTF8("Привет мир!"), DecodeUtf32As<std::string>(SwapByteOrder(U"Привет мир!")));
	EXPECT_EQ(UTF8("世界，您好！"), DecodeUtf32As<std::string>(SwapByteOrder(U"世界，您好！")));
}

TEST_F(Utf32BeDecodeTest, ShouldDecodeUtf32BeToUtf16) {
	EXPECT_EQ(u"Hello world!", DecodeUtf32As<std::u16string>(SwapByteOrder(U"Hello world!")));
	EXPECT_EQ(u"Привет мир!", DecodeUtf32As<std::u16string>(SwapByteOrder(U"Привет мир!")));
	EXPECT_EQ(u"世界，您好！", DecodeUtf32As<std::u16string>(SwapByteOrder(U"世界，您好！")));
}

TEST_F(Utf32BeDecodeTest, ShouldDecodeUtf32BeToUtf16WithSurrogates) {
	EXPECT_EQ(u"😀😎🙋", DecodeUtf32As<std::u16string>(SwapByteOrder(U"😀😎🙋")));
}

TEST_F(Utf32BeDecodeTest, ShouldDecodeUtf32BeToWString) {
	EXPECT_EQ(L"Hello world!", DecodeUtf32As<std::wstring>(SwapByteOrder(U"Hello world!")));
	EXPECT_EQ(L"Привет мир!", DecodeUtf32As<std::wstring>(SwapByteOrder(U"Привет мир!")));
	EXPECT_EQ(L"世界，您好！", DecodeUtf32As<std::wstring>(SwapByteOrder(U"世界，您好！")));
}

TEST_F(Utf32BeDecodeTest, ShouldDecodeUtf32BeToUtf32Le) {
	EXPECT_EQ(U"Привет мир!", DecodeUtf32As<std::u32string>(SwapByteOrder(U"Привет мир!")));
	EXPECT_EQ(U"世界，您好！", DecodeUtf32As<std::u32string>(SwapByteOrder(U"世界，您好！")));
}

TEST_F(Utf32BeDecodeTest, ShouldReturnIteratorToEnd)
{
	// Arrange
	const std::u32string testStr = SwapByteOrder(U"test");

	// Act
	std::u32string actualStr;
	const auto actualIt = Convert::Utf32Be::Decode(testStr.cbegin(), testStr.cend(), actualStr);

	// Assert
	EXPECT_TRUE(actualIt == testStr.cend());
}

#pragma warning(pop)
