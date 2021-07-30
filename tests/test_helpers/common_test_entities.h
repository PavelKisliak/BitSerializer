/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <charconv>
#include <cstddef>
#include <functional>
#include <tuple>
#include <utility>
#include <gtest/gtest.h>
#include "auto_fixture.h"
#include "gtest_asserts.h"
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
DECLARE_ENUM_STREAM_OPS(TestEnum)


//-----------------------------------------------------------------------------
union TestUnion
{
	static void BuildFixture(TestUnion& fixture) {
		::BuildFixture(fixture.mIntValue);
	}

	void Assert(const TestUnion& rhs) const
	{
		EXPECT_EQ(mIntValue, rhs.mIntValue);
	}

	TestUnion() = default;

	TestUnion(int x) noexcept
		: mIntValue(x)
	{ }

	bool operator==(const TestUnion& rhs) const noexcept { return mIntValue == rhs.mIntValue; }

	[[nodiscard]] std::string ToString() const { return std::to_string(mIntValue); }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::MakeAutoKeyValue("value", mIntValue);
	}

	int mIntValue;
	float mFloatValue;
};

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

	TestPointClass(int x, int y) noexcept
		: x(x), y(y)
	{ }

	bool operator==(const TestPointClass& rhs) const noexcept {
		return x == rhs.x && y == rhs.y;
	}

	bool operator<(const TestPointClass& rhs) const noexcept {
		return x < rhs.x || (!(rhs.x < x) && y < rhs.y);
	}

	[[nodiscard]] std::string ToString() const {
		return std::to_string(x) + ' ' + std::to_string(y);
	}

	void FromString(std::string_view str)
	{
		const auto next = str.find_first_of(' ');
		if (next != std::string_view::npos)
		{
			std::from_chars(str.data(), str.data() + str.size(), x);
			std::from_chars(str.data() + next + 1, str.data() + str.size(), y);
		}
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::MakeAutoKeyValue("x", x);
		archive << BitSerializer::MakeAutoKeyValue("y", y);
	}

	int x = 0;
	int y = 0;
};

namespace std
{
	template<>
	struct hash<TestPointClass>
	{
		using argument_type = TestPointClass;
		using result_type = size_t;

		result_type operator()(const argument_type& c) const noexcept
		{
			return static_cast<result_type>(c.x) + static_cast<result_type>(c.y);
		}
	};
}

//-----------------------------------------------------------------------------
class TestClassWithInheritance : public TestPointClass
{
public:
	static void BuildFixture(TestClassWithInheritance& fixture)
	{
		TestPointClass::BuildFixture(fixture);
		::BuildFixture(fixture.mTestUInt32);
		::BuildFixture(fixture.mTestUInt64);
	}

	void Assert(const TestClassWithInheritance& rhs) const
	{
		this->TestPointClass::Assert(rhs);
		EXPECT_EQ(mTestUInt32, rhs.mTestUInt32);
		EXPECT_EQ(mTestUInt64, rhs.mTestUInt64);
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::BaseObject<TestPointClass>(*this);

		archive << BitSerializer::MakeAutoKeyValue(L"TestUInt32", mTestUInt32);
		archive << BitSerializer::MakeAutoKeyValue(L"TestUInt64", mTestUInt64);
	}

private:
	uint32_t mTestUInt32 = 0u;
	uint64_t mTestUInt64 = 0u;
};

//-----------------------------------------------------------------------------
template <typename T>
class TestClassWithSubType
{
public:
	TestClassWithSubType()
	{
		::BuildFixture(mTestValue);
		mAssertFunc = [](const T& expected, const T& actual) {
			GTestExpectEq(expected, actual);
		};
	}

	explicit TestClassWithSubType(T initValue)
		: mTestValue(std::move(initValue))
	{
		mAssertFunc = [](const T& expected, const T& actual) {
			GTestExpectEq(expected, actual);
		};
	}

	TestClassWithSubType(std::function<void(const T&, const T&)> specialAssertFunc)
		: mAssertFunc(std::move(specialAssertFunc))
	{
		::BuildFixture(mTestValue);
	}

	static void BuildFixture(TestClassWithSubType& fixture) {
		::BuildFixture(fixture.mTestValue);
	}

	void Assert(const TestClassWithSubType& actual) const {
		mAssertFunc(mTestValue, actual.mTestValue);
	}

	template <class TArchive>
	void Serialize(TArchive& archive) {
		archive << BitSerializer::MakeAutoKeyValue("TestValue", mTestValue);
	}

	const T& GetValue() const { return mTestValue; }

private:
	T mTestValue;
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
			using TValue = std::tuple_element_t<I, std::tuple<Args...>>;

			// Get class member
			decltype(auto) expected = std::get<I>(*this);
			decltype(auto) actual = std::get<I>(rhs);

