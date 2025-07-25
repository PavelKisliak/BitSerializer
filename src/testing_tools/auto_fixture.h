﻿/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#if defined __has_include && __has_include(<version>)
#include <version>
#endif
#include <cstdlib>
#include <random>
#include <chrono>
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
#include <valarray>
#include <type_traits>
#include <tuple>
#include <filesystem>
#include <atomic>
#if defined(__cpp_lib_memory_resource)
#include <memory_resource>
#endif

#include "string_utils.h"
#include "bitserializer/convert.h"
#include "bitserializer/serialization_detail/bin_timestamp.h"

/**
 * @brief Checks whether a type has a static `BuildFixture()` method.
 */
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

/**
 * @brief Checks whether a type has an `Assert()` method.
 */
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

/**
 * @brief Builds a test fixture for class or union types using the static `BuildFixture()` method.
 *
 * If no such method exists, this will cause a static assertion failure.
 * Alternatively, a global `BuildFixture()` overload may be provided.
 *
 * @tparam T The class or union type.
 * @param value Reference to the object to initialize.
 */
template <typename T, std::enable_if_t<(std::is_class_v<T> || std::is_union_v<T>), int> = 0>
static void BuildFixture(T& value)
{
	constexpr auto hasBuildMethod = has_build_fixture_method_v<T>;
	static_assert(hasBuildMethod, "Your test class should implement static method BuildFixture(ClassType&).");

	if constexpr (hasBuildMethod) {
		T::BuildFixture(value);
	}
}

/**
 * @brief Builds a test fixture for integral types.
 *
 * @tparam T Integral type.
 * @param value Reference to the value to populate.
 */
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
static void BuildFixture(T& value)					{ value = static_cast<T>(std::rand()); }

[[maybe_unused]] static void BuildFixture(int64_t& value)			{ value = (static_cast<int64_t>(std::rand()) << std::numeric_limits<unsigned>::digits) + std::rand(); }
[[maybe_unused]] static void BuildFixture(uint64_t& value)			{ value = (static_cast<uint64_t>(std::rand()) << std::numeric_limits<unsigned>::digits) + std::rand(); }
[[maybe_unused]] static void BuildFixture(bool& value)				{ value = static_cast<bool>(std::rand() % 2); }
[[maybe_unused]] static void BuildFixture(float& value)				{ value = static_cast<float>((std::rand() % 1000) + 1) * 1.141592f; }
[[maybe_unused]] static void BuildFixture(double& value)			{ value = static_cast<double>((std::rand() % 100000) + 1) * 1.141592; }
[[maybe_unused]] static void BuildFixture(std::nullptr_t& value)	{ value = nullptr; }
[[maybe_unused]] static void BuildFixture(std::string& value)		{ value = UTF8("UTF-8 Тест_") + std::to_string(std::rand()); }
#if defined(__cpp_lib_memory_resource)
[[maybe_unused]] static void BuildFixture(std::pmr::string& value)	{ value = UTF8("UTF-8 Тест_") + std::to_string(std::rand()); }
#endif
#if defined(__cpp_lib_char8_t)
[[maybe_unused]] static void BuildFixture(std::u8string& value)		{ value = u8"U8-string Тест_"; }// +BitSerializer::Convert::To<std::u8string>(std::rand()); }
#endif
[[maybe_unused]] static void BuildFixture(std::wstring& value)		{ value = L"WString Тест_" + std::to_wstring(std::rand()); }
[[maybe_unused]] static void BuildFixture(std::u16string& value)	{ value = u"UTF-16 Тест_" + BitSerializer::Convert::To<std::u16string>(std::rand()); }
[[maybe_unused]] static void BuildFixture(std::u32string& value)	{ value = U"UTF-32 Тест_" + BitSerializer::Convert::To<std::u32string>(std::rand()); }

[[maybe_unused]] static void BuildFixture(std::byte& value)
{
	value = static_cast<std::byte>(std::rand() % (std::numeric_limits<unsigned char>::max)());
}

/**
 * @brief Builds a test fixture for enum types using registered values.
 *
 * @tparam T Enum type.
 * @param value Reference to the enum value to populate.
 */
