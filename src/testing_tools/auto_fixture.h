/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
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
#include <type_traits>
#include <tuple>
#include <filesystem>
#include "string_utils.h"
#include "bitserializer/convert.h"
#include "bitserializer/serialization_detail/archive_base.h"

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
static void BuildFixture(float& value)				{ value = static_cast<float>(std::rand() % 1000 + 1) * 1.141592f; }
static void BuildFixture(double& value)				{ value = static_cast<double>(std::rand() % 100000 + 1) * 1.141592; }
static void BuildFixture(std::nullptr_t& value)		{ value = nullptr; }
static void BuildFixture(std::string& value)		{ value = UTF8("UTF-8 Тест_") + std::to_string(std::rand()); }
static void BuildFixture(std::wstring& value)		{ value = L"WString Тест_" + std::to_wstring(std::rand()); }
static void BuildFixture(std::u16string& value)		{ value = u"UTF-16 Тест_" + BitSerializer::Convert::To<std::u16string>(std::rand()); }
static void BuildFixture(std::u32string& value)		{ value = U"UTF-32 Тест_" + BitSerializer::Convert::To<std::u32string>(std::rand()); }

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
static void BuildFixture(T& value)
{
	const auto randIndex = std::rand() % BitSerializer::Convert::Detail::EnumRegistry<T>::size();
	value = (BitSerializer::Convert::Detail::EnumRegistry<T>::cbegin() + randIndex)->Value;
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
		if constexpr (ArraySize > 2)
		{
			arr[0] = std::numeric_limits<TValue>::lowest();
			arr[1] = 1;
			for (size_t i = 2; i < (ArraySize - 1); i++) {
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
/// Builds the test fixture for std::tuple value.
/// </summary>
/// <param name="value">The tuple.</param>
template <typename ...TArgs>
static void BuildFixture(std::tuple<TArgs...>& value)
{
	std::apply([](auto&&... args) {
		((BuildFixture(args)), ...);
	}, value);
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
/// Builds the test fixture for std::chrono::time_point
/// </summary>
/// <param name="timePoint">The reference to time point.</param>
template <typename TClock, typename TDuration>
static void BuildFixture(std::chrono::time_point<TClock, TDuration>& timePoint)
{
	constexpr auto tpMaxSec = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::time_point<TClock, TDuration>::max())
		.time_since_epoch().count();
	constexpr auto tpMinSec = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::time_point<TClock, TDuration>::min())
		.time_since_epoch().count();

	constexpr int64_t time_0000_01_01T00_00_00 = -62167219200;
	constexpr int64_t time_9999_12_31T23_59_59 = 253402300799;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int64_t> distr(
		std::max(tpMinSec, time_0000_01_01T00_00_00),
		std::min(tpMaxSec, time_9999_12_31T23_59_59));

	timePoint = std::chrono::time_point<TClock, TDuration>(std::chrono::seconds(distr(gen)));
}

/// <summary>
/// Builds the test fixture for std::chrono::duration
/// </summary>
/// <param name="duration">The reference to duration.</param>
template <typename TRep, typename TPeriod>
static void BuildFixture(std::chrono::duration<TRep, TPeriod>& duration)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution distr(std::numeric_limits<TRep>::min(), std::numeric_limits<TRep>::max());

	duration = std::chrono::duration<TRep, TPeriod>(distr(gen));
}

namespace std
{
	/// <summary>
	/// Specialization of `std::numeric_limits<T>` for `CBinTimestamp`.
	/// </summary>
	template<>
	class numeric_limits<BitSerializer::Detail::CBinTimestamp>
	{
	public:
		static BitSerializer::Detail::CBinTimestamp min()
		{
			return BitSerializer::Detail::CBinTimestamp(std::numeric_limits<int64_t>::min(), std::numeric_limits<uint32_t>::min());
		}

		static BitSerializer::Detail::CBinTimestamp max()
		{
			return BitSerializer::Detail::CBinTimestamp(std::numeric_limits<int64_t>::max(), std::numeric_limits<uint32_t>::max());
		}
	};
}

/// <summary>
/// Builds the test fixture for CBinTimestamp
/// </summary>
/// <param name="timestamp">The reference to binary timestamp.</param>
static void BuildFixture(BitSerializer::Detail::CBinTimestamp& timestamp)
{
	BuildFixture(timestamp.Seconds);
	BuildFixture(timestamp.Nanoseconds);
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

/// <summary>
/// Builds the test fixture for std::filesystem::path value.
/// </summary>
/// <param name="path">The reference to path.</param>
static void BuildFixture(std::filesystem::path& path)
{
	path = std::filesystem::temp_directory_path() / (BuildFixture<std::string>() + ".txt");
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
