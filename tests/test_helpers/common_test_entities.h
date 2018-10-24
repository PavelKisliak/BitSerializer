/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdlib>
#include <type_traits>
#include <functional>
#include <tuple>
#include "gtest/gtest.h"
#include "common_test_methods.h"

//-----------------------------------------------------------------------------
enum class TestEnum {
	One = 1,
	Two = 2,
	Three = 3,
	Four = 4,
	Five = 5
};

REGISTER_ENUM_MAP(TestEnum)
{
	{ TestEnum::One,	"One" },
	{ TestEnum::Two,	"Two" },
	{ TestEnum::Three,	"Three" },
	{ TestEnum::Four,	"Four" },
	{ TestEnum::Five,	"Five" }
} END_ENUM_MAP()

//-----------------------------------------------------------------------------
class TestPointClass
{
public:
	static void BuildFixture(TestPointClass& fixture) {
		::BuildFixture(fixture.x);
		::BuildFixture(fixture.y);
	}

	TestPointClass() = default;

	TestPointClass(int x, int y)
		: x(x), y(y)
	{ }

	bool operator==(const TestPointClass& rhs) const {
		return x == rhs.x && y == rhs.y;
	}

	bool operator<(const TestPointClass& rhs) const {
		return x < rhs.x || (!(rhs.x < x) && y < rhs.y);
	}

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
		typename str_type::size_type prev(0), f(0);
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

	template <class TArchive>
	inline void Serialize(TArchive& archive)
	{
		archive << BitSerializer::MakeKeyValue("x", x);
		archive << BitSerializer::MakeKeyValue("y", y);
	};

	int x;
	int y;
};

//-----------------------------------------------------------------------------
template <class ...Args>
class TestClassWithSubTypes : public std::tuple<Args...>
{
public:
	template<std::size_t I = 0>
	static void BuildFixture(TestClassWithSubTypes& fixture)
	{
		if constexpr (I == sizeof...(Args))
			return;
		else
		{
			decltype(auto) member = std::get<I>(fixture);
			::BuildFixture(member);

			// Next
			BuildFixture<I + 1>(fixture);
		}
	}

	template<std::size_t I = 0>
	void Assert(const TestClassWithSubTypes& rhs) const
	{
		if constexpr (I == sizeof...(Args))
			return;
		else
		{
			// Get class member
			decltype(auto) expected = std::get<I>(*this);
			decltype(auto) actual = std::get<I>(rhs);

			// Assert
			EXPECT_EQ(expected, actual);

			// Next
			Assert<I + 1>(rhs);
		}
	}

	template <class TArchive, std::size_t I = 0>
	void Serialize(TArchive& archive)
	{
		if constexpr (I == sizeof...(Args))
			return;
		else
		{
			decltype(auto) member = std::get<I>(*this);
			std::string key = "Member_" + BitSerializer::Convert::ToString(I);
			archive << BitSerializer::MakeKeyValue(key, member);

			// Next
			Serialize<TArchive, I + 1>(archive);
		}
	};
};

//-----------------------------------------------------------------------------
class TestClassWithFundamentalTypes
{
public:
	static void BuildFixture(TestClassWithFundamentalTypes& fixture)
	{
		::BuildFixture(fixture.TestBool);
		::BuildFixture(fixture.TestInt8);
		::BuildFixture(fixture.TestInt16);
		::BuildFixture(fixture.TestInt32);
		::BuildFixture(fixture.TestInt64);
		::BuildFixture(fixture.TestFloat);
		::BuildFixture(fixture.TestDouble);
		::BuildFixture(fixture.TestEnum);
		::BuildFixture(fixture.TestString);
		::BuildFixture(fixture.TestWString);
	}

	void Assert(const TestClassWithFundamentalTypes& rhs) const
	{
		EXPECT_EQ(TestBool, rhs.TestBool);
		EXPECT_EQ(TestInt8, rhs.TestInt8);
		EXPECT_EQ(TestInt16, rhs.TestInt16);
		EXPECT_EQ(TestInt32, rhs.TestInt32);
		EXPECT_EQ(TestInt64, rhs.TestInt64);
		EXPECT_EQ(TestFloat, rhs.TestFloat);
		EXPECT_EQ(TestDouble, rhs.TestDouble);
		EXPECT_EQ(TestEnum, rhs.TestEnum);
		EXPECT_EQ(TestString, rhs.TestString);
		EXPECT_EQ(TestWString, rhs.TestWString);
	}

	template <class TArchive>
	inline void Serialize(TArchive& archive)
	{
		archive << BitSerializer::MakeKeyValue("TestBool", TestBool);
		archive << BitSerializer::MakeKeyValue("TestInt8", TestInt8);
		archive << BitSerializer::MakeKeyValue("TestInt16", TestInt16);
		archive << BitSerializer::MakeKeyValue("TestInt32", TestInt32);
		archive << BitSerializer::MakeKeyValue("TestInt64", TestInt64);
		archive << BitSerializer::MakeKeyValue("TestFloat", TestFloat);
		archive << BitSerializer::MakeKeyValue("TestDouble", TestDouble);
		archive << BitSerializer::MakeKeyValue("TestEnum", TestEnum);
		archive << BitSerializer::MakeKeyValue("TestString", TestString);
		archive << BitSerializer::MakeKeyValue("TestWString", TestWString);
	};

