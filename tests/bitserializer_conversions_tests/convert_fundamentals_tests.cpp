/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test conversion of any fundamental type to any other fundamental type.
//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, ToTheSameType)
{
	EXPECT_EQ(true, Convert::To<bool>(true));
	EXPECT_EQ(false, Convert::To<bool>(false));

	EXPECT_EQ(std::numeric_limits<int>::min(), Convert::To<int>(std::numeric_limits<int>::min()));
	EXPECT_EQ(std::numeric_limits<int>::max(), Convert::To<int>(std::numeric_limits<int>::max()));

	EXPECT_EQ(std::numeric_limits<uint64_t>::min(), Convert::To<uint64_t>(std::numeric_limits<uint64_t>::min()));
	EXPECT_EQ(std::numeric_limits<uint64_t>::max(), Convert::To<uint64_t>(std::numeric_limits<uint64_t>::max()));

	EXPECT_EQ(3.14f, Convert::To<float>(3.14f));
	EXPECT_EQ(3.141592654, Convert::To<double>(3.141592654));
}

TEST(ConvertFundamentals, BoolToUnsigned)
{
	EXPECT_EQ(0, Convert::To<bool>(false));
	EXPECT_EQ(1, Convert::To<bool>(true));
}

TEST(ConvertFundamentals, BoolFromIntegers)
{
	EXPECT_EQ(false, Convert::To<bool>(0));
	EXPECT_EQ(true, Convert::To<bool>(1u));
}

TEST(ConvertFundamentals, BoolFromTooBigIntegerThrowException)
{
	EXPECT_THROW(Convert::To<bool>(2), std::out_of_range);
	EXPECT_THROW(Convert::To<bool>(-1), std::out_of_range);
}

TEST(ConvertFundamentals, IntMaxToUnsigned)
{
	EXPECT_EQ(127, Convert::To<uint8_t>(std::numeric_limits<int8_t>::max()));
	EXPECT_EQ(32767, Convert::To<uint16_t>(std::numeric_limits<int16_t>::max()));
	EXPECT_EQ(2147483647, Convert::To<uint32_t>(std::numeric_limits<int32_t>::max()));
	EXPECT_EQ(9223372036854775807, Convert::To<uint64_t>(std::numeric_limits<int64_t>::max()));
}

TEST(ConvertFundamentals, IntToIntWithLessSize)
{
	EXPECT_EQ(-128, Convert::To<int8_t>(static_cast<int16_t>(-128)));
	EXPECT_EQ(-32768, Convert::To<int16_t>(static_cast<int32_t>(-32768)));
	EXPECT_EQ(-2147483648, Convert::To<int32_t>(static_cast<int64_t>(-2147483648)));
}

TEST(ConvertFundamentals, UnsignedToUnsignedWithLessSize)
{
	EXPECT_EQ(255, Convert::To<uint8_t>(static_cast<uint16_t>(255)));
	EXPECT_EQ(65535, Convert::To<uint16_t>(static_cast<uint32_t>(65535)));
	EXPECT_EQ(4294967295, Convert::To<uint32_t>(static_cast<uint64_t>(4294967295)));
}

TEST(ConvertFundamentals, IntToFloatingTypes)
{
	EXPECT_EQ(12345.f, Convert::To<float>(12345));
	EXPECT_EQ(12345.0, Convert::To<double>(12345));
}

TEST(ConvertFundamentals, IntFromFloatingTypesThrowException)
{
	EXPECT_THROW( Convert::To<int>(12345.f), std::invalid_argument);
	EXPECT_THROW(Convert::To<int>(12345.0), std::invalid_argument);
}

TEST(ConvertFundamentals, IntFromTooBigIntThrowException)
{
	EXPECT_THROW(Convert::To<int8_t>(128), std::out_of_range);
	EXPECT_THROW(Convert::To<int8_t>(-129), std::out_of_range);

	EXPECT_THROW(Convert::To<int16_t>(32768), std::out_of_range);
	EXPECT_THROW(Convert::To<int16_t>(-32769), std::out_of_range);

	EXPECT_THROW(Convert::To<int32_t>(2147483648ll), std::out_of_range);
	EXPECT_THROW(Convert::To<int32_t>(-2147483649ll), std::out_of_range);
}

