/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "pch.h"
#include "bitserializer\string_conversion.h"

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
TEST(Convert, CharFromString) {
	EXPECT_EQ(-128, Convert::FromString<char>("  -128  "));
	EXPECT_EQ(127, Convert::FromString<char>(L"  +127  "));
}

TEST(Convert, CharToString) {
	EXPECT_EQ("-128", Convert::ToString(char(-128)));
	EXPECT_EQ(L"127", Convert::ToWString(char(127)));
}

TEST(Convert, UnsignedCharFromString) {
	EXPECT_EQ(255, Convert::FromString<unsigned char>("  255  "));
	EXPECT_EQ(255, Convert::FromString<unsigned char>(L"  255  "));
}

TEST(Convert, UnsignedCharToString) {
	EXPECT_EQ("255", Convert::ToString(unsigned char(255)));
	EXPECT_EQ(L"255", Convert::ToWString(unsigned char(255)));
}

//-----------------------------------------------------------------------------
TEST(Convert, ShortFromString) {
	EXPECT_EQ(-32768, Convert::FromString<short>("  -32768  "));
	EXPECT_EQ(32767, Convert::FromString<short>(L"  +32767  "));
}

TEST(Convert, ShortToString) {
	EXPECT_EQ("-32768", Convert::ToString(short(-32768)));
	EXPECT_EQ(L"32767", Convert::ToWString(short(32767)));
}

TEST(Convert, UnsignedShortFromString) {
	EXPECT_EQ(65535, Convert::FromString<unsigned short>("  65535  "));
	EXPECT_EQ(65535, Convert::FromString<unsigned short>(L"  65535  "));
}

TEST(Convert, UnsignedShortToString) {
	EXPECT_EQ("65535", Convert::ToString(unsigned short(65535)));
	EXPECT_EQ(L"65535", Convert::ToWString(unsigned short(65535)));
}

//-----------------------------------------------------------------------------
TEST(Convert, IntFromString) {
	EXPECT_EQ(-32768, Convert::FromString<int>("  -32768  "));
	EXPECT_EQ(32767, Convert::FromString<int>(L"  +32767  "));
}

TEST(Convert, IntToString) {
	EXPECT_EQ("-32767", Convert::ToString(-32767));
	EXPECT_EQ(L"32767", Convert::ToWString(32767));
}

TEST(Convert, UnsignedIntFromString) {
	EXPECT_EQ(65535, Convert::FromString<unsigned int>("  65535  "));
	EXPECT_EQ(65535, Convert::FromString<unsigned int>(L"  65535  "));
}

TEST(Convert, UnsignedIntToString) {
	EXPECT_EQ("65535", Convert::ToString(65535));
	EXPECT_EQ(L"65535", Convert::ToWString(65535));
}

//-----------------------------------------------------------------------------
TEST(Convert, LongFromString) {
	EXPECT_EQ(-2147483647l, Convert::FromString<long>("  -2147483647  "));
	EXPECT_EQ(2147483647, Convert::FromString<long>(L"  +2147483647  "));
}

TEST(Convert, LongToString) {
	EXPECT_EQ("-2147483647", Convert::ToString(-2147483647l));
	EXPECT_EQ(L"2147483647", Convert::ToWString(2147483647));
}

TEST(Convert, UnsignedLongFromString) {
	EXPECT_EQ(4294967295l, Convert::FromString<unsigned long>("  4294967295  "));
	EXPECT_EQ(4294967295l, Convert::FromString<unsigned long>(L"  4294967295  "));
}

TEST(Convert, UnsignedLongToString) {
	EXPECT_EQ("4294967295", Convert::ToString(4294967295l));
	EXPECT_EQ(L"4294967295", Convert::ToWString(4294967295l));
}

//-----------------------------------------------------------------------------
TEST(Convert, LongLongFromString) {
	EXPECT_EQ(-9223372036854775808ll, Convert::FromString<long long>("  -9223372036854775808  "));
	EXPECT_EQ(9223372036854775807ll, Convert::FromString<long long>(L"  +9223372036854775807  "));
}

TEST(Convert, LongLongToString) {
	EXPECT_EQ("-9223372036854775808", Convert::ToString(-9223372036854775808ll));
	EXPECT_EQ(L"9223372036854775807", Convert::ToWString(9223372036854775807));
}

