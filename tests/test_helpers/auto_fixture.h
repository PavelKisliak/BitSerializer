/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdlib>
#include <limits>
#include <optional>
#include <memory>
#include <deque>
#include <bitset>
#include <list>
#include <forward_list>
#include <queue>
#include <stack>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <type_traits>
#include "bitserializer/convert.h"

/// <summary>
/// Checks that the class has static BuildFixture() method.
/// </summary>
template <typename T>
struct has_build_fixture_method
{
private:
	template <typename U>
	static decltype(U::BuildFixture(std::declval<U&>()), std::true_type()) test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_build_fixture_method_v = has_build_fixture_method<T>::value;

/// <summary>
/// Checks that the class has Assert() method.
/// </summary>
template <typename T>
struct has_assert_method
{
private:
	template <typename U>
	static decltype(std::declval<U>().Assert(std::declval<const U&>()), std::true_type()) test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_assert_method_v = has_assert_method<T>::value;

/// <summary>
/// Builds the test fixture for classes (they must have static method BuildFixture()).
/// As an alternative you can implement method BuildFixture() as global.
/// </summary>
template <typename T, std::enable_if_t<(std::is_class_v<T> || std::is_union_v<T>), int> = 0>
static void BuildFixture(T& value)
{
	constexpr auto hasBuildMethod = has_build_fixture_method_v<T>;
	static_assert(hasBuildMethod, "Your test class should implement static method BuildFixture(ClassType&).");

	if constexpr (hasBuildMethod) {
		T::BuildFixture(value);
	}
}

/// <summary>
/// Builds the test fixture for enum types (they must be registered in conversion system).
/// </summary>
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
static void BuildFixture(T& value)					{ value = static_cast<T>(std::rand()); }

static void BuildFixture(int64_t& value)			{ value = (static_cast<int64_t>(std::rand()) << std::numeric_limits<unsigned>::digits) + std::rand(); }
static void BuildFixture(uint64_t& value)			{ value = (static_cast<uint64_t>(std::rand()) << std::numeric_limits<unsigned>::digits) + std::rand(); }
static void BuildFixture(bool& value)				{ value = static_cast<bool>(std::rand() % 2); }
static void BuildFixture(float& value)				{ value = static_cast<float>(std::rand() / 10.0f); }
static void BuildFixture(double& value)				{ value = static_cast<double>(std::rand() / 1000.0); }
static void BuildFixture(std::nullptr_t& value)		{ value = nullptr; }
static void BuildFixture(std::string& value)		{ value = u8"UTF-8 Тест_" + std::to_string(std::rand()); }
static void BuildFixture(std::wstring& value)		{ value = L"WString Тест_" + std::to_wstring(std::rand()); }
static void BuildFixture(std::u16string& value)		{ value = u"UTF-16 Тест_" + BitSerializer::Convert::To<std::u16string>(std::rand()); }
static void BuildFixture(std::u32string& value)		{ value = U"UTF-32 Тест_" + BitSerializer::Convert::To<std::u32string>(std::rand()); }

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
static void BuildFixture(T& value)
{
	const auto& descriptors = BitSerializer::Convert::Detail::ConvertEnum::GetDescriptors<T>();
	value = descriptors[std::rand() % descriptors.size()].GetEnum();
}

/// <summary>
/// Builds the test fixture for array of types.
/// </summary>
/// <param name="arr">The array.</param>
template <typename TValue, size_t ArraySize>
static void BuildFixture(TValue(&arr)[ArraySize])
{
	static_assert(ArraySize != 0);

	if constexpr (std::is_arithmetic_v<TValue>)
	{
		// Using min/max values as generated elements in the array
		if constexpr (ArraySize > 1)
		{
			arr[0] = std::numeric_limits<TValue>::min();
			for (size_t i = 1; i < (ArraySize - 1); i++) {
				BuildFixture(arr[i]);
			}
		}
		arr[ArraySize-1] = std::numeric_limits<TValue>::max();
	}
	else
	{
		for (size_t i = 0; i < ArraySize; i++) {
			BuildFixture(arr[i]);
		}
	}
}

/// <summary>
/// Builds the test fixture for std::pair value.
/// </summary>
/// <param name="pair">The pair.</param>
template <typename TKey, typename TValue>
static void BuildFixture(std::pair<TKey, TValue>& pair)
{
	BuildFixture(pair.first);
	BuildFixture(pair.second);
}

/// <summary>
/// Builds the test fixture for std::optional value.
/// </summary>
/// <param name="optionalValue">The optional value.</param>
template <typename TValue>
static void BuildFixture(std::optional<TValue>& optionalValue)
{
	optionalValue = TValue();
	BuildFixture(optionalValue.value());
}

/// <summary>
/// Builds the test fixture for std::unique_ptr value.
/// </summary>
/// <param name="uniquePtr">The reference to unique pointer.</param>
template <typename TValue>
static void BuildFixture(std::unique_ptr<TValue>& uniquePtr)
{
	uniquePtr = std::make_unique<TValue>();
	BuildFixture(*uniquePtr);
}

/// <summary>
/// Builds the test fixture for std::shared_ptr value.
/// </summary>
/// <param name="sharedPtr">The reference to shared pointer.</param>
template <typename TValue>
static void BuildFixture(std::shared_ptr<TValue>& sharedPtr)
{
	sharedPtr = std::make_shared<TValue>();
	BuildFixture(*sharedPtr);
}

/// <summary>
/// Build the test fixture - overloaded variant with return value (can't be applied to c-array types).
/// </summary>
/// <returns>The generated fixture</returns>
template <typename T, std::enable_if_t<!std::is_array_v<T>, int> = 0>
static T BuildFixture()
{
	T fixture;
	BuildFixture(fixture);
	return fixture;
}

template <typename T, size_t Size>
static void BuildFixture(std::array<T, Size>& cont)
{
	for (size_t i = 0; i < Size; i++) {
		BuildFixture(cont[i]);
	}
}

template <typename T>
static void BuildFixture(std::vector<T>& cont)
{
	static constexpr int size = 7;
	cont.resize(size);
	for (auto& elem : cont) {
		BuildFixture(elem);
	}
}

static void BuildFixture(std::vector<bool>& cont)
{
	static constexpr int size = 7;
	cont.resize(size);

	for (size_t i = 0; i < size; i++) {
		bool elem;
		BuildFixture(elem);
		cont[i] = elem;
	}
}

template <typename T>
static void BuildFixture(std::deque<T>& cont)
{
	static constexpr int size = 7;
	cont.resize(size);
	for (auto& elem : cont) {
		BuildFixture(elem);
	}
}

template <size_t Size>
static void BuildFixture(std::bitset<Size>& cont)
{
	for (size_t i = 0; i < Size; i++) {
		cont.set(i, BuildFixture<bool>());
	}
}

template <typename T>
static void BuildFixture(std::list<T>& cont)
{
	static constexpr int size = 7;
	cont.resize(size);
	for (auto& elem : cont) {
		BuildFixture(elem);
	}
}

template <typename T>
static void BuildFixture(std::forward_list<T>& cont)
{
	static constexpr int size = 7;
	cont.resize(size);
	for (auto& elem : cont) {
		BuildFixture(elem);
	}
}

template <typename T>
static void BuildFixture(std::queue<T>& cont)
{
	static constexpr int size = 7;
	for (size_t i = 0; i < size; i++) {
		cont.push(BuildFixture<T>());
	}
}

template <typename T>
static void BuildFixture(std::priority_queue<T>& cont)
{
	static constexpr int size = 7;
	for (size_t i = 0; i < size; i++) {
		cont.push(BuildFixture<T>());
	}
}

template <typename T>
static void BuildFixture(std::stack<T>& cont)
{
	static constexpr int size = 7;
	for (size_t i = 0; i < size; i++) {
		cont.push(BuildFixture<T>());
	}
}

template <typename T>
static void BuildFixture(std::set<T>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	for (size_t i = 0; i < size; i++) {
		T element;
		BuildFixture(element);
		cont.emplace(std::move(element));
	}
}

template <typename T>
static void BuildFixture(std::unordered_set<T>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	for (size_t i = 0; i < size; i++) {
		T element;
		BuildFixture(element);
		cont.emplace(std::move(element));
	}
}

template <typename T>
static void BuildFixture(std::multiset<T>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	T element;
	for (size_t i = 0; i < size; i++)
	{
		// Duplicated element
		if (i % 2 == 0)
			BuildFixture(element);
		cont.insert(element);
	}
}

template <typename TKey, typename TValue>
static void BuildFixture(std::map<TKey, TValue>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	for (size_t i = 0; i < size; i++) {
		TValue value;
		BuildFixture(value);
		cont.emplace(BuildFixture<TKey>(), std::move(value));
	}
}

template <typename TKey, typename TValue>
static void BuildFixture(std::unordered_map<TKey, TValue>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	for (size_t i = 0; i < size; i++) {
		TValue value;
		BuildFixture(value);
		cont.emplace(BuildFixture<TKey>(), std::move(value));
	}
}

template <typename TKey, typename TValue>
static void BuildFixture(std::multimap<TKey, TValue>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	TKey key;
	for (size_t i = 0; i < size; i++)
	{
		if (i % 2 == 0)
			BuildFixture(key);
		TValue value;
		BuildFixture(value);
		cont.emplace(key, std::move(value));
	}
}