TEST(ConvertFundamentals, UnsignedFromNegativeIntThrowException)
{
	EXPECT_THROW(Convert::To<uint8_t>(-1), std::out_of_range);
	EXPECT_THROW(Convert::To<uint16_t>(std::numeric_limits<int16_t>::min()), std::out_of_range);
}

TEST(ConvertFundamentals, UnsignedFromTooBigUnsignedThrowException)
{
	EXPECT_THROW(Convert::To<uint8_t>(256), std::out_of_range);
	EXPECT_THROW(Convert::To<uint16_t>(65536), std::out_of_range);
	EXPECT_THROW(Convert::To<uint32_t>(4294967296ll), std::out_of_range);
}

TEST(ConvertFundamentals, FloatFromDouble)
{
	EXPECT_EQ(0.0f, Convert::To<float>(0.0));
	EXPECT_EQ(3.14f, Convert::To<float>(3.14));

	constexpr float lowestFloat = std::numeric_limits<float>::lowest();
	EXPECT_EQ(lowestFloat, Convert::To<float>(static_cast<double>(lowestFloat)));
}

TEST(ConvertFundamentals, FloatFromTooBigDoubleThrowException)
{
	constexpr auto sourceNumber = static_cast<double>(std::numeric_limits<float>::max()) * 1.00001;
	EXPECT_THROW(Convert::To<float>(sourceNumber), std::out_of_range);
}

TEST(ConvertFundamentals, FloatFromTooBigNegativeDoubleThrowException)
{
	constexpr auto sourceNumber = static_cast<double>(std::numeric_limits<float>::lowest()) * 1.00001;
	EXPECT_THROW(Convert::To<float>(sourceNumber), std::out_of_range);
}

TEST(ConvertFundamentals, DoubleFromFloatMax)
{
	EXPECT_EQ(static_cast<double>(std::numeric_limits<float>::min()), Convert::To<double>(std::numeric_limits<float>::min()));
	EXPECT_EQ(static_cast<double>(std::numeric_limits<float>::max()), Convert::To<double>(std::numeric_limits<float>::max()));
}

//-----------------------------------------------------------------------------
// Test conversion for fundamental types to/from strings
//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, BoolFromStringWithDigit) {
	EXPECT_EQ(false, Convert::To<bool>("  0  "));
	EXPECT_EQ(true, Convert::To<bool>(u"  1  "));
	EXPECT_EQ(false, Convert::To<bool>(U"0"));
}

TEST(ConvertFundamentals, BoolFromStringWithNegativeDigitShouldThrowException) {
	EXPECT_THROW(Convert::To<bool>("-1"), std::invalid_argument);
}

TEST(ConvertFundamentals, BoolFromStringWithBigNumberShouldThrowException) {
	EXPECT_THROW(Convert::To<bool>("2"), std::out_of_range);
	EXPECT_THROW(Convert::To<bool>("555"), std::out_of_range);
}

TEST(ConvertFundamentals, BoolFromStringWithTrueFalse) {
	EXPECT_EQ(true, Convert::To<bool>("  True"));
	EXPECT_EQ(false, Convert::To<bool>("  False"));

	EXPECT_EQ(true, Convert::To<bool>(u"tRuE\t"));
	EXPECT_EQ(true, Convert::To<bool>(u"TrUe,"));
	
	EXPECT_EQ(false, Convert::To<bool>(U"fAlSe\n"));
	EXPECT_EQ(false, Convert::To<bool>(U"FaLsE)"));
}

TEST(ConvertFundamentals, BoolFromStringWithWrongTextShouldThrowException) {
	EXPECT_THROW(Convert::To<bool>("test"), std::invalid_argument);
	EXPECT_THROW(Convert::To<bool>(",true"), std::invalid_argument);
}

TEST(ConvertFundamentals, BoolToString) {
	EXPECT_EQ("false", Convert::ToString(false));
	EXPECT_EQ(u"true", Convert::To<std::u16string>(true));
	EXPECT_EQ(U"false", Convert::To<std::u32string>(false));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, Int8FromString) {
	EXPECT_EQ(0, Convert::To<int8_t>("  -0  "));
	EXPECT_EQ(-128, Convert::To<int8_t>(u"  -128  "));
	EXPECT_EQ(127, Convert::To<int8_t>(U"  127  "));
}

TEST(ConvertFundamentals, Int8FromStringWithBigNumberShouldThrowException) {
	EXPECT_THROW(Convert::To<int8_t>("-129"), std::out_of_range);
	EXPECT_THROW(Convert::To<int8_t>("128"), std::out_of_range);
}