template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
[[maybe_unused]] static void BuildFixture(T& value)
{
	const auto randIndex = std::rand() % BitSerializer::Convert::Detail::EnumRegistry<T>::size();
	value = (BitSerializer::Convert::Detail::EnumRegistry<T>::cbegin() + randIndex)->Value;
}

/**
 * @brief Builds a test fixture for atomic types.
 *
 * @tparam T Atomic type.
 * @param value Reference to the atomic variable to populate.
 */
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[maybe_unused]] static void BuildFixture(std::atomic<T>& value)
{
	T temp;
	::BuildFixture<T>(temp);
	value = temp;
}

/**
 * @brief Builds a test fixture for C-style arrays.
 *
 * @tparam TValue Element type.
 * @tparam ArraySize Number of elements in the array.
 * @param arr Reference to the array.
 */
template <typename TValue, size_t ArraySize>
[[maybe_unused]] static void BuildFixture(TValue(&arr)[ArraySize])
{
	static_assert(ArraySize != 0);

	if constexpr (std::is_arithmetic_v<TValue>)
	{
		// Using min/max values as generated elements in the array
		arr[0] = std::numeric_limits<TValue>::lowest();
		if constexpr (ArraySize > 1)
		{
			for (size_t i = 1; i < (ArraySize - 1); i++) {
				BuildFixture(arr[i]);
			}
			arr[ArraySize - 1] = (std::numeric_limits<TValue>::max)();
		}
	}
	else
	{
		for (size_t i = 0; i < ArraySize; i++) {
			BuildFixture(arr[i]);
		}
	}
}

/**
 * @brief Builds a test fixture for `std::pair` value.
 *
 * @param pair Reference to the `std::pair`.
 */
template <typename TKey, typename TValue>
[[maybe_unused]] static void BuildFixture(std::pair<TKey, TValue>& pair)
{
	BuildFixture(pair.first);
	BuildFixture(pair.second);
}

/**
 * @brief Builds a test fixture for `std::tuple` value.
 *
 * @param value Reference to the `std::tuple`.
 */
template <typename ...TArgs>
[[maybe_unused]] static void BuildFixture(std::tuple<TArgs...>& value)
{
	std::apply([](auto&&... args) {
		((BuildFixture(args)), ...);
	}, value);
}

/**
 * @brief Builds a test fixture for `std::optional<TValue>` value.
 *
 * @param optionalValue Reference to the `std::optional<TValue>`.
 */
template <typename TValue>
[[maybe_unused]] static void BuildFixture(std::optional<TValue>& optionalValue)
{
	optionalValue = TValue();
	BuildFixture(optionalValue.value());
}

/**
 * @brief Builds a test fixture for `std::chrono::time_point`.
 *
 * @param timePoint Reference to the `std::chrono::time_point`.
 */
template <typename TClock, typename TDuration>
[[maybe_unused]] static void BuildFixture(std::chrono::time_point<TClock, TDuration>& timePoint)
{
	constexpr auto tpMaxSec = std::chrono::time_point_cast<std::chrono::seconds>((std::chrono::time_point<TClock, TDuration>::max)())
		.time_since_epoch().count();
	constexpr auto tpMinSec = std::chrono::time_point_cast<std::chrono::seconds>((std::chrono::time_point<TClock, TDuration>::min)())
		.time_since_epoch().count();

	constexpr int64_t time_0000_01_01T00_00_00 = -62167219200;
	constexpr int64_t time_9999_12_31T23_59_59 = 253402300799;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int64_t> distr(
		(std::max)(tpMinSec, time_0000_01_01T00_00_00),
		(std::min)(tpMaxSec, time_9999_12_31T23_59_59));

	timePoint = std::chrono::time_point<TClock, TDuration>(std::chrono::duration_cast<TDuration>(std::chrono::seconds(distr(gen))));
}

/**
 * @brief Builds a test fixture for `std::chrono::duration`.
 *
 * @param duration Reference to the `std::chrono::duration`.
 */
