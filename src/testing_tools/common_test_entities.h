/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
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

REGISTER_ENUM(TestEnum, {
	{ TestEnum::One,	"One" },
	{ TestEnum::Two,	"Two" },
	{ TestEnum::Three,	"Three" },
	{ TestEnum::Four,	"Four" },
	{ TestEnum::Five,	"Five" }
})
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
		archive << BitSerializer::KeyValue("value", mIntValue);
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
		archive << BitSerializer::KeyValue("x", x);
		archive << BitSerializer::KeyValue("y", y);
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

		archive << BitSerializer::KeyValue(L"TestUInt32", mTestUInt32);
		archive << BitSerializer::KeyValue(L"TestUInt64", mTestUInt64);
	}

private:
	uint32_t mTestUInt32 = 0u;
	uint64_t mTestUInt64 = 0u;
};

//-----------------------------------------------------------------------------
template <typename T, bool RequiredValidator = false>
class TestClassWithSubType
{
public:
	using value_type = T;
	static constexpr char KeyName[] = "TestValue";

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
		if constexpr (RequiredValidator == true) {
			archive << BitSerializer::KeyValue(KeyName, mTestValue, BitSerializer::Required());
		}
		else {
			archive << BitSerializer::KeyValue(KeyName, mTestValue);
		}
	}

	[[nodiscard]] const T& GetValue() const { return mTestValue; }

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
			// Get class member
			decltype(auto) expected = std::get<I>(*this);
			decltype(auto) actual = std::get<I>(rhs);

			// Assert
			GTestExpectEq(expected, actual);

			// Next
			Assert<I + 1>(rhs);
		}
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		SerializeImpl(archive);
	}

	TestClassWithSubTypes<Args...>& WithRequired()
	{
		mRequired = true;
		return *this;
	}

protected:
	template <class TArchive, std::size_t Index = 0, bool Reverse=false>
	void SerializeImpl(TArchive& archive)
	{
		if constexpr (Index >= sizeof...(Args))
			return;
		else
		{
			decltype(auto) member = std::get<Index>(*this);

			static const auto key = "Member_" + BitSerializer::Convert::ToString(Index);
			if (mRequired)
			{
				archive << BitSerializer::KeyValue(key, member, BitSerializer::Required());
			}
			else
			{
				archive << BitSerializer::KeyValue(key, member);
			}

			// Serialize next value
			if constexpr (Reverse)
			{
				SerializeImpl<TArchive, Index - 1, Reverse>(archive);
			}
			else
			{
				SerializeImpl<TArchive, Index + 1>(archive);
			}
		}
	}

	bool mRequired = false;
};

//-----------------------------------------------------------------------------

template <class ...Args>
class TestClassWithReverseLoad : public TestClassWithSubTypes<Args...>
{
public:
	TestClassWithReverseLoad() = default;
	TestClassWithReverseLoad(Args... args)
		: std::tuple<Args...>(args...)
	{ }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		if (archive.IsLoading())
		{
			if constexpr (sizeof...(Args) > 0) {
				TestClassWithSubTypes<Args...>::template SerializeImpl<TArchive, sizeof...(Args) - 1, true>(archive);
			}
		}
		else {
			TestClassWithSubTypes<Args...>::SerializeImpl(archive);
		}
	}
};

//-----------------------------------------------------------------------------
template <typename T>
class TestClassWithCustomKey
{
public:
	static void BuildFixture(TestClassWithCustomKey& fixture)
	{
		::BuildFixture(fixture.minValue.second);
		::BuildFixture(fixture.maxValue.second);
	}

	void Assert(const TestClassWithCustomKey& rhs) const
	{
		EXPECT_EQ(minValue.second, rhs.minValue.second);
		EXPECT_EQ(maxValue.second, rhs.maxValue.second);
	}

	TestClassWithCustomKey() = default;

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::KeyValue(minValue.first, minValue.second);
		archive << BitSerializer::KeyValue(maxValue.first, maxValue.second);
	}

	std::pair<T, int> minValue = { std::numeric_limits<T>::min(), {} };
	std::pair<T, int> maxValue = { std::numeric_limits<T>::max(), {} };
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
		archive << BitSerializer::KeyValue(L"TestArray", mTestArray);
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
		archive << BitSerializer::KeyValue(L"TestTwoDimArray", mTestTwoDimArray);
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
		::BuildFixture(fixture.mExistField);
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::KeyValue("ExistField", mExistField, BitSerializer::Required());

		// Trying to load not existing field
		if constexpr (TArchive::IsLoading())
		{
			TestType notExistField{};
			archive << BitSerializer::KeyValue(L"NotExistingField", notExistField, BitSerializer::Required());
		}
	}

private:
	TestType mExistField;
};

template <typename TestType>
class TestClassForCheckCompatibleTypes
{
public:
	static void BuildFixture(TestClassForCheckCompatibleTypes<TestType>& fixture)
	{
		::BuildFixture(fixture.mTestField);
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::KeyValue("TestField", mTestField, BitSerializer::Required());
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
			archive << BitSerializer::AttributeValue(attributeKey, member);

			// Next
			Serialize<TArchive, I + 1>(archive);
		}
	}
};