TEST(ConvertFundamentals, Int8ToString) {
	EXPECT_EQ("0", Convert::ToString(int8_t(0)));
	EXPECT_EQ(u"-128", Convert::To<std::u16string>(int8_t(-128)));
	EXPECT_EQ(U"127", Convert::To<std::u32string>(int8_t(127)));
}

TEST(ConvertFundamentals, UInt8FromString) {
	EXPECT_EQ(0, Convert::To<uint8_t>("  0  "));
	EXPECT_EQ(128, Convert::To<uint8_t>(u"  128  "));
	EXPECT_EQ(255, Convert::To<uint8_t>(U"  255  "));
}

TEST(ConvertFundamentals, UInt8FromStringWithBigNumberShouldThrowException) {
	EXPECT_THROW(Convert::To<uint8_t>("256"), std::out_of_range);
}

TEST(ConvertFundamentals, UInt8ToString) {
	EXPECT_EQ("0", Convert::ToString(uint8_t(0)));
	EXPECT_EQ(u"100", Convert::To<std::u16string>(uint8_t(100)));
	EXPECT_EQ(U"255", Convert::To<std::u32string>(uint8_t(255)));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, Int16FromString) {
	EXPECT_EQ(0, Convert::To<int16_t>("  -0  "));
	EXPECT_EQ(-32767, Convert::To<int16_t>(u"  -32767  "));
	EXPECT_EQ(32767, Convert::To<int16_t>(U"  32767  "));
}

TEST(ConvertFundamentals, Int16FromStringWithBigNumberShouldThrowException) {
	EXPECT_THROW(Convert::To<int16_t>("-32769"), std::out_of_range);
	EXPECT_THROW(Convert::To<int16_t>("32768"), std::out_of_range);
}

TEST(ConvertFundamentals, Int16ToString) {
	EXPECT_EQ("0", Convert::ToString(int16_t(0)));
	EXPECT_EQ(u"-32768", Convert::To<std::u16string>(int16_t(-32768)));
	EXPECT_EQ(U"32767", Convert::To<std::u32string>(int16_t(32767)));
}

TEST(ConvertFundamentals, UInt16FromString) {
	EXPECT_EQ(0, Convert::To<uint16_t>("  0  "));
	EXPECT_EQ(32768, Convert::To<uint16_t>(u"  32768  "));
	EXPECT_EQ(65535, Convert::To<uint16_t>(U"  65535  "));
}

TEST(ConvertFundamentals, UInt16FromStringWithBigNumberShouldThrowException) {
	EXPECT_THROW(Convert::To<uint16_t>("65536"), std::out_of_range);
}

TEST(ConvertFundamentals, UInt16ToString) {
	EXPECT_EQ("0", Convert::ToString(uint16_t(0)));
	EXPECT_EQ(u"32768", Convert::To<std::u16string>(uint16_t(32768)));
	EXPECT_EQ(U"65535", Convert::To<std::u32string>(uint16_t(65535)));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, Int32FromString) {
	EXPECT_EQ(0, Convert::To<int32_t>("  -0  "));
	EXPECT_EQ(-2147483648, Convert::To<int32_t>(u"  -2147483648  "));
	EXPECT_EQ(2147483647, Convert::To<int32_t>(L"  2147483647  "));
	EXPECT_EQ(2147483647, Convert::To<int32_t>(U"  2147483647  "));
}

TEST(ConvertFundamentals, Int32FromStringWithBigNumberShouldThrowException) {
	EXPECT_THROW(Convert::To<int32_t>("-2147483649"), std::out_of_range);
	EXPECT_THROW(Convert::To<int32_t>("2147483648"), std::out_of_range);
}

TEST(ConvertFundamentals, Int32ToString) {
	EXPECT_EQ("0", Convert::ToString(0));
	EXPECT_EQ(u"-2147483648", Convert::To<std::u16string>(int32_t(-2147483648)));
	EXPECT_EQ(U"2147483647", Convert::To<std::u32string>(int32_t(2147483647)));
}

TEST(ConvertFundamentals, UInt32FromString) {
	EXPECT_EQ(0, Convert::To<uint32_t>("  0  "));
	EXPECT_EQ(4294967295l, Convert::To<uint32_t>(u"  4294967295  "));
	EXPECT_EQ(4294967295l, Convert::To<uint32_t>(L"  4294967295  "));
	EXPECT_EQ(4294967295l, Convert::To<uint32_t>(U"  4294967295  "));
}

