/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "pch.h"
#include "../test-helpers/common_test_entities.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test conversion to the same type
//-----------------------------------------------------------------------------
TEST(Convert, StringToString) {
	EXPECT_EQ("Test", Convert::ToString("Test"));
}

TEST(Convert, WStringToWString) {
	EXPECT_EQ(L"Test", Convert::ToWString(L"Test"));
}

TEST(Convert, StringFromString) {
	EXPECT_EQ("Test", Convert::FromString<std::string>("Test"));
}

TEST(Convert, WStringFromWString) {
	EXPECT_EQ(L"Test", Convert::FromString<std::wstring>(L"Test"));
}

//-----------------------------------------------------------------------------
// Test conversion std::string to std::wstring and vice versa
//-----------------------------------------------------------------------------
TEST(Convert, StringToWString) {
	EXPECT_EQ(L"Test", Convert::ToWString("Test"));
}

TEST(Convert, WStringToString) {
	EXPECT_EQ("Test", Convert::ToString(L"Test"));
}

TEST(Convert, StringFromWString) {
	EXPECT_EQ("Test", Convert::FromString<std::string>(L"Test"));
}

TEST(Convert, WStringFromString) {
	EXPECT_EQ(L"Test", Convert::FromString<std::wstring>("Test"));
}

//-----------------------------------------------------------------------------
// Test conversion for fundamental types
//-----------------------------------------------------------------------------
TEST(Convert, BoolFromString) {
	EXPECT_EQ(false, Convert::FromString<bool>("0"));
	EXPECT_EQ(true, Convert::FromString<bool>("1"));
}

TEST(Convert, BoolFromWString) {
	EXPECT_EQ(false, Convert::FromString<bool>(L"0"));
	EXPECT_EQ(true, Convert::FromString<bool>(L"1"));
}

TEST(Convert, BoolToString) {
	EXPECT_EQ("0", Convert::ToString(false));
	EXPECT_EQ("1", Convert::ToString(true));
}

TEST(Convert, BoolToWString) {
	EXPECT_EQ(L"0", Convert::ToWString(false));
	EXPECT_EQ(L"1", Convert::ToWString(true));
}

//-----------------------------------------------------------------------------
TEST(Convert, Int8FromString) {
	EXPECT_EQ(-128, Convert::FromString<int8_t>("  -128  "));
	EXPECT_EQ(127, Convert::FromString<int8_t>(L"  +127  "));
}

TEST(Convert, Int8ToString) {
	EXPECT_EQ("-128", Convert::ToString(int8_t(-128)));
	EXPECT_EQ(L"127", Convert::ToWString(int8_t(127)));
}

TEST(Convert, UInt8FromString) {
	EXPECT_EQ(255, Convert::FromString<uint8_t>("  255  "));
	EXPECT_EQ(255, Convert::FromString<uint8_t>(L"  255  "));
}

TEST(Convert, UInt8ToString) {
	EXPECT_EQ("255", Convert::ToString(uint8_t(255)));
	EXPECT_EQ(L"255", Convert::ToWString(uint8_t(255)));
}

//-----------------------------------------------------------------------------
TEST(Convert, Int16FromString) {
	EXPECT_EQ(-32768, Convert::FromString<int16_t>("  -32768  "));
	EXPECT_EQ(32767, Convert::FromString<int16_t>(L"  +32767  "));
}

TEST(Convert, Int16ToString) {
	EXPECT_EQ("-32768", Convert::ToString(int16_t(-32768)));
	EXPECT_EQ(L"32767", Convert::ToWString(int16_t(32767)));
}

TEST(Convert, UInt16FromString) {
	EXPECT_EQ(65535, Convert::FromString<uint16_t>("  65535  "));
	EXPECT_EQ(65535, Convert::FromString<uint16_t>(L"  65535  "));
}

TEST(Convert, UInt16ToString) {
	EXPECT_EQ("65535", Convert::ToString(uint16_t(65535)));
	EXPECT_EQ(L"65535", Convert::ToWString(uint16_t(65535)));
}

//-----------------------------------------------------------------------------
TEST(Convert, Int32FromString) {
	EXPECT_EQ(-2147483647l, Convert::FromString<int32_t>("  -2147483647  "));
	EXPECT_EQ(2147483647, Convert::FromString<int32_t>(L"  +2147483647  "));
}

TEST(Convert, Int32ToString) {
	EXPECT_EQ("-2147483647", Convert::ToString(-2147483647l));
	EXPECT_EQ(L"2147483647", Convert::ToWString(2147483647));
}

TEST(Convert, UInt32FromString) {
	EXPECT_EQ(4294967295l, Convert::FromString<uint32_t>("  4294967295  "));
	EXPECT_EQ(4294967295l, Convert::FromString<uint32_t>(L"  4294967295  "));
}