template <typename TRep, typename TPeriod>
[[maybe_unused]] static void BuildFixture(std::chrono::duration<TRep, TPeriod>& duration)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	using TCommon = std::common_type_t<TRep, intmax_t>;
	std::uniform_int_distribution<TCommon> distr((std::numeric_limits<TRep>::min)(), (std::numeric_limits<TRep>::max)());

	duration = std::chrono::duration<TRep, TPeriod>(distr(gen));
}

namespace std
{
	/**
	 * @brief Specialization of `std::numeric_limits<T>` for `CBinTimestamp`.
	 */
	template<>
	class numeric_limits<BitSerializer::Detail::CBinTimestamp>
	{
	public:
		static BitSerializer::Detail::CBinTimestamp (min)()
		{
			return BitSerializer::Detail::CBinTimestamp((std::numeric_limits<int64_t>::min)(), (std::numeric_limits<int32_t>::min)());
		}

		static BitSerializer::Detail::CBinTimestamp (max)()
		{
			return BitSerializer::Detail::CBinTimestamp((std::numeric_limits<int64_t>::max)(), (std::numeric_limits<int32_t>::max)());
		}
	};
}

/**
 * @brief Builds a test fixture for `CBinTimestamp`.
 *
 * @param timestamp Reference to the `CBinTimestamp`.
 */
[[maybe_unused]] static void BuildFixture(BitSerializer::Detail::CBinTimestamp& timestamp)
{
	BuildFixture(timestamp.Seconds);
	BuildFixture(timestamp.Nanoseconds);
}

/**
 * @brief Builds a test fixture for `std::unique_ptr<TValue>`.
 *
 * @param uniquePtr Reference to the `std::unique_ptr<TValue>`.
 */
template <typename TValue>
[[maybe_unused]] static void BuildFixture(std::unique_ptr<TValue>& uniquePtr)
{
	uniquePtr = std::make_unique<TValue>();
	BuildFixture(*uniquePtr);
}

/**
 * @brief Builds a test fixture for `std::shared_ptr<TValue>`.
 *
 * @param sharedPtr Reference to the `std::shared_ptr<TValue>`.
 */
template <typename TValue>
[[maybe_unused]] static void BuildFixture(std::shared_ptr<TValue>& sharedPtr)
{
	sharedPtr = std::make_shared<TValue>();
	BuildFixture(*sharedPtr);
}

/**
 * @brief Builds a test fixture - overloaded variant with return value (can't be applied to c-array types).
 *
 * @return The generated fixture
 */
template <typename T, std::enable_if_t<!std::is_array_v<T>, int> = 0>
[[maybe_unused]] static T BuildFixture()
{
	T fixture{};
	BuildFixture(fixture);
	return fixture;
}

/**
 * @brief Builds a test fixture for `std::filesystem::path`.
 *
 * @param path Reference to the `std::filesystem::path`.
 */
[[maybe_unused]] static void BuildFixture(std::filesystem::path& path)
{
	path = std::filesystem::temp_directory_path() / (BuildFixture<std::string>() + ".txt");
}

/**
 * @brief Builds a test fixture for `std::array`.
 *
 * @param cont Reference to the `std::array`.
 */
template <typename T, size_t Size>
[[maybe_unused]] static void BuildFixture(std::array<T, Size>& cont)
{
	for (size_t i = 0; i < Size; i++) {
		BuildFixture(cont[i]);
	}
}

template <typename T, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::vector<T, TAllocator>& cont)
{
	static constexpr int size = 7;
	cont.resize(size);
	for (auto& elem : cont) {
		BuildFixture(elem);
	}
}

[[maybe_unused]] static void BuildFixture(std::vector<bool>& cont)
{
	static constexpr int size = 7;
	cont.resize(size);

	for (size_t i = 0; i < size; i++) {
		bool elem{};
		BuildFixture(elem);
		cont[i] = elem;
	}
}

template <typename T, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::deque<T, TAllocator>& cont)
{
	static constexpr int size = 7;
	cont.resize(size);
	for (auto& elem : cont) {
		BuildFixture(elem);
	}
}