TEST(ConvertFundamentals, UInt32FromStringWithBigNumberShouldThrowException) {
	EXPECT_THROW(Convert::To<uint32_t>("4294967296"), std::out_of_range);
}

TEST(ConvertFundamentals, UInt32ToString) {
	EXPECT_EQ("0", Convert::ToString(0));
	EXPECT_EQ(u"2147483648", Convert::To<std::u16string>(2147483648));
	EXPECT_EQ(U"4294967295", Convert::To<std::u32string>(4294967295l));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, Int64FromString) {
	EXPECT_EQ(0, Convert::To<int64_t>("  000  "));
	EXPECT_EQ(std::numeric_limits<int64_t>::min(), Convert::To<int64_t>(u"  -9223372036854775808  "));
	EXPECT_EQ(std::numeric_limits<int64_t>::max(), Convert::To<int64_t>(L"  9223372036854775807  "));
	EXPECT_EQ(std::numeric_limits<int64_t>::max(), Convert::To<int64_t>(U"  9223372036854775807  "));
}

TEST(ConvertFundamentals, Int64FromStringWithBigNumberShouldThrowException) {
	EXPECT_THROW(Convert::To<int64_t>("-9223372036854775809"), std::out_of_range);
	EXPECT_THROW(Convert::To<int64_t>("9223372036854775808"), std::out_of_range);
}

TEST(ConvertFundamentals, Int64FromEmptyStringShouldThrowException) {
	EXPECT_THROW(Convert::To<int64_t>(""), std::invalid_argument);
}

TEST(ConvertFundamentals, Int64FromStringWithWrongTextShouldThrowException) {
	EXPECT_THROW(Convert::To<int64_t>("test"), std::invalid_argument);
	EXPECT_THROW(Convert::To<int64_t>(u"`150"), std::invalid_argument);
	EXPECT_THROW(Convert::To<int64_t>(U"x45.4"), std::invalid_argument);
}

TEST(ConvertFundamentals, Int64FromStringWithFloatShouldThrowException) {
	EXPECT_THROW(Convert::To<int64_t>("3.1"), std::invalid_argument);
	EXPECT_THROW(Convert::To<int64_t>(u"9.9"), std::invalid_argument);
	EXPECT_THROW(Convert::To<int64_t>(U"-1.0"), std::invalid_argument);
}

TEST(ConvertFundamentals, Int64ToString) {
	EXPECT_EQ("0", Convert::ToString(0));
	EXPECT_EQ(u"-9223372036854775808", Convert::To<std::u16string>(std::numeric_limits<int64_t>::min()));
	EXPECT_EQ(U"9223372036854775807", Convert::To<std::u32string>(std::numeric_limits<int64_t>::max()));
}

TEST(ConvertFundamentals, UInt64FromString) {
	EXPECT_EQ(0, Convert::To<uint64_t>("  000  "));
	EXPECT_EQ(std::numeric_limits<uint64_t>::max(), Convert::To<uint64_t>(u"  18446744073709551615  "));
	EXPECT_EQ(std::numeric_limits<uint64_t>::max(), Convert::To<uint64_t>(L"  18446744073709551615  "));
	EXPECT_EQ(std::numeric_limits<uint64_t>::max(), Convert::To<uint64_t>(U"  18446744073709551615  "));
}

TEST(ConvertFundamentals, UInt64FromStringWithBigNumberShouldThrowException) {
	EXPECT_THROW(Convert::To<uint64_t>("18446744073709551616"), std::out_of_range);
}

TEST(ConvertFundamentals, UInt64FromEmptyStringShouldThrowException) {
	EXPECT_THROW(Convert::To<uint64_t>(""), std::invalid_argument);
}

TEST(ConvertFundamentals, UInt64FromStringWithWrongTextShouldThrowException) {
	EXPECT_THROW(Convert::To<uint64_t>("test"), std::invalid_argument);
	EXPECT_THROW(Convert::To<uint64_t>(u"`150"), std::invalid_argument);
	EXPECT_THROW(Convert::To<uint64_t>(U"x45.4"), std::invalid_argument);
}

