﻿/*******************************************************************************
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

#pragma warning(pop)
