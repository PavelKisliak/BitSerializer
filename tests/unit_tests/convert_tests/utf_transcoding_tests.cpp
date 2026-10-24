/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;

#pragma warning(push)
#pragma warning(disable: 4566)
//-----------------------------------------------------------------------------
// Test conversions any UTF string to any other UTF format
//-----------------------------------------------------------------------------
TEST(UtfTranscodeTest, TransocodeUt8ToUtf8)
{
	// Arrange
	std::string outString = UTF8("Привет ");
	const std::string_view source = UTF8("мир!");

	// Act
	const auto result = Convert::Utf::Transcode(source, outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(UtfTranscodeTest, TransocodeUtf8ToUtf16)
{
	// Arrange
	std::u16string outString = u"Привет ";
	const std::string_view source = UTF8("мир!");

	// Act
	const auto result = Convert::Utf::Transcode(source, outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"Привет мир!", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(UtfTranscodeTest, TransocodeUtf8ToUtf32)
{
	// Arrange
	std::u32string outString = U"Привет ";
	const std::string_view source = UTF8("мир!");

	// Act
	const auto result = Convert::Utf::Transcode(source, outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"Привет мир!", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(UtfTranscodeTest, TransocodeUtf16ToUtf8)
{
	// Arrange
	std::string outString = UTF8("Привет ");
	const std::u16string_view source = u"мир!";

	// Act
	const auto result = Convert::Utf::Transcode(source, outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(UtfTranscodeTest, TransocodeUtf16ToUtf32)
{
	// Arrange
	std::u32string outString = U"Привет ";
	const std::u16string_view source = u"мир!";

	// Act
	const auto result = Convert::Utf::Transcode(source, outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(U"Привет мир!", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(UtfTranscodeTest, TransocodeUtf32ToUtf8)
{
	// Arrange
	std::string outString = UTF8("Привет ");
	const std::u32string_view source = U"мир!";

	// Act
	const auto result = Convert::Utf::Transcode(source, outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(UTF8("Привет мир!"), outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(UtfTranscodeTest, TransocodeUtf32ToUtf16)
{
	// Arrange
	std::u16string outString = u"Привет ";
	const std::u32string_view source = U"мир!";

	// Act
	const auto result = Convert::Utf::Transcode(source, outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(u"Привет мир!", outString);
	EXPECT_EQ(static_cast<ptrdiff_t>(source.size()), std::distance(std::begin(source), result.Iterator));
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

//-----------------------------------------------------------------------------
// Tests of transcoding between same-width but different character types
// (e.g. char16_t and wchar_t on Windows, char32_t and wchar_t on Linux)
//-----------------------------------------------------------------------------
TEST(UtfTranscodeTest, TransocodeUtf16ToWide)
{
	// Arrange
	const std::u16string_view source = u"Привет 😀";
	std::wstring outString;

	// Act
	const auto result = Convert::Utf::Transcode(source, outString);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(L"Привет 😀", outString);
	EXPECT_EQ(0U, result.InvalidSequencesCount);
}

TEST(UtfTranscodeTest, TransocodeUtf16ToWideWithInvalidSequenceAndSkipPolicy)
{
	// Arrange
	const std::u16string wrongUtf16({ Convert::Utf::UnicodeTraits::LowSurrogatesEnd, Convert::Utf::UnicodeTraits::LowSurrogatesStart });
	const std::u16string source = wrongUtf16 + u"test_value" + wrongUtf16;
	const std::u16string_view sourceView = source;
	std::wstring outString;

	// Act
	const auto result = Convert::Utf::Transcode(sourceView, outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(L"☐☐test_value☐☐", outString);
	EXPECT_EQ(sourceView.size(), static_cast<size_t>(std::distance(sourceView.begin(), result.Iterator)));
	EXPECT_EQ(4U, result.InvalidSequencesCount);
}

TEST(UtfTranscodeTest, TransocodeUtf16ToWideWithInvalidSequenceAndThrowPolicy)
{
	// Arrange
	const std::u16string wrongUtf16({ Convert::Utf::UnicodeTraits::LowSurrogatesEnd, Convert::Utf::UnicodeTraits::LowSurrogatesStart });
	const std::u16string source = wrongUtf16 + u"test_value" + wrongUtf16;
	std::wstring outString;

	// Act
	const auto result = Convert::Utf::Transcode(std::u16string_view(source), outString, Convert::Utf::UtfEncodingErrorPolicy::ThrowError);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::InvalidSequence, result.ErrorCode);
}

TEST(UtfTranscodeTest, TransocodeUtf32ToWideWithInvalidCodePointAndSkipPolicy)
{
	// Arrange
	std::u32string wrongUtf32;
	wrongUtf32.push_back(0xD800);		// Surrogate code point
	wrongUtf32.push_back(0x110000);		// Out of Unicode range
	const std::u32string source = wrongUtf32 + U"test_value";
	const std::u32string_view sourceView = source;
	std::wstring outString;

	// Act
	const auto result = Convert::Utf::Transcode(sourceView, outString, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Assert
	EXPECT_TRUE(result);
	EXPECT_EQ(L"☐☐test_value", outString);
	EXPECT_EQ(sourceView.size(), static_cast<size_t>(std::distance(sourceView.begin(), result.Iterator)));
	EXPECT_EQ(2U, result.InvalidSequencesCount);
}

TEST(UtfTranscodeTest, TransocodeUtf32ToWideWithInvalidCodePointAndThrowPolicy)
{
	// Arrange
	std::u32string wrongUtf32;
	wrongUtf32.push_back(0xD800);		// Surrogate code point
	wrongUtf32.push_back(0x110000);		// Out of Unicode range
	const std::u32string source = wrongUtf32 + U"test_value";
	std::wstring outString;

	// Act
	const auto result = Convert::Utf::Transcode(std::u32string_view(source), outString, Convert::Utf::UtfEncodingErrorPolicy::ThrowError);

	// Assert
	EXPECT_FALSE(result);
	EXPECT_EQ(Convert::Utf::UtfEncodingErrorCode::InvalidSequence, result.ErrorCode);
}

#pragma warning(pop)
