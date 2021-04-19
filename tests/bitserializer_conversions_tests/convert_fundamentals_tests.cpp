/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test conversion for fundamental types
//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, BoolFromString) {
	EXPECT_EQ(false, Convert::To<bool>("  0  "));
	EXPECT_EQ(true, Convert::To<bool>(u"  1  "));
	EXPECT_EQ(false, Convert::To<bool>(U"0"));
}

TEST(ConvertFundamentals, BoolToString) {
	EXPECT_EQ("0", Convert::ToString(false));
	EXPECT_EQ(u"1", Convert::To<std::u16string>(true));
	EXPECT_EQ(U"0", Convert::To<std::u32string>(false));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, Int8FromString) {
	EXPECT_EQ(0, Convert::To<int8_t>("  -0  "));
	EXPECT_EQ(-128, Convert::To<int8_t>(u"  -128  "));
	EXPECT_EQ(127, Convert::To<int8_t>(U"  127  "));
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

TEST(ConvertFundamentals, UInt16ToString) {
	EXPECT_EQ("0", Convert::ToString(uint16_t(0)));
	EXPECT_EQ(u"32768", Convert::To<std::u16string>(uint16_t(32768)));
	EXPECT_EQ(U"65535", Convert::To<std::u32string>(uint16_t(65535)));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, Int32FromString) {
	EXPECT_EQ(0, Convert::To<int32_t>("  -0  "));
	EXPECT_EQ(-2147483647, Convert::To<int32_t>(u"  -2147483647  "));
	EXPECT_EQ(2147483647, Convert::To<int32_t>(L"  2147483647  "));
	EXPECT_EQ(2147483647, Convert::To<int32_t>(U"  2147483647  "));
}

TEST(ConvertFundamentals, Int32ToString) {
	EXPECT_EQ("0", Convert::ToString(0));
	EXPECT_EQ(u"-2147483648", Convert::To<std::u16string>(-2147483648));
	EXPECT_EQ(U"2147483647", Convert::To<std::u32string>(2147483647));
}

TEST(ConvertFundamentals, UInt32FromString) {
	EXPECT_EQ(0, Convert::To<uint32_t>("  0  "));
	EXPECT_EQ(4294967295l, Convert::To<uint32_t>(u"  4294967295  "));
	EXPECT_EQ(4294967295l, Convert::To<uint32_t>(L"  4294967295  "));
	EXPECT_EQ(4294967295l, Convert::To<uint32_t>(U"  4294967295  "));
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

TEST(ConvertFundamentals, LongDoubleToString) {
	EXPECT_EQ("-0", Convert::ToString(-0.0L));
	EXPECT_EQ(u"3.14159265358979", Convert::To<std::u16string>(3.14159265358979L));
	EXPECT_EQ(U"-3.14159265358979", Convert::To<std::u32string>(-3.14159265358979L));
}

//-----------------------------------------------------------------------------
// Test out of range exception
//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, ThrowOutOfRangeExceptionForBool) {
	EXPECT_THROW(Convert::To<bool>("-1"), std::out_of_range);
}

TEST(ConvertFundamentals, ThrowOutOfRangeExceptionForInt8) {
	EXPECT_THROW(Convert::To<int8_t>("-129"), std::out_of_range);
	EXPECT_THROW(Convert::To<int8_t>("128"), std::out_of_range);
	EXPECT_THROW(Convert::To<uint8_t>("-1"), std::out_of_range);
	EXPECT_THROW(Convert::To<uint8_t>("256"), std::out_of_range);
}

TEST(ConvertFundamentals, ThrowOutOfRangeExceptionForInt16) {
	EXPECT_THROW(Convert::To<int16_t>("-32769"), std::out_of_range);
	EXPECT_THROW(Convert::To<int16_t>("32768"), std::out_of_range);
	EXPECT_THROW(Convert::To<uint16_t>("-1"), std::out_of_range);
	EXPECT_THROW(Convert::To<uint16_t>("65536"), std::out_of_range);
}

TEST(ConvertFundamentals, ThrowOutOfRangeExceptionForInt32) {
	EXPECT_THROW(Convert::To<int32_t>("-2147483649"), std::out_of_range);
	EXPECT_THROW(Convert::To<int32_t>("2147483648"), std::out_of_range);
	EXPECT_THROW(Convert::To<uint32_t>("4294967296"), std::out_of_range);
}

TEST(ConvertFundamentals, ThrowOutOfRangeExceptionForInt64) {
	EXPECT_THROW(Convert::To<int64_t>("-9223372036854775809"), std::out_of_range);
	EXPECT_THROW(Convert::To<int64_t>("9223372036854775808"), std::out_of_range);
	EXPECT_THROW(Convert::To<uint64_t>("18446744073709551616"), std::out_of_range);
}