TEST(Convert, UInt32ToString) {
	EXPECT_EQ("4294967295", Convert::ToString(4294967295l));
	EXPECT_EQ(L"4294967295", Convert::ToWString(4294967295l));
}

//-----------------------------------------------------------------------------
TEST(Convert, Int64FromString) {
	EXPECT_EQ(-9223372036854775808ll, Convert::FromString<int64_t>("  -9223372036854775808  "));
	EXPECT_EQ(9223372036854775807ll, Convert::FromString<int64_t>(L"  +9223372036854775807  "));
}

TEST(Convert, Int64ToString) {
	EXPECT_EQ("-9223372036854775808", Convert::ToString(-9223372036854775808ll));
	EXPECT_EQ(L"9223372036854775807", Convert::ToWString(9223372036854775807));
}

TEST(Convert, UInt64FromString) {
	EXPECT_EQ(18446744073709551615ull, Convert::FromString<uint64_t>("  18446744073709551615  "));
	EXPECT_EQ(18446744073709551615ull, Convert::FromString<uint64_t>(L"  18446744073709551615  "));
}

TEST(Convert, UInt64ToString) {
	EXPECT_EQ("18446744073709551615", Convert::ToString(18446744073709551615ull));
	EXPECT_EQ(L"18446744073709551615", Convert::ToWString(18446744073709551615ull));
}

//-----------------------------------------------------------------------------
TEST(Convert, FloatFromString) {
	EXPECT_EQ(-123.123f, Convert::FromString<float>("  -123.123  "));
	EXPECT_EQ(-123.123f, Convert::FromString<float>(L"  -123.123  "));
}

TEST(Convert, FloatToString) {
	EXPECT_EQ("-100.500000", Convert::ToString(-100.5f));
	EXPECT_EQ(L"-100.500000", Convert::ToWString(-100.5f));
}

//-----------------------------------------------------------------------------
TEST(Convert, DoubleFromString) {
	EXPECT_EQ(-12345.12345, Convert::FromString<double>("  -12345.12345  "));
	EXPECT_EQ(-12345.12345, Convert::FromString<double>(L"  -12345.12345  "));
}

TEST(Convert, DoubleToString) {
	EXPECT_EQ("-12345.123450", Convert::ToString(-12345.12345));
	EXPECT_EQ(L"-12345.123450", Convert::ToWString(-12345.12345));
}

//-----------------------------------------------------------------------------
// Test conversion from hexadecimal numbers
//-----------------------------------------------------------------------------
TEST(Convert, HexUnsignedIntFromString) {
	EXPECT_EQ(65535, Convert::FromString<unsigned int>("  0xFFFF  "));
	EXPECT_EQ(65535, Convert::FromString<unsigned int>(L"  0Xffff  "));
}

TEST(Convert, HexIntFromString) {
	EXPECT_EQ(-32767, Convert::FromString<int>("  -0x7fff  "));
	EXPECT_EQ(32767, Convert::FromString<int>(L"  +0X7FFF  "));
}

