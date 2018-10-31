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
#include "auto_fixture.h"
#include "bitserializer/bit_serializer.h"

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

	void Assert(const TestPointClass& rhs) const
	{
		EXPECT_EQ(x, rhs.x);
		EXPECT_EQ(y, rhs.y);
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
class TestClassWithInheritance : public TestPointClass
{
public:
	static void BuildFixture(TestClassWithInheritance& fixture)
	{
		TestPointClass::BuildFixture(fixture);
		::BuildFixture(fixture.TestUInt32);
		::BuildFixture(fixture.TestUInt64);
	}

	void Assert(const TestClassWithInheritance& rhs) const
	{
		this->TestPointClass::Assert(rhs);
		EXPECT_EQ(TestUInt32, rhs.TestUInt32);
		EXPECT_EQ(TestUInt64, rhs.TestUInt64);
	}

	template <class TArchive>
	inline void Serialize(TArchive& archive)
	{
		archive << BitSerializer::BaseObject<TestPointClass>(*this);
		archive << BitSerializer::MakeKeyValue("TestUInt32", TestUInt32);
		archive << BitSerializer::MakeKeyValue("TestUInt64", TestUInt64);
	};

	uint32_t TestUInt32;
	uint32_t TestUInt64;
};

//-----------------------------------------------------------------------------
template <typename T>
class TestClassWithSubType
{
public:
	TestClassWithSubType(const T& initValue = {})
		: TestSubValue(initValue)
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
template <class ...Args>
class TestClassWithSubTypes : public std::tuple<Args...>
{
public:
	TestClassWithSubTypes() = default;
	TestClassWithSubTypes(Args... args)
		: std::tuple<Args...>(args...)
	{
	}

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