TEST(ConvertFundamentals, UInt64FromStringWithFloatShouldThrowException) {
	EXPECT_THROW(Convert::To<uint64_t>("3.1"), std::invalid_argument);
	EXPECT_THROW(Convert::To<uint64_t>(u"9.9"), std::invalid_argument);
	EXPECT_THROW(Convert::To<uint64_t>(U"1.0"), std::invalid_argument);
}

TEST(ConvertFundamentals, UInt64ToString) {
	EXPECT_EQ("0", Convert::ToString(0ull));
	EXPECT_EQ(u"9223372036854775808", Convert::To<std::u16string>(9223372036854775808ull));
	EXPECT_EQ(U"18446744073709551615", Convert::To<std::u32string>(std::numeric_limits<uint64_t>::max()));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, FloatFromString) {
	EXPECT_EQ(0.f, Convert::To<float>("  0  "));
	EXPECT_EQ(123.123f, Convert::To<float>(u"  123.123  "));
	EXPECT_EQ(-123.123f, Convert::To<float>(U"  -123.123  "));
}

TEST(ConvertFundamentals, FloatFromEmptyStringShouldThrowException) {
	EXPECT_THROW(Convert::To<float>(""), std::invalid_argument);
}

TEST(ConvertFundamentals, FloatFromStringWithWrongTextShouldThrowException) {
	EXPECT_THROW(Convert::To<float>("test"), std::invalid_argument);
	EXPECT_THROW(Convert::To<float>(u"#150"), std::invalid_argument);
	EXPECT_THROW(Convert::To<float>(U"x45.4"), std::invalid_argument);
}

TEST(ConvertFundamentals, FloatToString) {
	EXPECT_EQ("0", Convert::ToString(0.f));
	EXPECT_EQ(u"-100.255", Convert::To<std::u16string>(-100.255f));
	EXPECT_EQ("23613", Convert::ToString(23613.f));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, DoubleFromString) {
	EXPECT_EQ(-0.0, Convert::To<double>("  -0  "));
	EXPECT_EQ(1234567.1234567, Convert::To<double>(u"  1234567.1234567  "));
	EXPECT_EQ(-1234567.1234567, Convert::To<double>(U"  -1234567.1234567  "));
}

TEST(ConvertFundamentals, DoubleFromEmptyStringShouldThrowException) {
	EXPECT_THROW(Convert::To<double>(""), std::invalid_argument);
}

TEST(ConvertFundamentals, DoubleFromStringWithWrongTextShouldThrowException) {
	EXPECT_THROW(Convert::To<double>("test"), std::invalid_argument);
	EXPECT_THROW(Convert::To<double>(u"#150"), std::invalid_argument);
	EXPECT_THROW(Convert::To<double>(U"x45.4"), std::invalid_argument);
}

TEST(ConvertFundamentals, DoubleToString) {
	EXPECT_EQ("0", Convert::ToString(0.0));
	EXPECT_EQ(u"-1234567.1234567", Convert::To<std::u16string>(-1234567.1234567));
	EXPECT_EQ(U"1234567.1234567", Convert::To<std::u32string>(1234567.1234567));
}

//-----------------------------------------------------------------------------

TEST(ConvertFundamentals, LongDoubleFromString) {
	EXPECT_EQ(0.0L, Convert::To<long double>("  0  "));
	EXPECT_EQ(3.14159265358979L, Convert::To<long double>(u"  3.14159265358979  "));
	EXPECT_EQ(-3.14159265358979L, Convert::To<long double>(U"  -3.14159265358979  "));
}

TEST(ConvertFundamentals, LongDoubleFromEmptyStringShouldThrowException) {
	EXPECT_THROW(Convert::To<long double>(""), std::invalid_argument);
}

TEST(ConvertFundamentals, LongDoubleFromStringWithWrongTextShouldThrowException) {
	EXPECT_THROW(Convert::To<long double>("test"), std::invalid_argument);
	EXPECT_THROW(Convert::To<long double>(u"#150"), std::invalid_argument);
	EXPECT_THROW(Convert::To<long double>(U"x45.4"), std::invalid_argument);
}

TEST(ConvertFundamentals, LongDoubleToString) {
	EXPECT_EQ("-0", Convert::ToString(-0.0L));
	EXPECT_EQ(u"3.14159265358979", Convert::To<std::u16string>(3.14159265358979L));
	EXPECT_EQ(U"-3.14159265358979", Convert::To<std::u32string>(-3.14159265358979L));
}
