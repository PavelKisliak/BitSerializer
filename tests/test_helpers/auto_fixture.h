/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <deque>
#include <map>
#include <set>
#include <list>
#include <forward_list>
#include <type_traits>
#include "bitserializer/string_conversion.h"

/// <summary>
/// Checks that the class has BuildFixture() method.
/// </summary>
template <typename T>
struct has_buld_fixture_method
{
private:
	template <typename U>
	static decltype(T::BuildFixture(std::declval<T&>()), void(), std::true_type()) test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_buld_fixture_method_v = has_buld_fixture_method<T>::value;

/// <summary>
/// Builds the test fixture for classes (they must have static method BuildFixture()).
/// As an alternative you can implement method BuildFixture() as global.
/// </summary>
template <typename T, std::enable_if_t<std::is_class_v<T>, int> = 0>
static void BuildFixture(T& value)
{
	constexpr auto hasBuldMethod = has_buld_fixture_method_v<T>;
	static_assert(hasBuldMethod, "Your test class should implements static method BuildFixture(ClassType&).");

	if constexpr (hasBuldMethod) {
		T::BuildFixture(value);
	}
}

/// <summary>
/// Builds the test fixture for enum types (they must be registered in conversion system).
/// </summary>
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
static void BuildFixture(T& value)					{ value = static_cast<T>(std::rand()); }

static void BuildFixture(int64_t& value)			{ value = (static_cast<int64_t>(std::rand()) << 32) + std::rand(); }
static void BuildFixture(uint64_t& value)			{ value = (static_cast<uint64_t>(std::rand()) << 32) + std::rand(); }
static void BuildFixture(bool& value)				{ value = static_cast<bool>(std::rand() % 2); }
static void BuildFixture(float& value)				{ value = static_cast<float>(std::rand() / 10.0f); }
static void BuildFixture(double& value)				{ value = static_cast<double>(std::rand() / 1000.0); }
static void BuildFixture(std::string& value)		{ value = std::to_string(std::rand()); }
static void BuildFixture(std::wstring& value)		{ value = std::to_wstring(std::rand()); }

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
static void BuildFixture(T& value)
{
	const auto& descriptors = BitSerializer::Convert::Detail::ConvertEnum::GetDescriptors<T>();
	value = descriptors[std::rand() % descriptors.size()].GetEnum();
}

/// <summary>
/// Builds the test fixture for array of types.
/// </summary>
/// <param name="arr">The arr.</param>
/// <returns></returns>
template <typename TValue, size_t ArraySize>
static void BuildFixture(TValue(&arr)[ArraySize])
{
	for (size_t i = 0; i < ArraySize; i++) {
		BuildFixture(arr[i]);
	}
}

template <typename TKey, typename TValue>
static void BuildFixture(std::pair<TKey, TValue>& pair)
{
	BuildFixture(pair.first);
	BuildFixture(pair.second);
}

/// <summary>
/// Build the test fixture - overloaded variant with return value (can't be applied to c-array types).
/// </summary>
/// <returns></returns>
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
		cont.emplace(std::move(key), std::move(value));
	}
}