template <size_t Size>
[[maybe_unused]] static void BuildFixture(std::bitset<Size>& cont)
{
	for (size_t i = 0; i < Size; i++) {
		cont.set(i, BuildFixture<bool>());
	}
}

template <typename T, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::list<T, TAllocator>& cont)
{
	static constexpr int size = 7;
	cont.resize(size);
	for (auto& elem : cont) {
		BuildFixture(elem);
	}
}

template <typename T, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::forward_list<T, TAllocator>& cont)
{
	static constexpr int size = 7;
	cont.resize(size);
	for (auto& elem : cont) {
		BuildFixture(elem);
	}
}

template <typename T>
[[maybe_unused]] static void BuildFixture(std::queue<T>& cont)
{
	static constexpr int size = 7;
	for (size_t i = 0; i < size; i++) {
		cont.push(BuildFixture<T>());
	}
}

template <typename T>
[[maybe_unused]] static void BuildFixture(std::priority_queue<T>& cont)
{
	static constexpr int size = 7;
	for (size_t i = 0; i < size; i++) {
		cont.push(BuildFixture<T>());
	}
}

template <typename T>
[[maybe_unused]] static void BuildFixture(std::stack<T>& cont)
{
	static constexpr int size = 7;
	for (size_t i = 0; i < size; i++) {
		cont.push(BuildFixture<T>());
	}
}

template <typename T, typename TComparer, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::set<T, TComparer, TAllocator>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	for (size_t i = 0; i < size; i++) {
		T element;
		BuildFixture(element);
		cont.emplace(std::move(element));
	}
}

template <typename T, typename TComparer, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::multiset<T, TComparer, TAllocator>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	T element;
	for (size_t i = 0; i < size; i++)
	{
		// Duplicated element
		if (i % 2 == 0) {
			BuildFixture(element);
		}
		cont.insert(element);
	}
}

template <typename T, typename THasher, typename TKeyEq, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::unordered_set<T, THasher, TKeyEq, TAllocator>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	for (size_t i = 0; i < size; i++) {
		T element;
		BuildFixture(element);
		cont.emplace(std::move(element));
	}
}

template <typename T, typename THasher, typename TKeyEq, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::unordered_multiset<T, THasher, TKeyEq, TAllocator>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	T element;
	for (size_t i = 0; i < size; i++)
	{
		// Duplicated element
		if (i % 2 == 0) {
			BuildFixture(element);
		}
		cont.insert(element);
	}
}

template <typename TKey, typename TValue, typename TComparer, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::map<TKey, TValue, TComparer, TAllocator>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	for (size_t i = 0; i < size; i++) {
		TValue value;
		BuildFixture(value);
		cont.emplace(BuildFixture<TKey>(), std::move(value));
	}
}

template <typename TKey, typename TValue, typename TComparer, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::multimap<TKey, TValue, TComparer, TAllocator>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	TKey key;
	for (size_t i = 0; i < size; i++)
	{
		if (i % 2 == 0) {
			BuildFixture(key);
		}
		TValue value;
		BuildFixture(value);
		cont.emplace(key, std::move(value));
	}
}

template <typename TKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::unordered_map<TKey, TValue, THasher, TKeyEq, TAllocator>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	for (size_t i = 0; i < size; i++) {
		TValue value;
		BuildFixture(value);
		cont.emplace(BuildFixture<TKey>(), std::move(value));
	}
}

template <typename TKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
[[maybe_unused]] static void BuildFixture(std::unordered_multimap<TKey, TValue, THasher, TKeyEq, TAllocator>& cont)
{
	static constexpr int size = 7;

	cont.clear();
	TKey key;
	for (size_t i = 0; i < size; i++)
	{
		if (i % 2 == 0) {
			BuildFixture(key);
		}
		TValue value;
		BuildFixture(value);
		cont.emplace(key, std::move(value));
	}
}

template <typename T>
[[maybe_unused]] static void BuildFixture(std::valarray<T>& cont)
{
	static constexpr int size = 7;
	cont.resize(size);
	for (size_t i = 0; i < size; i++) {
		BuildFixture(cont[i]);
	}
}