			// Assert
			GTestExpectEq(expected, actual);

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

			// Auto key adaptation uses for able to test different type of archives.
			static const auto key = BitSerializer::Convert::To<typename TArchive::key_type>("Member_")
				+ BitSerializer::Convert::To<typename TArchive::key_type>(I);
			archive << BitSerializer::MakeKeyValue(key, member);

			// Next
			Serialize<TArchive, I + 1>(archive);
		}
	}
};

//-----------------------------------------------------------------------------
template <typename T, size_t ArraySize = 7>
class TestClassWithSubArray
{
public:
	static void BuildFixture(TestClassWithSubArray<T>& fixture) {
		::BuildFixture(fixture.mTestArray);
	}

	void Assert(const TestClassWithSubArray<T>& rhs) const {
		for (size_t i = 0; i < ArraySize; i++) {
			ASSERT_EQ(mTestArray[i], rhs.mTestArray[i]);
		}
	}

	template <class TArchive>
	void Serialize(TArchive& archive) {
		archive << BitSerializer::MakeAutoKeyValue(L"TestArray", mTestArray);
	}

private:
	T mTestArray[ArraySize];
};

//-----------------------------------------------------------------------------
template <typename T, size_t ArraySize1 = 3, size_t ArraySize2 = 5>
class TestClassWithSubTwoDimArray
{
public:
	static const size_t Array1stLevelSize = ArraySize1;
	static const size_t Array2stLevelSize = ArraySize2;

	static void BuildFixture(TestClassWithSubTwoDimArray& fixture) {
		::BuildFixture(fixture.mTestTwoDimArray);
	}

	void Assert(const TestClassWithSubTwoDimArray& rhs) const {
		for (size_t i = 0; i < ArraySize1; i++) {
			for (size_t c = 0; c < ArraySize2; c++) {
				GTestExpectEq(mTestTwoDimArray[i][c], rhs.mTestTwoDimArray[i][c]);
			}
		}
	}

	template <class TArchive>
	void Serialize(TArchive& archive) {
		archive << BitSerializer::MakeAutoKeyValue(L"TestTwoDimArray", mTestTwoDimArray);
	}

private:
	T mTestTwoDimArray[ArraySize1][ArraySize2];
};

//-----------------------------------------------------------------------------
template <typename TestType>
class TestClassForCheckValidation
{
public:
	static void BuildFixture(TestClassForCheckValidation<TestType>& fixture)
	{
		::BuildFixture(fixture.mExistSingleField);
		::BuildFixture(fixture.mExistArrayField);
	}

	void Assert() const
	{
		EXPECT_EQ(2, BitSerializer::Context.GetValidationErrors().size());
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::MakeAutoKeyValue(L"ExistSingleField", mExistSingleField, BitSerializer::Required());
		archive << BitSerializer::MakeAutoKeyValue(L"ExistArrayField", mExistArrayField, BitSerializer::Required());

		// Trying to load not exist fields
		if (archive.IsLoading())
		{
			TestType notExistSingleField{};
			TestType notExistArrayField[3];

			archive << BitSerializer::MakeAutoKeyValue(L"NotExistSingleField", notExistSingleField, BitSerializer::Required());
			archive << BitSerializer::MakeAutoKeyValue(L"NotExistArrayField", notExistArrayField, BitSerializer::Required());
		}
	}

private:
	TestType mExistSingleField;
	TestType mExistArrayField[3];
};

template <typename TestType>
class TestClassForCheckCompatibleTypes
{
public:
	static void BuildFixture(TestClassForCheckCompatibleTypes<TestType>& fixture)
	{
		::BuildFixture(fixture.mTestField);
	}

	void Assert() const
	{
		EXPECT_EQ(1, BitSerializer::Context.GetValidationErrors().size());
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::MakeAutoKeyValue("TestField", mTestField, BitSerializer::Required());
	}

private:
	TestType mTestField;
};

//-----------------------------------------------------------------------------
template <class ...Args>
class TestClassWithAttributes : public std::tuple<Args...>
{
public:
	TestClassWithAttributes() = default;
	TestClassWithAttributes(Args... args)
		: std::tuple<Args...>(args...)
	{
	}

	template<std::size_t I = 0>
	static void BuildFixture(TestClassWithAttributes& fixture)
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
	void Assert(const TestClassWithAttributes& rhs) const
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

			// Auto key adaptation uses for able to test different type of archives.
			static const auto attributeKey = BitSerializer::Convert::To<typename TArchive::key_type>("Attribute_")
				+ BitSerializer::Convert::To<typename TArchive::key_type>(I);
			archive << BitSerializer::MakeAttributeValue(attributeKey, member);

			// Next
			Serialize<TArchive, I + 1>(archive);
		}
	}
};