	bool TestBool;
	int8_t TestInt8;
	int16_t TestInt16;
	int32_t TestInt32;
	int64_t TestInt64;
	float TestFloat;
	double TestDouble;
	TestEnum TestEnum;
	std::string TestString;
	std::string TestWString;
};

//-----------------------------------------------------------------------------
class TestClassWithInheritance : public TestClassWithFundamentalTypes
{
public:
	static void BuildFixture(TestClassWithInheritance& fixture)
	{
		TestClassWithFundamentalTypes::BuildFixture(fixture);
		::BuildFixture(fixture.TestUInt32);
		::BuildFixture(fixture.TestUInt64);
	}

	void Assert(const TestClassWithInheritance& rhs) const
	{
		this->TestClassWithFundamentalTypes::Assert(rhs);
		EXPECT_EQ(TestUInt32, rhs.TestUInt32);
		EXPECT_EQ(TestUInt64, rhs.TestUInt64);
	}

	template <class TArchive>
	inline void Serialize(TArchive& archive)
	{
		archive << BitSerializer::BaseObject<TestClassWithFundamentalTypes>(*this);
		archive << BitSerializer::MakeKeyValue("TestUInt32", TestUInt32);
		archive << BitSerializer::MakeKeyValue("TestUInt64", TestUInt64);
	};

	uint32_t TestUInt32;
	uint32_t TestUInt64;
};

//-----------------------------------------------------------------------------
class TestClassWithSubClass
{
public:
	static void BuildFixture(TestClassWithSubClass& fixture) {
		TestClassWithFundamentalTypes::BuildFixture(fixture.TestSubClass);
	}

	void Assert(const TestClassWithSubClass& rhs) const {
		TestSubClass.Assert(rhs.TestSubClass);
	}

	template <class TArchive>
	inline void Serialize(TArchive& archive) {
		archive << BitSerializer::MakeKeyValue("TestSubClass", TestSubClass);
	};

	TestClassWithFundamentalTypes TestSubClass;
};

//-----------------------------------------------------------------------------
template <typename T>
class TestClassWithSubType
{
public:
	TestClassWithSubType()
	{
		mAssertFunc = [](const T& expected, const T& actual) {
			ASSERT_EQ(expected, actual);
		};
	}

	TestClassWithSubType(const std::function<void(const T&, const T&)>& specialAssertFunc)
		: mAssertFunc(specialAssertFunc)
	{ }

	static void BuildFixture(TestClassWithSubType& fixture) {
		::BuildFixture(fixture.TestSubValue);
	}

	void Assert(const TestClassWithSubType& actual) const {
		mAssertFunc(TestSubValue, actual.TestSubValue);
	}

	template <class TArchive>
	inline void Serialize(TArchive& archive) {
		archive << BitSerializer::MakeKeyValue("TestSubValue", TestSubValue);
	};

	T TestSubValue;

private:
	std::function<void(const T&, const T&)> mAssertFunc;
};

//-----------------------------------------------------------------------------
template <typename T, size_t ArraySize = 7>
class TestClassWithSubArray
{
public:
	static void BuildFixture(TestClassWithSubArray<T>& fixture) {
		::BuildFixture(fixture.TestArray);
	}

	void Assert(const TestClassWithSubArray<T>& rhs) const {
		for (size_t i = 0; i < ArraySize; i++) {
			ASSERT_EQ(TestArray[i], rhs.TestArray[i]);
		}
	}

	template <class TArchive>
	inline void Serialize(TArchive& archive) {
		archive << BitSerializer::MakeKeyValue("TestArray", TestArray);
	};

	T TestArray[ArraySize];
};

//-----------------------------------------------------------------------------
template <typename T, size_t ArraySize1 = 3, size_t ArraySize2 = 5>
class TestClassWithSubTwoDimArray
{
public:
	static void BuildFixture(TestClassWithSubTwoDimArray& fixture) {
		::BuildFixture(fixture.TestTwoDimArray);
	}

	void Assert(const TestClassWithSubTwoDimArray& rhs) const {
		for (size_t i = 0; i < ArraySize1; i++) {
			for (size_t c = 0; c < ArraySize2; c++) {
				ASSERT_EQ(TestTwoDimArray[i][c], rhs.TestTwoDimArray[i][c]);
			}
		}
	}

	template <class TArchive>
	inline void Serialize(TArchive& archive) {
		archive << BitSerializer::MakeKeyValue("TestTwoDimArray", TestTwoDimArray);
	};

	T TestTwoDimArray[ArraySize1][ArraySize2];
};

//-----------------------------------------------------------------------------
template <typename TestType>
class TestClassForCheckValidation
{
public:
	void Assert() const
	{
		EXPECT_EQ(2, BitSerializer::Context.GetValidationErrors().size());
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::MakeKeyValue("existSingleField", existSingleField, BitSerializer::Required());
		archive << BitSerializer::MakeKeyValue("existArrayField", existArrayField, BitSerializer::Required());

		// Trying to load not exist fields
		if (archive.IsLoading())
		{
			TestType notExistSingleField;
			TestType notExistArrayField[3];
			archive << BitSerializer::MakeKeyValue("notExistSingleField", notExistSingleField, BitSerializer::Required());
			archive << BitSerializer::MakeKeyValue("notExistArrayField", notExistArrayField, BitSerializer::Required());
		}
	};

	TestType existSingleField;
	TestType existArrayField[3];
};