//-----------------------------------------------------------------------------
// Test out of range exception
//-----------------------------------------------------------------------------
TEST(Convert, ThrowOutOfRangeExceptionForInt8) {
	EXPECT_THROW(Convert::FromString<int8_t>("-129"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<int8_t>("128"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<uint8_t>("-1"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<uint8_t>("256"), std::out_of_range);
}

TEST(Convert, ThrowOutOfRangeExceptionForInt16) {
	EXPECT_THROW(Convert::FromString<int16_t>("-32769"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<int16_t>("32768"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<uint16_t>("-1"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<uint16_t>("65536"), std::out_of_range);
}

TEST(Convert, ThrowOutOfRangeExceptionForInt32) {
	EXPECT_THROW(Convert::FromString<int32_t>("-2147483649"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<int32_t>("2147483648"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<uint32_t>("4294967296"), std::out_of_range);
}

TEST(Convert, ThrowOutOfRangeExceptionForInt64) {
	EXPECT_THROW(Convert::FromString<int64_t>("-9223372036854775809"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<int64_t>("9223372036854775808"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<uint64_t>("18446744073709551616"), std::out_of_range);
}

//-----------------------------------------------------------------------------
// Test conversion for enum types
//-----------------------------------------------------------------------------
TEST(Convert, EnumFromString) {
	EXPECT_EQ(TestEnum::One, Convert::FromString<TestEnum>("One"));
}

TEST(Convert, EnumFromWString) {
	EXPECT_EQ(TestEnum::Two, Convert::FromString<TestEnum>(L"Two"));
}

TEST(Convert, EnumToString) {
	EXPECT_EQ("Three", Convert::ToString(TestEnum::Three));
}

TEST(Convert, EnumToWString) {
	EXPECT_EQ(L"Four", Convert::ToWString(TestEnum::Four));
}

//-----------------------------------------------------------------------------
// Test conversion for class types (struct, class, union)
//-----------------------------------------------------------------------------
TEST(Convert, ClassFromString) {
	auto actual = Convert::FromString<TestPointClass>("100 -200");
	EXPECT_EQ(TestPointClass(100, -200), actual);
}

TEST(Convert, ClassFromWString) {
	auto actual = Convert::FromString<TestPointClass>(L"-123 555");
	EXPECT_EQ(TestPointClass(-123, 555), actual);
}

TEST(Convert, ClassToString) {
	EXPECT_EQ("16384 32768", Convert::ToString(TestPointClass(16384, 32768)));
}

TEST(Convert, ClassToWString) {
	EXPECT_EQ(L"-777 -888", Convert::ToWString(TestPointClass(-777, -888)));
}

//-----------------------------------------------------------------------------
// Test universal function for conversion
//-----------------------------------------------------------------------------
TEST(Convert, UniversalStringToString) {
	const char* testStr = "Test ANSI string";
	EXPECT_EQ(std::string(testStr), Convert::To<std::string>(testStr));
	EXPECT_EQ(std::string(testStr), Convert::To<std::string>(std::string(testStr)));
}

TEST(Convert, UniversalWStringToWString) {
	const wchar_t* testWStr = L"Test wide string";
	EXPECT_EQ(std::wstring(testWStr), Convert::To<std::wstring>(testWStr));
	EXPECT_EQ(std::wstring(testWStr), Convert::To<std::wstring>(std::wstring(testWStr)));
}

TEST(Convert, UniversalStringToInt) {
	EXPECT_EQ(-12345, Convert::To<int16_t>("-12345"));
	EXPECT_EQ(-12345, Convert::To<int16_t>(L"-12345"));
	EXPECT_EQ(-12345, Convert::To<int16_t>(std::string("-12345")));
	EXPECT_EQ(-12345, Convert::To<int16_t>(std::wstring(L"-12345")));
}

TEST(Convert, UniversalIntToString) {
	EXPECT_EQ("-12345", Convert::To<std::string>(-12345));
	EXPECT_EQ(L"-12345", Convert::To<std::wstring>(-12345));
}

//-----------------------------------------------------------------------------
// Test streaming functions
//-----------------------------------------------------------------------------
TEST(Convert, ConvertClassToStream) {
	std::ostringstream oss;
	oss << TestPointClass(543, -345);
	EXPECT_EQ("543 -345", oss.str());
}

TEST(Convert, ConvertClassToWStream) {
	std::wostringstream oss;
	oss << TestPointClass(543, -345);
	EXPECT_EQ(L"543 -345", oss.str());
}

TEST(Convert, ConvertEnumToStream) {
	std::ostringstream oss;
	oss << TestEnum::Five;
	EXPECT_EQ("Five", oss.str());
}

TEST(Convert, ConvertEnumToWStream) {
	std::wostringstream oss;
	oss << TestEnum::Five;
	EXPECT_EQ(L"Five", oss.str());
}

//-----------------------------------------------------------------------------
// Traits tests
//-----------------------------------------------------------------------------
class TestConvertibleClass
{
public:
	std::string ToString() const				{ return std::string(); }
	std::wstring ToWString() const				{ return std::wstring(); }
	void FromString(const std::string& str)		{ }
	void FromString(const std::wstring& str)	{ }
};

class TestNotConvertibleClass { };

TEST(Convert, ShouldCheckThatClassHasToStringMethod) {
	bool convertibleResult = Convert::Detail::has_to_string_v<TestConvertibleClass, std::string>;
	EXPECT_TRUE(convertibleResult);
	bool notConvertibleResult = Convert::Detail::has_to_string_v<TestNotConvertibleClass, std::string>;
	EXPECT_FALSE(notConvertibleResult);
}

TEST(Convert, ShouldCheckThatClassHasToWStringMethod) {
	bool convertibleResult = Convert::Detail::has_to_string_v<TestConvertibleClass, std::wstring>;
	EXPECT_TRUE(convertibleResult);
	bool notConvertibleResult = Convert::Detail::has_to_string_v<TestNotConvertibleClass, std::wstring>;
	EXPECT_FALSE(notConvertibleResult);
}

TEST(Convert, ShouldCheckThatClassHasFromStringMethod) {
	bool convertibleResult = Convert::Detail::has_from_string_v<TestConvertibleClass, std::string>;
	EXPECT_TRUE(convertibleResult);
	bool notConvertibleResult = Convert::Detail::has_from_string_v<TestNotConvertibleClass, std::string>;
	EXPECT_FALSE(notConvertibleResult);
}

TEST(Convert, ShouldCheckThatClassHasFromWStringMethod) {
	bool convertibleResult = Convert::Detail::has_from_string_v<TestConvertibleClass, std::wstring>;
	EXPECT_TRUE(convertibleResult);
	bool notConvertibleResult = Convert::Detail::has_from_string_v<TestNotConvertibleClass, std::wstring>;
	EXPECT_FALSE(notConvertibleResult);
}