TEST(Convert, UnsignedLongLongFromString) {
	EXPECT_EQ(18446744073709551615ull, Convert::FromString<unsigned long long>("  18446744073709551615  "));
	EXPECT_EQ(18446744073709551615ull, Convert::FromString<unsigned long long>(L"  18446744073709551615  "));
}

TEST(Convert, UnsignedLongLongToString) {
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
TEST(Convert, ThrowOutOfRangeExceptionForCharType) {
	EXPECT_THROW(Convert::FromString<char>("-129"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<char>("128"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<unsigned char>("-1"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<unsigned char>("256"), std::out_of_range);
}

TEST(Convert, ThrowOutOfRangeExceptionForShortType) {
	EXPECT_THROW(Convert::FromString<short>("-32769"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<short>("32768"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<unsigned short>("-1"), std::out_of_range);
	EXPECT_THROW(Convert::FromString<unsigned short>("65536"), std::out_of_range);
}

//-----------------------------------------------------------------------------
// Test conversion for enum types
//-----------------------------------------------------------------------------
enum class TestEnum {
	One = 1,
	Two = 2,
	Three = 3,
	Four = 4
};

namespace BitSerializer::Convert::Detail {
	static const bool _TestEnum = ConvertEnum::Register<TestEnum>(
	{
		{ TestEnum::One,	"One" },
		{ TestEnum::Two,	"Two" },
		{ TestEnum::Three,	"Three" },
		{ TestEnum::Four,	"Four" }
	});
} // namespace BitSerializer::Convert::Detail

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
struct TestStringConvertibleClass
{
	TestStringConvertibleClass() = default;

	TestStringConvertibleClass(int x, int y)
		: x(x), y(y)
	{ }

	std::string ToString() const {
		return std::to_string(x) + ' ' + std::to_string(y);
	}

	std::wstring ToWString() const {
		return std::to_wstring(x) + L' ' + std::to_wstring(y);
	}

	template <typename TSym, typename TAllocator>
	void FromString(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& str)
	{
		using str_type = std::basic_string<TSym, std::char_traits<TSym>, TAllocator>;
		str_type::size_type prev(0), f(0);
		for (int i = 0; i < 2; ++i)
		{
			prev = f = str.find_first_not_of(' ', f);
			if (prev != str_type::npos)
			{
				f = str.find_first_of(' ', f);
				switch (i)
				{
				case 0:
					x = std::stoi(str.substr(prev, f - prev));
					break;
				case 1:
					y = std::stoi(str.substr(prev, f - prev));
					break;
				}
			}
			else break;
		}
	}

	int x;
	int y;
};

TEST(Convert, ClassFromString) {
	auto actual = Convert::FromString<TestStringConvertibleClass>("100 -200");
	EXPECT_EQ(100, actual.x);
	EXPECT_EQ(-200, actual.y);
}

TEST(Convert, ClassFromWString) {
	auto actual = Convert::FromString<TestStringConvertibleClass>(L"-123 555");
	EXPECT_EQ(-123, actual.x);
	EXPECT_EQ(555, actual.y);
}

TEST(Convert, ClassToString) {
	EXPECT_EQ("16384 32768", Convert::ToString(TestStringConvertibleClass(16384, 32768)));
}

TEST(Convert, ClassToWString) {
	EXPECT_EQ(L"-777 -888", Convert::ToWString(TestStringConvertibleClass(-777, -888)));
}

//-----------------------------------------------------------------------------
// Test universal function for conversion
//-----------------------------------------------------------------------------
TEST(Convert, UniversalStringToInt) {
	EXPECT_EQ(-12345, Convert::To<int>(std::string("-12345")));
	EXPECT_EQ(-12345, Convert::To<int>(std::wstring(L"-12345")));
}

TEST(Convert, UniversalIntToString) {
	EXPECT_EQ("-12345", Convert::To<std::string>(-12345));
	EXPECT_EQ(L"-12345", Convert::To<std::wstring>(-12345));
}

//-----------------------------------------------------------------------------
// Traits tests
//-----------------------------------------------------------------------------
class TestConvertibleClass
{
public:
	std::string ToString() const { return std::string(); }
	std::wstring ToWString() const { return std::wstring(); }
	void FromString(const std::string& str) { }
	void FromString(const std::wstring& str) { }
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