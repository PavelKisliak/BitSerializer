/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <deque>
#include <map>
#include <list>
#include <forward_list>
#include "gtest/gtest.h"
#include "bitserializer/string_conversion.h"

/// <summary>
/// Builds the test fixture (classes must have static method GetTestFixture()).
/// </summary>
/// <returns></returns>
template <typename T, std::enable_if_t<std::is_class_v<T>, int> = 0>
static void BuildFixture(T& value)					{ T::BuildTestFixture(value); }

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
static void BuildFixture(T& value)					{ value = static_cast<T>(std::rand()); }

static void BuildFixture(int64_t& value)			{ value = static_cast<int64_t>(std::rand()) * std::rand(); }
static void BuildFixture(uint64_t& value)			{ value = static_cast<uint64_t>(std::rand()) * std::rand(); }
static void BuildFixture(bool& value)				{ value = static_cast<bool>(std::rand() % 2); }
static void BuildFixture(float& value)				{ value = static_cast<float>(std::rand() / (std::rand() + 1)); }
static void BuildFixture(double& value)				{ value = static_cast<double>(std::rand() / (std::rand() + 1)); }
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
