/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
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
TEST(ValidatorRange, ShouldAlwaysPassIfValueIsNotLoaded)
{
	// Arrange
	auto validator = Range(10, 20);

	// Act
	auto result = validator(0, false);

	// Assert
	EXPECT_FALSE(result.has_value());
}


TEST(ValidatorRange, ShouldNotReturnErrorIfValueIsInRangeLoaded)
{
	// Arrange
	auto validator = Range(1, 1);

	// Act
	auto result = validator(1, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorRange, ShouldReturnErrorIfValueIsLessThanMin)
{
	// Arrange
	auto validator = Range(10, 20);

	// Act
	auto result = validator(9, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_FALSE(result->empty());
}

TEST(ValidatorRange, ShouldReturnErrorIfValueIsGreaterThanMax)
{
	// Arrange
	auto validator = Range(10, 20);

	// Act
	auto result = validator(21, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_FALSE(result->empty());
}

//-----------------------------------------------------------------------------
// Tests for 'MinSize' validator
//-----------------------------------------------------------------------------
TEST(ValidatorMinSize, ShouldAlwaysPassIfValueIsNotLoaded)
{
	// Arrange
	auto validator = MinSize(10);
	std::string testValue(9, '#');

	// Act
	auto result = validator(testValue, false);

	// Assert
	EXPECT_FALSE(result.has_value());
}

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
TEST(ValidatorMaxSize, ShouldAlwaysPassIfValueIsNotLoaded)
{
	// Arrange
	auto validator = MaxSize(10);
	std::string testValue(11, '#');

	// Act
	auto result = validator(testValue, false);

	// Assert
	EXPECT_FALSE(result.has_value());
}

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

//-----------------------------------------------------------------------------
// Tests for 'Email' validator
//-----------------------------------------------------------------------------
TEST(ValidatorEmail, TestDifferentStringTypes)
{
	// Arrange
	constexpr auto validator = Email();

	// Act / Assert
	EXPECT_FALSE(validator("simple@example.com", true).has_value());
	EXPECT_FALSE(validator(std::string("simple@example.com"), true).has_value());
	EXPECT_FALSE(validator(std::u16string(u"simple@example.com"), true).has_value());
	EXPECT_FALSE(validator(std::u32string(U"simple@example.com"), true).has_value());
	EXPECT_FALSE(validator(std::wstring(L"simple@example.com"), true).has_value());
}

TEST(ValidatorEmail, TestValidEmails)
{
	// Arrange
	constexpr auto validator = Email();

	// Test local part
	EXPECT_FALSE(validator("simple@example.com", true).has_value());
	EXPECT_FALSE(validator("very.common@example.com", true).has_value());
	EXPECT_FALSE(validator("ABCDEFGHIJKLMNOPQRSTUVWXYZ.abcdefghijklmnopqrstuvwxyz@ABCDEFGHIJKLMNOPQRSTUVWXYZ.abcdefghijklmnopqrstuvwxyz", true).has_value());
	EXPECT_FALSE(validator("0123456789@example.com", true).has_value()) << "Digits are allowed in the local part";
	EXPECT_FALSE(validator("x@example.com", true).has_value()) << "One-letter local-part";
	EXPECT_FALSE(validator("!#$%&'*+-/=?{|}~@example.com", true).has_value()) << "Test allowed printable symbols in the local part";
	EXPECT_FALSE(validator(std::string(64, 'a') + "@example.com", true).has_value()) << "Local part is allowed up to 64 characters";

	// Test domain part
	EXPECT_FALSE(validator("admin@example", true).has_value()) << "Local domain name with no TLD";
	EXPECT_FALSE(validator("admin@example10.com", true).has_value()) << "Domain name with digits";
	EXPECT_FALSE(validator("admin@best-example.com", true).has_value()) << "Domain name with hyphen";
	EXPECT_FALSE(validator("admin@very.long.long.long.long.long.long.long.long.long.long.long.subdomains.example.com", true).has_value()) << "Multiple sub-domain parts";
	EXPECT_FALSE(validator("admin@" + std::string(63, 'a') + ".com", true).has_value()) << "Label in the domain part is allowed up to 63 characters";
	EXPECT_FALSE(validator("admin@long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long"
		".long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long"
		".long.long.long.long.domain.com", true).has_value()) << "Domain part is allowed up to 255 characters";
}

TEST(ValidatorEmail, TestInvalidEmails)
{
	// Arrange
	constexpr auto validator = Email();

	// Test local part
	EXPECT_TRUE(validator("", true).has_value()) << "Empty string";
	EXPECT_TRUE(validator(" ", true).has_value()) << "Space";
	EXPECT_TRUE(validator("@", true).has_value()) << "Only @ sign";
	EXPECT_TRUE(validator("abc.example.com", true).has_value()) << "No @ character";
	EXPECT_TRUE(validator("a@b@example.com", true).has_value()) << "Only one @ is allowed";
	EXPECT_TRUE(validator("first last@example.com", true).has_value()) << "Space in the local part is not allowed";
	EXPECT_TRUE(validator("first\tlast@example.com", true).has_value()) << "Tab in the local part is not allowed";
	EXPECT_TRUE(validator("\"john..doe\"@example.org)", true).has_value()) << "Quotes are allowed by RFC but not supported";
	EXPECT_TRUE(validator("john(doe)@example.org)", true).has_value()) << "Round brackets are not allowed";
	EXPECT_TRUE(validator("john,doe@example.org)", true).has_value()) << "Comma is not allowed";
	EXPECT_TRUE(validator("john:doe;@example.org)", true).has_value()) << "Colon and semicolon are not allowed";
	EXPECT_TRUE(validator("john<doe>@example.org)", true).has_value()) << "Less than and greater than signs are not allowed";
	EXPECT_TRUE(validator("john\x07F@example.org)", true).has_value()) << "Del code is not allowed";

	EXPECT_TRUE(validator(".name@example.com", true).has_value()) << "First dot in the local part is not allowed";
	EXPECT_TRUE(validator("name.@example.com", true).has_value()) << "Last dot in the local part is not allowed";
	EXPECT_TRUE(validator("first..last@example.com", true).has_value()) << "Consecutive dots in the local part are not allowed";

	EXPECT_TRUE(validator(std::string(65, 'a') + "@example.com", true).has_value()) << "Local - part is longer than 64 characters";

	// Test domain part
	EXPECT_TRUE(validator("john_doe@", true).has_value()) << "Empty domain part";
	EXPECT_TRUE(validator("john_doe@-example.com", true).has_value()) << "Hyphen cannot be first";
	EXPECT_TRUE(validator("john_doe@example.com-", true).has_value()) << "Hyphen cannot be last";
	EXPECT_TRUE(validator("john_doe@10example.com", true).has_value()) << "Domain part can't start with digits";
	EXPECT_TRUE(validator("john_doe@example com", true).has_value()) << "Domain part can't contain spaces";
	EXPECT_TRUE(validator("john_doe@example_com", true).has_value()) << "Domain part can't contain underscore";
	EXPECT_TRUE(validator("john_doe@example+com", true).has_value()) << "Domain part can't contain plus";
	EXPECT_TRUE(validator("john_doe@example/com", true).has_value()) << "Domain part can't contain slashes";
	EXPECT_TRUE(validator("john_doe@example*com", true).has_value()) << "Domain part can't contain asterisk";

	EXPECT_TRUE(validator("i.like.underscores@but_they_are_not_allowed_in_this_part", true).has_value()) << "Underscore is not allowed in domain part";
	EXPECT_TRUE(validator("john_doe@" + std::string(64, 'a') + ".com", true).has_value()) << "Too long label in the domain part";
	EXPECT_TRUE(validator("john_doe@too.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long"
		".long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long.long"
		".long.long.long.long.domain.csnet", true).has_value()) << "Too long domain part";
}
