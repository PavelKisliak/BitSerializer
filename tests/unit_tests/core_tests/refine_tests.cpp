/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/refine.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests for 'Fallback' post-processor
//-----------------------------------------------------------------------------
TEST(RefineFallback, ShouldSetFallbackValueWhenNotDeserialized)
{
	// Arrange
	constexpr auto fallback = Refine::Fallback(1.0f);
	float testValue = 0;

	// Act
	fallback(testValue, false);

	// Assert
	EXPECT_EQ(1.0f, testValue);
}

TEST(RefineFallback, ShouldDoNothingWhenDeserializedValue)
{
	// Arrange
	constexpr auto fallback = Refine::Fallback(1.0f);
	float testValue = 100.f;

	// Act
	fallback(testValue, true);

	// Assert
	EXPECT_EQ(100.f, testValue);
}

TEST(RefineFallback, ShouldSetFallbackValueFromCompatibleType)
{
	// Arrange
	constexpr auto fallback = Refine::Fallback("default");
	std::string testValue = "test";

	// Act
	fallback(testValue, false);

	// Assert
	EXPECT_EQ("default", testValue);
}

TEST(RefineFallback, ShouldConstructFallbackValueFromVarArgs)
{
	// Arrange
	auto fallback = Refine::Fallback<std::string>(3, '-');
	std::string testValue = "test";

	// Act
	fallback(testValue, false);

	// Assert
	EXPECT_EQ("---", testValue);
}

//-----------------------------------------------------------------------------
// Tests for 'TrimWhitespace' refiner
//-----------------------------------------------------------------------------
TEST(RefineTrimWhitespace, ShouldDoNothingWhenNotDeserialized)
{
	// Arrange
	constexpr auto trimWhitespace = Refine::TrimWhitespace();
	std::string testValue = " test ";

	// Act
	trimWhitespace(testValue, false);

	// Assert
	EXPECT_EQ(" test ", testValue);
}

TEST(RefineTrimWhitespace, ShouldTrimUtf8String)
{
	// Arrange
	constexpr auto trimWhitespace = Refine::TrimWhitespace();
	std::string testValue = "\t test \t\n";

	// Act
	trimWhitespace(testValue, true);

	// Assert
	EXPECT_EQ("test", testValue);
}

TEST(RefineTrimWhitespace, ShouldTrimUtf16String)
{
	// Arrange
	constexpr auto trimWhitespace = Refine::TrimWhitespace();
	std::u16string testValue = u"Hello world!\t\n";

	// Act
	trimWhitespace(testValue, true);

	// Assert
	EXPECT_EQ(u"Hello world!", testValue);
}

TEST(RefineTrimWhitespace, ShouldTrimUtf32String)
{
	// Arrange
	constexpr auto trimWhitespace = Refine::TrimWhitespace();
	std::u32string testValue = U"\t t \t\n";

	// Act
	trimWhitespace(testValue, true);

	// Assert
	EXPECT_EQ(U"t", testValue);
}

TEST(RefineTrimWhitespace, ShouldHandleSingleCharacterString)
{
	// Arrange
	constexpr auto trimWhitespace = Refine::TrimWhitespace();
	std::string testValue = "A";

	// Act
	trimWhitespace(testValue, true);

	// Assert
	EXPECT_EQ("A", testValue);
}

TEST(RefineTrimWhitespace, ShouldHandleAllWhitespaceString)
{
	// Arrange
	constexpr auto trimWhitespace = Refine::TrimWhitespace();
	std::string testValue = " \t\n\v\f\r ";

	// Act
	trimWhitespace(testValue, true);

	// Assert
	EXPECT_EQ("", testValue);
}

TEST(RefineTrimWhitespace, ShouldIgnoreEmptyString)
{
	// Arrange
	constexpr auto trimWhitespace = Refine::TrimWhitespace();
	std::string testValue;

	// Act
	trimWhitespace(testValue, true);

	// Assert
	EXPECT_EQ("", testValue);
}
