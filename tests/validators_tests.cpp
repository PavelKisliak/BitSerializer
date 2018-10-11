/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "pch.h"
#include "bitserializer/serialization_detail/validators.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests for 'Required' validator
//-----------------------------------------------------------------------------
TEST(ValidatorRequired, ShouldNotReturnErrorIfValueIsLoaded)
{
	// Arrange
	auto validator = Required();

	// Act
	auto result = validator(10, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorRequired, ShouldReturnErrorIfValueIsNotLoaded)
{
	// Arrange
	auto validator = Required();

	// Act
	auto result = validator(10, false);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_FALSE(result->empty());
}

//-----------------------------------------------------------------------------
// Tests for 'Range' validator
//-----------------------------------------------------------------------------
TEST(ValidatorRange, ShouldNotReturnErrorIfValueIsInRangeLoaded)
{
	// Arrange
	auto validator = Range<int>(1, 2);

	// Act
	auto result = validator(1, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorRange, ShouldReturnErrorIfValueIsLessThanMin)
{
	// Arrange
	auto validator = Range<int>(10, 20);

	// Act
	auto result = validator(5, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_FALSE(result->empty());
}

TEST(ValidatorRange, ShouldReturnErrorIfValueIsEqualToMax)
{
	// Arrange
	auto validator = Range<int>(10, 20);

	// Act
	auto result = validator(20, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_FALSE(result->empty());
}

TEST(ValidatorRange, ShouldReturnErrorIfValueIsGreaterThanMax)
{
	// Arrange
	auto validator = Range<int>(10, 20);

	// Act
	auto result = validator(21, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_FALSE(result->empty());
}

//-----------------------------------------------------------------------------
// Tests for 'MinSize' validator
//-----------------------------------------------------------------------------
TEST(ValidatorMinSize, ShouldNotReturnErrorIfSizeIsEqual)
{
	// Arrange
	auto validator = MinSize(10);
	std::string testValue(10, '#');

	// Act
	auto result = validator(testValue, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorMinSize, ShouldNotReturnErrorIfSizeIsGreater)
{
	// Arrange
	auto validator = MinSize(10);
	std::string testValue(11, '#');

	// Act
	auto result = validator(testValue, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorMinSize, ShouldReturnErrorIfSizeIsLess)
{
	// Arrange
	auto validator = MinSize(10);
	std::string testValue(9, '#');

	// Act
	auto result = validator(testValue, true);

	// Assert
	EXPECT_TRUE(result.has_value());
}

//-----------------------------------------------------------------------------
// Tests for 'MaxSize' validator
//-----------------------------------------------------------------------------
TEST(ValidatorMaxSize, ShouldNotReturnErrorIfSizeIsEqual)
{
	// Arrange
	auto validator = MaxSize(10);
	std::string testValue(10, '#');

	// Act
	auto result = validator(testValue, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorMaxSize, ShouldNotReturnErrorIfSizeIsLess)
{
	// Arrange
	auto validator = MaxSize(10);
	std::string testValue(9, '#');

	// Act
	auto result = validator(testValue, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorMaxSize, ShouldReturnErrorIfSizeIsGreater)
{
	// Arrange
	auto validator = MaxSize(10);
	std::string testValue(11, '#');

	// Act
	auto result = validator(testValue, true);

	// Assert
	EXPECT_TRUE(result.has_value());
}