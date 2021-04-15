/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "test_helpers/common_test_entities.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test conversion for fundamental types
//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, BoolFromString) {
	EXPECT_EQ(false, Convert::To<bool>("0"));
	EXPECT_EQ(true, Convert::To<bool>("1"));
}

TEST(ConvertFundamentals, BoolFromWString) {
	EXPECT_EQ(false, Convert::To<bool>(L"0"));
	EXPECT_EQ(true, Convert::To<bool>(L"1"));
}

TEST(ConvertFundamentals, BoolToString) {
	EXPECT_EQ("0", Convert::ToString(false));
	EXPECT_EQ("1", Convert::ToString(true));
}

TEST(ConvertFundamentals, BoolToWString) {
	EXPECT_EQ(L"0", Convert::ToWString(false));
	EXPECT_EQ(L"1", Convert::ToWString(true));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, Int8FromString) {
	EXPECT_EQ(-128, Convert::To<int8_t>("  -128  "));
	EXPECT_EQ(127, Convert::To<int8_t>(L"  +127  "));
}

TEST(ConvertFundamentals, Int8ToString) {
	EXPECT_EQ("-128", Convert::ToString(int8_t(-128)));
	EXPECT_EQ(L"127", Convert::ToWString(int8_t(127)));
}

TEST(ConvertFundamentals, UInt8FromString) {
	EXPECT_EQ(255, Convert::To<uint8_t>("  255  "));
	EXPECT_EQ(255, Convert::To<uint8_t>(L"  255  "));
}

TEST(ConvertFundamentals, UInt8ToString) {
	EXPECT_EQ("255", Convert::ToString(uint8_t(255)));
	EXPECT_EQ(L"255", Convert::ToWString(uint8_t(255)));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, Int16FromString) {
	EXPECT_EQ(-32768, Convert::To<int16_t>("  -32768  "));
	EXPECT_EQ(32767, Convert::To<int16_t>(L"  +32767  "));
}

TEST(ConvertFundamentals, Int16ToString) {
	EXPECT_EQ("-32768", Convert::ToString(int16_t(-32768)));
	EXPECT_EQ(L"32767", Convert::ToWString(int16_t(32767)));
}

TEST(ConvertFundamentals, UInt16FromString) {
	EXPECT_EQ(65535, Convert::To<uint16_t>("  65535  "));
	EXPECT_EQ(65535, Convert::To<uint16_t>(L"  65535  "));
}

TEST(ConvertFundamentals, UInt16ToString) {
	EXPECT_EQ("65535", Convert::ToString(uint16_t(65535)));
	EXPECT_EQ(L"65535", Convert::ToWString(uint16_t(65535)));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, Int32FromString) {
	EXPECT_EQ(-2147483647l, Convert::To<int32_t>("  -2147483647  "));
	EXPECT_EQ(2147483647, Convert::To<int32_t>(L"  +2147483647  "));
}

TEST(ConvertFundamentals, Int32ToString) {
	EXPECT_EQ("-2147483647", Convert::ToString(-2147483647l));
	EXPECT_EQ(L"2147483647", Convert::ToWString(2147483647));
}

TEST(ConvertFundamentals, UInt32FromString) {
	EXPECT_EQ(4294967295l, Convert::To<uint32_t>("  4294967295  "));
	EXPECT_EQ(4294967295l, Convert::To<uint32_t>(L"  4294967295  "));
}

TEST(ConvertFundamentals, UInt32ToString) {
	EXPECT_EQ("4294967295", Convert::ToString(4294967295l));
	EXPECT_EQ(L"4294967295", Convert::ToWString(4294967295l));
}

TEST(ConvertFundamentals, Int64FromString) {
	EXPECT_EQ(std::numeric_limits<int64_t>::min(), Convert::To<int64_t>("  -9223372036854775808  "));
	EXPECT_EQ(std::numeric_limits<int64_t>::max(), Convert::To<int64_t>(L"  +9223372036854775807  "));
}

TEST(ConvertFundamentals, Int64ToString) {
	EXPECT_EQ("-9223372036854775808", Convert::ToString(std::numeric_limits<int64_t>::min()));
	EXPECT_EQ(L"9223372036854775807", Convert::ToWString(std::numeric_limits<int64_t>::max()));
}

TEST(ConvertFundamentals, UInt64FromString) {
	EXPECT_EQ(std::numeric_limits<uint64_t>::max(), Convert::To<uint64_t>("  18446744073709551615  "));
	EXPECT_EQ(std::numeric_limits<uint64_t>::max(), Convert::To<uint64_t>(L"  18446744073709551615  "));
}

TEST(ConvertFundamentals, UInt64ToString) {
	EXPECT_EQ("18446744073709551615", Convert::ToString(18446744073709551615ull));
	EXPECT_EQ(L"18446744073709551615", Convert::ToWString(18446744073709551615ull));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, FloatFromString) {
	EXPECT_EQ(-123.123f, Convert::To<float>("  -123.123  "));
	EXPECT_EQ(-123.123f, Convert::To<float>(L"  -123.123  "));
}

TEST(ConvertFundamentals, FloatToString) {
	EXPECT_EQ("-100.500000", Convert::ToString(-100.5f));
	EXPECT_EQ(L"-100.500000", Convert::ToWString(-100.5f));
}

//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, DoubleFromString) {
	EXPECT_EQ(-12345.12345, Convert::To<double>("  -12345.12345  "));
	EXPECT_EQ(-12345.12345, Convert::To<double>(L"  -12345.12345  "));
}

TEST(ConvertFundamentals, DoubleToString) {
	EXPECT_EQ("-12345.123450", Convert::ToString(-12345.12345));
	EXPECT_EQ(L"-12345.123450", Convert::ToWString(-12345.12345));
}

//-----------------------------------------------------------------------------
// Test conversion from hexadecimal numbers
//-----------------------------------------------------------------------------
TEST(ConvertFundamentals, HexUnsignedIntFromString) {
	EXPECT_EQ(65535, Convert::To<unsigned int>("  0xFFFF  "));
	EXPECT_EQ(65535, Convert::To<unsigned int>(L"  0Xffff  "));
}

TEST(ConvertFundamentals, HexIntFromString) {
	EXPECT_EQ(-32767, Convert::To<int>("  -0x7fff  "));
	EXPECT_EQ(32767, Convert::To<int>(L"  +0X7FFF  "));
}

//-----------------------------------------------------------------------------
// Test out of range exception
//-----------------------------------------------------------------------------
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
