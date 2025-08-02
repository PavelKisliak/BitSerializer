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
