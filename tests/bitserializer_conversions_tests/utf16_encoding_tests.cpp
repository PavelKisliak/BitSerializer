/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"
#include "testing_tools/string_utils.h"

using namespace BitSerializer;

// UTF-16 encode test fixture
template <class TEncoder>
class Utf16EncodeBaseFixture : public testing::Test
{
protected:
	template <typename TOutStr = std::u16string>
	static TOutStr EncodeUtf16(const std::string& srcStr, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutStr::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutStr::value_type>())
	{
		TOutStr result;
		TEncoder::Encode(srcStr.begin(), srcStr.end(), result, encodePolicy, errorMark);
		return result;
	}

	template <typename TOutStr = std::u16string>
	static TOutStr EncodeUtf16(const std::wstring& srcStr, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutStr::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutStr::value_type>())
	{
		TOutStr result;
		TEncoder::Encode(srcStr.begin(), srcStr.end(), result, encodePolicy, errorMark);
		return result;
	}

	template <typename TOutStr = std::u16string>
	static TOutStr EncodeUtf16(const std::u16string& srcStr, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutStr::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutStr::value_type>())
	{
		TOutStr result;
		TEncoder::Encode(srcStr.begin(), srcStr.end(), result, encodePolicy, errorMark);
		return result;
	}

	template <typename TOutStr = std::u16string>
	static TOutStr EncodeUtf16(const std::u32string& srcStr, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::WriteErrorMark,
		const typename TOutStr::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutStr::value_type>())
	{
		TOutStr result;
		TEncoder::Encode(srcStr.begin(), srcStr.end(), result, encodePolicy, errorMark);
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
	static TOutputString DecodeUtf16As(const std::u16string& utf16Str, Convert::EncodeErrorPolicy encodePolicy = Convert::EncodeErrorPolicy::ThrowException,
		const typename TOutputString::value_type* errorMark = Convert::Detail::GetDefaultErrorMark<typename TOutputString::value_type>())
	{
		TOutputString result;
		TEncoder::Decode(utf16Str.begin(), utf16Str.end(), result, encodePolicy, errorMark);
		return result;
	}
};

class Utf16LeDecodeTest : public Utf16DecodeBaseFixture<Convert::Utf16Le> {};
class Utf16BeDecodeTest : public Utf16DecodeBaseFixture<Convert::Utf16Be> {};

#pragma warning(push)
#pragma warning(disable: 4566)
//-----------------------------------------------------------------------------
// UTF-16 LE: Tests for encoding string
//-----------------------------------------------------------------------------
TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromAnsi) {
	EXPECT_EQ(NativeStringToLittleEndian(u"Hello world!"), EncodeUtf16("Hello world!"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf8) {
	EXPECT_EQ(NativeStringToLittleEndian(u"Привет мир!"), EncodeUtf16(UTF8("Привет мир!")));
	EXPECT_EQ(NativeStringToLittleEndian(u"世界，您好！"), EncodeUtf16(UTF8("世界，您好！")));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf8Surrogates) {
	EXPECT_EQ(NativeStringToLittleEndian(u"😀😎🙋"), EncodeUtf16(UTF8("😀😎🙋")));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf16) {
	EXPECT_EQ(NativeStringToLittleEndian(u"Привет мир!"), EncodeUtf16(u"Привет мир!"));
	EXPECT_EQ(NativeStringToLittleEndian(u"世界，您好！"), EncodeUtf16(u"世界，您好！"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf16Surrogates) {
	EXPECT_EQ(NativeStringToLittleEndian(u"😀😎🙋"), EncodeUtf16(u"😀😎🙋"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromWString) {
	EXPECT_EQ(NativeStringToLittleEndian(u"Привет мир!"), EncodeUtf16(L"Привет мир!"));
	EXPECT_EQ(NativeStringToLittleEndian(u"世界，您好！"), EncodeUtf16(L"世界，您好！"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf32) {
	EXPECT_EQ(NativeStringToLittleEndian(u"Привет мир!"), EncodeUtf16(U"Привет мир!"));
	EXPECT_EQ(NativeStringToLittleEndian(u"世界，您好！"), EncodeUtf16(U"世界，您好！"));
}

TEST_F(Utf16LeEncodeTest, ShouldEncodeUtf16FromUtf32Surrogates) {
	EXPECT_EQ(NativeStringToLittleEndian(u"😀😎🙋"), EncodeUtf16(U"😀😎🙋"));
}

TEST_F(Utf16LeEncodeTest, ShouldReturnIteratorToEnd)
{
	// Arrange
	constexpr std::u32string_view testStr = U"test";

	// Act
	std::u16string actualStr;
	const auto actualIt = Convert::Utf16Le::Encode(testStr.cbegin(), testStr.cend(), actualStr);

	// Assert
	EXPECT_TRUE(actualIt == testStr.cend());
}


//-----------------------------------------------------------------------------
// UTF-16 LE: Tests decoding string
//-----------------------------------------------------------------------------
TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToAnsi) {
	EXPECT_EQ("Hello world!", DecodeUtf16As<std::string>(NativeStringToLittleEndian(u"Hello world!")));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf8) {
	EXPECT_EQ(UTF8("Привет мир!"), DecodeUtf16As<std::string>(NativeStringToLittleEndian(u"Привет мир!")));
	EXPECT_EQ(UTF8("世界，您好！"), DecodeUtf16As<std::string>(NativeStringToLittleEndian(u"世界，您好！")));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf16) {
	EXPECT_EQ(u"Hello world!", DecodeUtf16As<std::u16string>(NativeStringToLittleEndian(u"Hello world!")));
	EXPECT_EQ(u"Привет мир!", DecodeUtf16As<std::u16string>(NativeStringToLittleEndian(u"Привет мир!")));
	EXPECT_EQ(u"世界，您好！", DecodeUtf16As<std::u16string>(NativeStringToLittleEndian(u"世界，您好！")));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf16WithSurrogates) {
	EXPECT_EQ(u"😀😎🙋", DecodeUtf16As<std::u16string>(NativeStringToLittleEndian(u"😀😎🙋")));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToWString) {
	EXPECT_EQ(L"Hello world!", DecodeUtf16As<std::wstring>(NativeStringToLittleEndian(u"Hello world!")));
	EXPECT_EQ(L"Привет мир!", DecodeUtf16As<std::wstring>(NativeStringToLittleEndian(u"Привет мир!")));
	EXPECT_EQ(L"世界，您好！", DecodeUtf16As<std::wstring>(NativeStringToLittleEndian(u"世界，您好！")));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf32) {
	EXPECT_EQ(U"Hello world!", DecodeUtf16As<std::u32string>(NativeStringToLittleEndian(u"Hello world!")));
	EXPECT_EQ(U"Привет мир!", DecodeUtf16As<std::u32string>(NativeStringToLittleEndian(u"Привет мир!")));
	EXPECT_EQ(U"世界，您好！", DecodeUtf16As<std::u32string>(NativeStringToLittleEndian(u"世界，您好！")));
}

TEST_F(Utf16LeDecodeTest, ShouldDecodeUtf16ToUtf32WithSurrogates) {
	EXPECT_EQ(U"😀😎🙋", DecodeUtf16As<std::u32string>(NativeStringToLittleEndian(u"😀😎🙋")));
}

TEST_F(Utf16LeDecodeTest, ShouldPutErrorMarkWhenSurrogateStartsWithWrongCode) {
	const std::u16string wrongStartCodes({ Convert::Unicode::LowSurrogatesEnd, Convert::Unicode::LowSurrogatesStart });
	EXPECT_EQ(U"☐☐test☐☐", DecodeUtf16As<std::u32string>(NativeStringToLittleEndian(wrongStartCodes + u"test" + wrongStartCodes), Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf16LeDecodeTest, ShouldPutErrorMarkWhenNoSecondCodeInSurrogate) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(U"☐test", DecodeUtf16As<std::u32string>(NativeStringToLittleEndian(notFullSurrogatePair + u"test"), Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf16LeDecodeTest, ShouldPutCustomErrorMarkWhenError) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(U"<ERROR>test", DecodeUtf16As<std::u32string>(NativeStringToLittleEndian(notFullSurrogatePair + u"test"), Convert::EncodeErrorPolicy::WriteErrorMark, U"<ERROR>"));
}

TEST_F(Utf16LeDecodeTest, ShouldHandlePolicyThrowException) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_THROW(DecodeUtf16As<std::u32string>(NativeStringToLittleEndian(notFullSurrogatePair + u"test"), Convert::EncodeErrorPolicy::ThrowException), std::runtime_error);
}

TEST_F(Utf16LeDecodeTest, ShouldHandlePolicySkip) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(U"test", DecodeUtf16As<std::u32string>(NativeStringToLittleEndian(notFullSurrogatePair + u"test"), Convert::EncodeErrorPolicy::Skip));
}

TEST_F(Utf16LeDecodeTest, ShouldReturnIteratorToEnd)
{
	// Arrange
	const std::u16string testStr = NativeStringToLittleEndian(u"test");

	// Act
	std::u32string actualStr;
	const auto actualIt = Convert::Utf16Le::Decode(testStr.cbegin(), testStr.cend(), actualStr);

	// Assert
	EXPECT_TRUE(actualIt == testStr.cend());
}

TEST_F(Utf16LeDecodeTest, ShouldReturnIteratorToCroppedSurrogatePairAtEndWhenDecodeToUtf8)
{
	// Arrange
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string testStr = NativeStringToLittleEndian(u"test_тест" + croppedSequence);
	const size_t expectedPos = testStr.size() - croppedSequence.size();

	// Act
	std::string actual;
	const size_t actualPos = Convert::Utf16Le::Decode(testStr.cbegin(), testStr.cend(), actual) - testStr.cbegin();

	// Assert
	EXPECT_EQ(expectedPos, actualPos);
	EXPECT_EQ(UTF8("test_тест"), actual);
}
TEST_F(Utf16LeDecodeTest, ShouldReturnIteratorToCroppedSurrogatePairAtEndWhenDecodeToUtf16)
{
	// Arrange
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string testStr = NativeStringToLittleEndian(u"test_тест" + croppedSequence);
	const size_t expectedPos = testStr.size() - croppedSequence.size();

	// Act
	std::u16string actual;
	const size_t actualPos = Convert::Utf16Le::Decode(testStr.cbegin(), testStr.cend(), actual) - testStr.cbegin();

	// Assert
	EXPECT_EQ(expectedPos, actualPos);
	EXPECT_EQ(u"test_тест", actual);
}

TEST_F(Utf16LeDecodeTest, ShouldReturnIteratorToCroppedSurrogatePairAtEndWhenDecodeToUtf32)
{
	// Arrange
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string testStr = NativeStringToLittleEndian(u"test_тест" + croppedSequence);
	const size_t expectedPos = testStr.size() - croppedSequence.size();

	// Act
	std::u32string actual;
	const size_t actualPos = Convert::Utf16Le::Decode(testStr.cbegin(), testStr.cend(), actual) - testStr.cbegin();

	// Assert
	EXPECT_EQ(expectedPos, actualPos);
	EXPECT_EQ(U"test_тест", actual);
}

//-----------------------------------------------------------------------------
// UTF-16 BE: Tests for encoding string
//-----------------------------------------------------------------------------
TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromAnsi) {
	EXPECT_EQ(NativeStringToBigEndian(u"Hello world!"), EncodeUtf16("Hello world!"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf8) {
	EXPECT_EQ(NativeStringToBigEndian(u"Привет мир!"), EncodeUtf16(UTF8("Привет мир!")));
	EXPECT_EQ(NativeStringToBigEndian(u"世界，您好！"), EncodeUtf16(UTF8("世界，您好！")));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf8Surrogates) {
	EXPECT_EQ(NativeStringToBigEndian(u"😀😎🙋"), EncodeUtf16(UTF8("😀😎🙋")));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf16) {
	EXPECT_EQ(NativeStringToBigEndian(u"Привет мир!"), EncodeUtf16(u"Привет мир!"));
	EXPECT_EQ(NativeStringToBigEndian(u"世界，您好！"), EncodeUtf16(u"世界，您好！"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf16Surrogates) {
	EXPECT_EQ(NativeStringToBigEndian(u"😀😎🙋"), EncodeUtf16(u"😀😎🙋"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromWString) {
	EXPECT_EQ(NativeStringToBigEndian(u"Привет мир!"), EncodeUtf16(L"Привет мир!"));
	EXPECT_EQ(NativeStringToBigEndian(u"世界，您好！"), EncodeUtf16(L"世界，您好！"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf32) {
	EXPECT_EQ(NativeStringToBigEndian(u"Привет мир!"), EncodeUtf16(U"Привет мир!"));
	EXPECT_EQ(NativeStringToBigEndian(u"世界，您好！"), EncodeUtf16(U"世界，您好！"));
}

TEST_F(Utf16BeEncodeTest, ShouldEncodeUtf16BeFromUtf32Surrogates) {
	EXPECT_EQ(NativeStringToBigEndian(u"😀😎🙋"), EncodeUtf16(U"😀😎🙋"));
}

TEST_F(Utf16BeEncodeTest, ShouldReturnIteratorToEnd)
{
	// Arrange
	constexpr std::u32string_view testStr = U"test";

	// Act
	std::u16string actualStr;
	const auto actualIt = Convert::Utf16Be::Encode(testStr.cbegin(), testStr.cend(), actualStr);

	// Assert
	EXPECT_TRUE(actualIt == testStr.cend());
}


//-----------------------------------------------------------------------------
// UTF-16 BE: Tests decoding string
//-----------------------------------------------------------------------------
TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToAnsi) {
	EXPECT_EQ("Hello world!", DecodeUtf16As<std::string>(NativeStringToBigEndian(u"Hello world!")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf8) {
	EXPECT_EQ(UTF8("Привет мир!"), DecodeUtf16As<std::string>(NativeStringToBigEndian(u"Привет мир!")));
	EXPECT_EQ(UTF8("世界，您好！"), DecodeUtf16As<std::string>(NativeStringToBigEndian(u"世界，您好！")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf16Le) {
	EXPECT_EQ(u"Hello world!", DecodeUtf16As<std::u16string>(NativeStringToBigEndian(u"Hello world!")));
	EXPECT_EQ(u"Привет мир!", DecodeUtf16As<std::u16string>(NativeStringToBigEndian(u"Привет мир!")));
	EXPECT_EQ(u"世界，您好！", DecodeUtf16As<std::u16string>(NativeStringToBigEndian(u"世界，您好！")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf16LeWithSurrogates) {
	EXPECT_EQ(u"😀😎🙋", DecodeUtf16As<std::u16string>(NativeStringToBigEndian(u"😀😎🙋")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToWString) {
	EXPECT_EQ(L"Hello world!", DecodeUtf16As<std::wstring>(NativeStringToBigEndian(u"Hello world!")));
	EXPECT_EQ(L"Привет мир!", DecodeUtf16As<std::wstring>(NativeStringToBigEndian(u"Привет мир!")));
	EXPECT_EQ(L"世界，您好！", DecodeUtf16As<std::wstring>(NativeStringToBigEndian(u"世界，您好！")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf32) {
	EXPECT_EQ(U"Hello world!", DecodeUtf16As<std::u32string>(NativeStringToBigEndian(u"Hello world!")));
	EXPECT_EQ(U"Привет мир!", DecodeUtf16As<std::u32string>(NativeStringToBigEndian(u"Привет мир!")));
	EXPECT_EQ(U"世界，您好！", DecodeUtf16As<std::u32string>(NativeStringToBigEndian(u"世界，您好！")));
}

TEST_F(Utf16BeDecodeTest, ShouldDecodeUtf16BeToUtf32WithSurrogates) {
	EXPECT_EQ(U"😀😎🙋", DecodeUtf16As<std::u32string>(NativeStringToBigEndian(u"😀😎🙋")));
}

TEST_F(Utf16BeDecodeTest, ShouldPutErrorMarkWhenSurrogateStartsWithWrongCode) {
	const std::u16string wrongStartCodes({ Convert::Unicode::LowSurrogatesEnd, Convert::Unicode::LowSurrogatesStart });
	EXPECT_EQ(U"☐☐test☐☐",
		DecodeUtf16As<std::u32string>(NativeStringToBigEndian(wrongStartCodes + u"test" + wrongStartCodes), Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf16BeDecodeTest, ShouldPutErrorMarkWhenNoSecondCodeInSurrogate) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(U"☐test", DecodeUtf16As<std::u32string>(NativeStringToBigEndian(notFullSurrogatePair + u"test"), Convert::EncodeErrorPolicy::WriteErrorMark));
}

TEST_F(Utf16BeDecodeTest, ShouldHandlePolicyThrowException) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_THROW(DecodeUtf16As<std::u32string>(NativeStringToBigEndian(notFullSurrogatePair + u"test"), Convert::EncodeErrorPolicy::ThrowException), std::runtime_error);
}

TEST_F(Utf16BeDecodeTest, ShouldHandlePolicySkip) {
	const std::u16string notFullSurrogatePair({ Convert::Unicode::HighSurrogatesStart });
	EXPECT_EQ(U"test", DecodeUtf16As<std::u32string>(NativeStringToBigEndian(notFullSurrogatePair + u"test"), Convert::EncodeErrorPolicy::Skip));
}

TEST_F(Utf16BeDecodeTest, ShouldReturnIteratorToEnd)
{
	// Arrange
	const std::u16string testStr = NativeStringToBigEndian(u"test");

	// Act
	std::u32string actualStr;
	const auto actualIt = Convert::Utf16Be::Decode(testStr.cbegin(), testStr.cend(), actualStr);

	// Assert
	EXPECT_TRUE(actualIt == testStr.cend());
}

TEST_F(Utf16BeDecodeTest, ShouldReturnIteratorToCroppedSurrogatePairAtEndWhenDecodeToUtf8)
{
	// Arrange
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string testStr = NativeStringToBigEndian(u"test_тест" + croppedSequence);
	const size_t expectedPos = testStr.size() - croppedSequence.size();

	// Act
	std::string actual;
	const size_t actualPos = Convert::Utf16Be::Decode(testStr.cbegin(), testStr.cend(), actual) - testStr.cbegin();

	// Assert
	EXPECT_EQ(expectedPos, actualPos);
	EXPECT_EQ(UTF8("test_тест"), actual);
}

TEST_F(Utf16BeDecodeTest, ShouldReturnIteratorToCroppedSurrogatePairAtEndWhenDecodeToUtf16)
{
	// Arrange
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string testStr = NativeStringToBigEndian(u"test_тест" + croppedSequence);
	const size_t expectedPos = testStr.size() - croppedSequence.size();

	// Act
	std::u16string actual;
	const size_t actualPos = Convert::Utf16Be::Decode(testStr.cbegin(), testStr.cend(), actual) - testStr.cbegin();

	// Assert
	EXPECT_EQ(expectedPos, actualPos);
	EXPECT_EQ(u"test_тест", actual);
}

TEST_F(Utf16BeDecodeTest, ShouldReturnIteratorToCroppedSurrogatePairAtEndWhenDecodeToUtf32)
{
	// Arrange
	const std::u16string croppedSequence({ 0xD83D });
	const std::u16string testStr = NativeStringToBigEndian(u"test_тест" + croppedSequence);
	const size_t expectedPos = testStr.size() - croppedSequence.size();

	// Act
	std::u32string actual;
	const size_t actualPos = Convert::Utf16Be::Decode(testStr.cbegin(), testStr.cend(), actual) - testStr.cbegin();

	// Assert
	EXPECT_EQ(expectedPos, actualPos);
	EXPECT_EQ(U"test_тест", actual);
}

#pragma warning(pop)
