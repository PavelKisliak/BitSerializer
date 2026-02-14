/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/validate.h"

// NOLINTBEGIN(bugprone-unchecked-optional-access)

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests for 'Required' validator
//-----------------------------------------------------------------------------
TEST(ValidatorRequired, ShouldNotReturnErrorIfValueIsLoaded)
{
	// Arrange
	const auto validator = Validate::Required();

	// Act
	const auto result = validator(10, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorRequired, ShouldReturnErrorIfValueIsNotLoaded)
{
	// Arrange
	const auto validator = Validate::Required();

	// Act
	const auto result = validator(0, false);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Value is required", result.value());
}

TEST(ValidatorRequired, ShouldReturnCustomErrorMessage)
{
	// Arrange
	const auto validator = Validate::Required("Custom error message");

	// Act
	const auto result = validator(0, false);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Custom error message", result.value());
}

//-----------------------------------------------------------------------------
// Tests for 'GreaterThan' validator
//-----------------------------------------------------------------------------
TEST(ValidatorGreaterThan, ShouldAlwaysPassIfValueIsNotLoaded)
{
	// Arrange
	const auto validator = Validate::GreaterThan(10);

	// Act
	const auto result = validator(0, false);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorGreaterThan, ShouldPassWhenValueIsGreater)
{
	// Arrange
	const auto validator = Validate::GreaterThan(1.0f);

	// Act
	const auto result = validator(1.1f, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorGreaterThan, ShouldReturnErrorWhenValueIsEqual)
{
	// Arrange
	const auto validator = Validate::GreaterThan(100);

	// Act
	const auto result = validator(100, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Value must be greater than 100", result.value());
}

TEST(ValidatorGreaterThan, ShouldReturnCustomErrorMessage)
{
	// Arrange
	const auto validator = Validate::GreaterThan(11, "Custom error message");

	// Act
	const auto result = validator(10, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Custom error message", result.value());
}

//-----------------------------------------------------------------------------
// Tests for 'GreaterThanOrEqual' validator
//-----------------------------------------------------------------------------
TEST(ValidatorGreaterThanOrEqual, ShouldAlwaysPassIfValueIsNotLoaded)
{
	// Arrange
	const auto validator = Validate::GreaterThanOrEqual(10);

	// Act
	const auto result = validator(0, false);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorGreaterThanOrEqual, ShouldPassWhenValueIsGreater)
{
	// Arrange
	const auto validator = Validate::GreaterThanOrEqual(1.0f);

	// Act
	const auto result = validator(1.1f, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorGreaterThanOrEqual, ShouldPassWhenValueIsEqual)
{
	// Arrange
	const auto validator = Validate::GreaterThanOrEqual(std::chrono::seconds(500));

	// Act
	const auto result = validator(std::chrono::seconds(500), true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorGreaterThanOrEqual, ShouldReturnErrorWhenValueIsLess)
{
	// Arrange
	const auto validator = Validate::GreaterThanOrEqual(100);

	// Act
	const auto result = validator(99, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Value must be greater than or equal to 100", result.value());
}

TEST(ValidatorGreaterThanOrEqual, ShouldReturnCustomErrorMessage)
{
	// Arrange
	const auto validator = Validate::GreaterThanOrEqual(11, "Custom error message");

	// Act
	const auto result = validator(10, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Custom error message", result.value());
}

//-----------------------------------------------------------------------------
// Tests for 'LessThan' validator
//-----------------------------------------------------------------------------
TEST(ValidatorLessThan, ShouldAlwaysPassIfValueIsNotLoaded)
{
	// Arrange
	const auto validator = Validate::LessThan(10);

	// Act
	const auto result = validator(0, false);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorLessThan, ShouldPassWhenValueIsLess)
{
	// Arrange
	const auto validator = Validate::LessThan(1.0f);

	// Act
	const auto result = validator(0.9f, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorLessThan, ShouldReturnErrorWhenValueIsEqual)
{
	// Arrange
	const auto validator = Validate::LessThan(100);

	// Act
	const auto result = validator(100, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Value must be less than 100", result.value());
}

TEST(ValidatorLessThan, ShouldReturnCustomErrorMessage)
{
	// Arrange
	const auto validator = Validate::LessThan(9, "Custom error message");

	// Act
	const auto result = validator(10, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Custom error message", result.value());
}

//-----------------------------------------------------------------------------
// Tests for 'LessThanOrEqual' validator
//-----------------------------------------------------------------------------
TEST(ValidatorLessThanOrEqual, ShouldAlwaysPassIfValueIsNotLoaded)
{
	// Arrange
	const auto validator = Validate::LessThanOrEqual(10);

	// Act
	const auto result = validator(0, false);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorLessThanOrEqual, ShouldPassWhenValueIsLess)
{
	// Arrange
	const auto validator = Validate::LessThanOrEqual(1.0f);

	// Act
	const auto result = validator(0.9f, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorLessThanOrEqual, ShouldPassWhenValueIsEqual)
{
	// Arrange
	const auto validator = Validate::LessThanOrEqual(std::chrono::nanoseconds(500));

	// Act
	const auto result = validator(std::chrono::nanoseconds(500), true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorLessThanOrEqual, ShouldReturnErrorWhenValueIsGreater)
{
	// Arrange
	const auto validator = Validate::LessThanOrEqual(100);

	// Act
	const auto result = validator(101, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Value must be less than or equal to 100", result.value());
}

TEST(ValidatorLessThanOrEqual, ShouldReturnCustomErrorMessage)
{
	// Arrange
	const auto validator = Validate::LessThanOrEqual(9, "Custom error message");

	// Act
	const auto result = validator(10, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Custom error message", result.value());
}

//-----------------------------------------------------------------------------
// Tests for 'Range' validator
//-----------------------------------------------------------------------------
TEST(ValidatorRange, ShouldAlwaysPassIfValueIsNotLoaded)
{
	// Arrange
	const auto validator = Validate::Range(10, 20);

	// Act
	const auto result = validator(0, false);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorRange, ShouldNotReturnErrorIfValueIsInRangeLoaded)
{
	// Arrange
	const auto validator = Validate::Range(1, 1);

	// Act
	const auto result = validator(1, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorRange, ShouldReturnErrorIfValueIsLessThanMin)
{
	// Arrange
	const auto validator = Validate::Range(10, 20);

	// Act
	const auto result = validator(9, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_FALSE(result->empty());
}

TEST(ValidatorRange, ShouldReturnErrorIfValueIsGreaterThanMax)
{
	// Arrange
	const auto validator = Validate::Range(10, 20);

	// Act
	const auto result = validator(21, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_FALSE(result->empty());
}

TEST(ValidatorRange, ShouldReturnCustomErrorMessage)
{
	// Arrange
	const auto validator = Validate::Range(1, 2, "Custom error message");

	// Act
	const auto result = validator(3, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Custom error message", result.value());
}

//-----------------------------------------------------------------------------
// Tests for 'MultipleOf' validator
//-----------------------------------------------------------------------------
TEST(ValidatorMultipleOf, ShouldAlwaysPassIfValueIsNotLoaded)
{
	// Arrange
	const auto validator = Validate::MultipleOf(10);

	// Act / Assert
	EXPECT_FALSE(validator(0, false));
}

TEST(ValidatorMultipleOf, ShouldValidateIntegerValues)
{
	// Arrange
	const auto validator = Validate::MultipleOf(100ULL);

	// Act / Assert
	EXPECT_FALSE(validator(100ULL, true).has_value());
	EXPECT_FALSE(validator(1000ULL, true).has_value());
	EXPECT_FALSE(validator(18446744073709551600ULL, true).has_value());

	EXPECT_TRUE(validator(1ULL, true).has_value());
	EXPECT_TRUE(validator(99ULL, true).has_value());
	EXPECT_TRUE(validator(101ULL, true).has_value());
}

TEST(ValidatorMultipleOf, ShouldValidateFloatValues)
{
	// Arrange
	const auto validator = Validate::MultipleOf(0.1f);

	// Act / Assert
	EXPECT_FALSE(validator(0.3f, true).has_value());
	EXPECT_FALSE(validator(0.6f, true).has_value());

	EXPECT_TRUE(validator(0.35f, true).has_value());
	EXPECT_TRUE(validator(0.67f, true).has_value());
}

TEST(ValidatorMultipleOf, ShouldValidateDoubleValues)
{
	// Arrange
	const auto validator = Validate::MultipleOf(0.01);

	// Act / Assert
	EXPECT_FALSE(validator(1.00, true).has_value());
	EXPECT_FALSE(validator(1.99, true).has_value());
	EXPECT_FALSE(validator(0.01, true).has_value());
	EXPECT_FALSE(validator(1234.56, true).has_value());

	EXPECT_TRUE(validator(1.995, true).has_value());
	EXPECT_TRUE(validator(0.001, true).has_value());
}

TEST(ValidatorMultipleOf, ShouldValidateWithNegativeDivisor)
{
	auto validator = Validate::MultipleOf(-5);

	// Act / Assert
	EXPECT_FALSE(validator(10, true).has_value());
	EXPECT_FALSE(validator(-10, true).has_value());
	EXPECT_FALSE(validator(0, true).has_value());

	EXPECT_TRUE(validator(7, true).has_value());
	EXPECT_TRUE(validator(-7, true).has_value());
}

TEST(ValidatorMultipleOf, ShouldThrowsExceptionWhenZeroDivisor)
{
	EXPECT_THROW(Validate::MultipleOf(0), std::invalid_argument);
	EXPECT_THROW(Validate::MultipleOf(0U), std::invalid_argument);
}

TEST(ValidatorMultipleOf, ShouldThrowsExceptionWhenSmallDivisor)
{
	EXPECT_THROW(Validate::MultipleOf(std::numeric_limits<float>::epsilon() / 2.0f), std::invalid_argument);
	EXPECT_THROW(Validate::MultipleOf(std::numeric_limits<double>::epsilon() / 2.0), std::invalid_argument);
}

TEST(ValidatorMultipleOf, ShouldReturnCustomErrorMessage)
{
	// Arrange
	const auto validator = Validate::MultipleOf(5, "Value must be multiple of 5 cents");

	// Act
	auto result2 = validator(7, true);

	// Assert
	ASSERT_TRUE(result2.has_value());
	EXPECT_EQ(result2.value(), "Value must be multiple of 5 cents");
}

//-----------------------------------------------------------------------------
// Tests for 'MinSize' validator
//-----------------------------------------------------------------------------
TEST(ValidatorMinSize, ShouldAlwaysPassIfValueIsNotLoaded)
{
	// Arrange
	const auto validator = Validate::MinSize(10);
	const std::string testValue(9, '#');

	// Act
	const auto result = validator(testValue, false);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorMinSize, ShouldNotReturnErrorIfSizeIsEqual)
{
	// Arrange
	const auto validator = Validate::MinSize(10);
	const std::string testValue(10, '#');

	// Act
	const auto result = validator(testValue, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorMinSize, ShouldNotReturnErrorIfSizeIsGreater)
{
	// Arrange
	const auto validator = Validate::MinSize(10);
	const std::string testValue(11, '#');

	// Act
	const auto result = validator(testValue, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorMinSize, ShouldReturnErrorIfSizeIsLess)
{
	// Arrange
	const auto validator = Validate::MinSize(10);
	const std::string testValue(9, '#');

	// Act
	const auto result = validator(testValue, true);

	// Assert
	EXPECT_TRUE(result.has_value());
}

TEST(ValidatorMinSize, ShouldReturnCustomErrorMessage)
{
	// Arrange
	const auto validator = Validate::MinSize(10, "Custom error message");
	const std::string testValue(9, '#');

	// Act
	const auto result = validator(testValue, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Custom error message", result.value());
}

//-----------------------------------------------------------------------------
// Tests for 'MaxSize' validator
//-----------------------------------------------------------------------------
TEST(ValidatorMaxSize, ShouldAlwaysPassIfValueIsNotLoaded)
{
	// Arrange
	const auto validator = Validate::MaxSize(10);
	const std::string testValue(11, '#');

	// Act
	const auto result = validator(testValue, false);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorMaxSize, ShouldNotReturnErrorIfSizeIsEqual)
{
	// Arrange
	const auto validator = Validate::MaxSize(10);
	const std::string testValue(10, '#');

	// Act
	const auto result = validator(testValue, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorMaxSize, ShouldNotReturnErrorIfSizeIsLess)
{
	// Arrange
	const auto validator = Validate::MaxSize(10);
	const std::string testValue(9, '#');

	// Act
	const auto result = validator(testValue, true);

	// Assert
	EXPECT_FALSE(result.has_value());
}

TEST(ValidatorMaxSize, ShouldReturnErrorIfSizeIsGreater)
{
	// Arrange
	const auto validator = Validate::MaxSize(10);
	const std::string testValue(11, '#');

	// Act
	const auto result = validator(testValue, true);

	// Assert
	EXPECT_TRUE(result.has_value());
}

TEST(ValidatorMaxSize, ShouldReturnCustomErrorMessage)
{
	// Arrange
	const auto validator = Validate::MaxSize(10, "Custom error message");
	const std::string testValue(11, '#');

	// Act
	const auto result = validator(testValue, true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Custom error message", result.value());
}

//-----------------------------------------------------------------------------
// Tests for 'Email' validator
//-----------------------------------------------------------------------------
TEST(ValidatorEmail, TestDifferentStringTypes)
{
	// Arrange
	const auto validator = Validate::Email();

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
	const auto validator = Validate::Email();

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
	const auto validator = Validate::Email();

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

TEST(ValidatorEmail, ShouldReturnCustomErrorMessage)
{
	// Arrange
	const auto validator = Validate::Email("Custom error message");

	// Act
	const auto result = validator("abc.example.com", true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Custom error message", result.value());
}

//-----------------------------------------------------------------------------
// Tests for 'PhoneNumber' validator
//-----------------------------------------------------------------------------
TEST(ValidatorPhoneNumber, TestDifferentStringTypes)
{
	// Arrange
	const auto validator = Validate::PhoneNumber();

	// Act / Assert
	EXPECT_FALSE(validator("+123 (555) 555-55-55", true).has_value());
	EXPECT_FALSE(validator(std::string("+123 (555) 555-55-55"), true).has_value());
	EXPECT_FALSE(validator(std::u16string(u"+123 (555) 555-55-55"), true).has_value());
	EXPECT_FALSE(validator(std::u32string(U"+123 (555) 555-55-55"), true).has_value());
	EXPECT_FALSE(validator(std::wstring(L"+123 (555) 555-55-55"), true).has_value());
}

TEST(ValidatorPhoneNumber, TestValidPhones)
{
	const auto validator = Validate::PhoneNumber();

	EXPECT_FALSE(validator("+1 (555) 555-55-55", true).has_value());

	EXPECT_FALSE(validator("+44 20 7123 1234", true).has_value());
	EXPECT_FALSE(validator("+91-22-27782183", true).has_value());
	EXPECT_FALSE(validator(" +91 - 22 - 27782183 ", true).has_value());
}

TEST(ValidatorPhoneNumber, TestValidPhonesWithoutPlus)
{
	const auto validator = Validate::PhoneNumber(7, 15, false);

	EXPECT_FALSE(validator("(555) 555-55-55", true).has_value());
	EXPECT_FALSE(validator("44 20 7123 1234", true).has_value());
	EXPECT_FALSE(validator(" (55) 555-5555 ", true).has_value());
}

TEST(ValidatorPhoneNumber, TestPhonesWithInvalidNumberOfDigits)
{
	const auto validator = Validate::PhoneNumber(6, 12);

	EXPECT_TRUE(validator("+12345", true).has_value()) << "Should contain at least 6 digits";
	EXPECT_TRUE(validator("+1234567890123", true).has_value()) << "Should contain maximum 12 digits";
}

TEST(ValidatorPhoneNumber, TestPhonesWithInvalidParenthesis)
{
	const auto validator = Validate::PhoneNumber(6, 12);

	EXPECT_TRUE(validator("+1 ((555)) 555-55-55", true).has_value()) << "Nested parenthesis are not allowed";
	EXPECT_TRUE(validator("+1 (555 555-55-55", true).has_value()) << "Missing closing parenthesis";
	EXPECT_TRUE(validator("+1 (555) )555-55-55", true).has_value()) << "Invalid closing parenthesis";
	EXPECT_TRUE(validator("+1 () 555-55-55", true).has_value()) << "Invalid closing parenthesis";
	EXPECT_TRUE(validator("+1 555 555-55-55 )", true).has_value()) << "Invalid closing parenthesis";
	EXPECT_TRUE(validator("+1 555 555-55-55 ()", true).has_value()) << "Invalid parenthesis";
}

TEST(ValidatorPhoneNumber, TestPhonesWithInvalidDashes)
{
	const auto validator = Validate::PhoneNumber(6, 12);

	EXPECT_TRUE(validator("-1 (555) 555-5555", true).has_value()) << "The leading dash is not allowed";
	EXPECT_TRUE(validator("-(555) 555-5555", true).has_value()) << "The leading dash is not allowed";
	EXPECT_TRUE(validator("+1 (555) 555--5555", true).has_value()) << "The sequence of dashes is not allowed";
	EXPECT_TRUE(validator("+1 (555) 555-5555-", true).has_value()) << "The dash at the end is not allowed";
	EXPECT_TRUE(validator("+1 (555) -555-55-55", true).has_value()) << "Invalid dash after parenthesis";
	EXPECT_TRUE(validator("+1 (-555) 555-55-55", true).has_value()) << "Invalid dash in the parenthesis";
	EXPECT_TRUE(validator("+1 (555-) 555-55-55", true).has_value()) << "Invalid dash in the parenthesis";
}

TEST(ValidatorPhoneNumber, TestPhonesWithInvalidCharacters)
{
	const auto validator = Validate::PhoneNumber(6, 12);

	EXPECT_TRUE(validator("*1 (555) 555-55-55", true).has_value());
	EXPECT_TRUE(validator("1 (555) 555-55-55$", true).has_value());
	EXPECT_TRUE(validator("1 (555) 555-55=55", true).has_value());
}

//-----------------------------------------------------------------------------
// Tests for 'Uuid' validator
//-----------------------------------------------------------------------------
TEST(ValidatorUuid, TestValidUuids)
{
	// Arrange
	auto validator = Validate::Uuid();

	// Act / Assert
	EXPECT_FALSE(validator(std::u16string(u"550e8400-e29b-41d4-a716-446655440000"), true).has_value());	// Lowercase variant
	EXPECT_FALSE(validator(std::u32string(U"550E8400-E29B-41D4-A716-446655440000"), true).has_value());	// Uppercase variant

	EXPECT_FALSE(validator(std::string("550e8400-e29b-11d4-a716-446655440000"), true).has_value());		// UUID v1 (time-based)
	EXPECT_FALSE(validator(std::string("550e8400-e29b-21d4-a716-446655440000"), true).has_value());		// UUID v2 (DCE security)
	EXPECT_FALSE(validator(std::string("550e8400-e29b-31d4-a716-446655440000"), true).has_value());		// UUID v3 (MD5 hash)
	EXPECT_FALSE(validator(std::string("550e8400-e29b-41d4-a716-446655440000"), true).has_value());		// UUID v4 (random)
	EXPECT_FALSE(validator(std::string("550e8400-e29b-51d4-a716-446655440000"), true).has_value());		// UUID v5 (SHA-1 hash)
	EXPECT_FALSE(validator(std::string("1ec9414c-22a8-6f5f-b432-2f8e3a5f9a6d"), true).has_value());		// UUID v6 (reordered time-based)
	EXPECT_FALSE(validator(std::string("01889360-5c47-7b7c-a3d1-9f8b6c5d4e3a"), true).has_value());		// UUID v7 (Unix Epoch time + random)
	EXPECT_FALSE(validator(std::string("550e8400-e29b-81d4-a716-446655440000"), true).has_value());		// UUID v8 (custom)
}

TEST(ValidatorUuid, TestInvalidUuids)
{
	// Arrange
	auto validator = Validate::Uuid();

	// Empty string
	EXPECT_TRUE(validator("", true).has_value());

	// Wrong length
	EXPECT_TRUE(validator("550e8400-e29b-41d4-a716-44665544000", true).has_value());							// 35 chars
	EXPECT_TRUE(validator("550e8400-e29b-41d4-a716-4466554400000", true).has_value());							// 37 chars

	// Missing hyphens
	EXPECT_TRUE(validator(std::u16string(u"550e8400e29b-41d4-a716-446655440000"), true).has_value());
	EXPECT_TRUE(validator(std::u32string(U"550e8400-e29b41d4-a716-446655440000"), true).has_value());

	// Wrong hyphen positions
	EXPECT_TRUE(validator(std::string_view("550e840-e29b-41d4-a716-446655440000"), true).has_value());	// hyphen at pos 7
	EXPECT_TRUE(validator(std::string_view("550e8400e-29b-41d4-a716-446655440000"), true).has_value());	// hyphen at pos 9

	// Non-hex characters
	EXPECT_TRUE(validator(std::string_view("550e8400-e29b-41d4-a71g-446655440000"), true).has_value());	// 'g' is not hex
	EXPECT_TRUE(validator(std::string_view("550e8400-e29b-41d4-a716-44665544000-"), true).has_value());	// trailing hyphen

	// Whitespace
	EXPECT_TRUE(validator(std::string_view(" 550e8400-e29b-41d4-a716-446655440000"), true).has_value());
	EXPECT_TRUE(validator(std::string_view("550e8400-e29b-41d4-a716-446655440000 "), true).has_value());

	// Too long
	const auto result = validator(std::string_view("12345678-1234-1234-1234-1234567890123"), true);
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("Invalid UUID format", result.value());
}

TEST(ValidatorUuid, ShouldReturnCustomErrorMessage)
{
	// Arrange
	const auto validator = Validate::Uuid("ID must be a valid UUID v4");

	// Act
	const auto result = validator(std::string_view("550e8400-e29b-"), true);

	// Assert
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ("ID must be a valid UUID v4", result.value());
}

// NOLINTEND(bugprone-unchecked-optional-access)
