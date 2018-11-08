/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "media_archive_base.h"
#include "object_traits.h"

namespace BitSerializer {

/// <summary>
/// Checks that the type is inherited from ArchiveScope.
/// </summary>
template <typename T>
struct is_archive_scope
{
	constexpr static bool value = std::is_base_of<ArchiveScope<SerializeMode::Load>, T>::value ||
		std::is_base_of<ArchiveScope<SerializeMode::Save>, T>::value;
};

template <typename T>
constexpr bool is_archive_scope_v = is_archive_scope<T>::value;

//------------------------------------------------------------------------------

/// <summary>
/// Checks that archive is support required input data type (like strings, streams, binary data).
/// </summary>
template <typename TArchive, typename TInput>
struct is_archive_support_input_data_type
{
	constexpr static bool value = is_input_stream_v<TInput>
		? std::is_constructible_v<TArchive, TInput&>
		: std::is_constructible_v<TArchive, const TInput&>;
};

template <typename TArchive, typename TInput>
constexpr bool is_archive_support_input_data_type_v = is_archive_support_input_data_type<TArchive, TInput>::value;

//------------------------------------------------------------------------------

/// <summary>
/// Checks that archive is support required output data type (like strings, binary data).
/// </summary>
template <typename TArchive, typename TOutput>
struct is_archive_support_output_data_type
{
	constexpr static bool value = std::is_constructible_v<TArchive, TOutput&>;
};

template <typename TArchive, typename TOutput>
constexpr bool is_archive_support_output_data_type_v = is_archive_support_output_data_type<TArchive, TOutput>::value;

//------------------------------------------------------------------------------

/// <summary>
/// Checks that the FUNDAMENTAL VALUE can be serialized in target archive scope.
/// </summary>
template <typename TArchive, typename TValue>
struct can_serialize_value
{
private:
	template <typename TObj, typename TVal>
	static std::enable_if_t<std::is_void_v<decltype(std::declval<TObj>().SerializeValue(std::declval<TVal&>()))>, std::true_type> test(int);

	template <typename, typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive, TValue>(0)) type;
	enum { value = type::value };
};

template <typename TArchive, typename TValue>
constexpr bool can_serialize_value_v = can_serialize_value<TArchive, TValue>::value;

/// <summary>
/// Checks that the FUNDAMENTAL VALUE can be serialized WITH KEY in target archive scope.
/// </summary>
template <typename TArchive, typename TValue, typename TKey>
struct can_serialize_value_with_key
{
private:
	template <typename TObj, typename TVal>
	static std::enable_if_t<std::is_same_v<bool, decltype(std::declval<TObj>().SerializeValue(std::declval<TKey>(), std::declval<TVal&>()))>, std::true_type> test(int);

	template <typename, typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive, TValue>(0)) type;
	enum { value = type::value };
};

template <typename TArchive, typename TValue, typename TKey>
constexpr bool can_serialize_value_with_key_v = can_serialize_value_with_key<TArchive, TValue, TKey>::value;

//------------------------------------------------------------------------------

/// <summary>
/// Checks that the STRING can be serialized in target archive scope.
/// </summary>
template <typename TArchive, typename TValue>
struct can_serialize_string
{
private:
	template <typename TObj, typename TVal>
	static std::enable_if_t<std::is_void_v<decltype(std::declval<TObj>().SerializeString(std::declval<TVal&>()))>, std::true_type> test(int);

	template <typename, typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive, TValue>(0)) type;
	enum { value = type::value };
};

template <typename TArchive, typename TValue>
constexpr bool can_serialize_string_v = can_serialize_string<TArchive, TValue>::value;

/// <summary>
/// Checks that the STRING can be serialized WITH KEY in target archive scope.
/// </summary>
template <typename TArchive, typename TValue, typename TKey>
struct can_serialize_string_with_key
{
private:
	template <typename TObj, typename TVal>
	static std::enable_if_t<std::is_same_v<bool, decltype(std::declval<TObj>().SerializeString(std::declval<TKey>(), std::declval<TVal&>()))>, std::true_type> test(int);

	template <typename, typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive, TValue>(0)) type;
	enum { value = type::value };
};

template <typename TArchive, typename TValue, typename TKey>
constexpr bool can_serialize_string_with_key_v = can_serialize_string_with_key<TArchive, TValue, TKey>::value;

//------------------------------------------------------------------------------

/// <summary>
/// Checks that the CLASS OBJECT can be serialized in target archive scope.
/// </summary>
template <typename TArchive>
struct can_serialize_object
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenObjectScope())>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive>(0)) type;
	enum { value = type::value };
};

template <typename TArchive>
constexpr bool can_serialize_object_v = can_serialize_object<TArchive>::value;

/// <summary>
/// Checks that the CLASS OBJECT can be serialized WITH KEY in target archive scope.
/// </summary>
template <typename TArchive, typename TKey>
struct can_serialize_object_with_key
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenObjectScope(std::declval<TKey>()))>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive>(0)) type;
	enum { value = type::value };
};

template <typename TArchive, typename TKey>
constexpr bool can_serialize_object_with_key_v = can_serialize_object_with_key<TArchive, TKey>::value;

/// <summary>
/// Checks that the archive scope has support serialize values with keys (by checking existence of GetKeyByIndex() method).
/// </summary>
template <typename TArchive, typename TKey>
struct is_object_scope
{
private:
	template <typename TObj>
	static std::enable_if_t<std::is_same_v<decltype(std::declval<TObj>().GetKeyByIndex(std::declval<size_t>())), TKey>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive>(0)) type;
	enum { value = type::value };
};

template <typename TArchive, typename TKey>
constexpr bool is_object_scope_v = is_object_scope<TArchive, TKey>::value;

//------------------------------------------------------------------------------

/// <summary>
/// Checks that the ARRAY can be serialized in target archive scope.
/// </summary>
template <typename TArchive>
struct can_serialize_array
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenArrayScope(std::declval<size_t>()))>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive>(0)) type;
	enum { value = type::value };
};

template <typename TArchive>
constexpr bool can_serialize_array_v = can_serialize_array<TArchive>::value;

/// <summary>
/// Checks that the ARRAY can be serialized WITH KEY in target archive scope.
/// </summary>
template <typename TArchive, typename TKey>
struct can_serialize_array_with_key
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenArrayScope(std::declval<TKey>(), std::declval<size_t>()))>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive>(0)) type;
	enum { value = type::value };
};

template <typename TArchive, typename TKey>
constexpr bool can_serialize_array_with_key_v = can_serialize_array_with_key<TArchive, TKey>::value;

/// <summary>
/// Checks that provided type is convertible to one of element from std::tuple
/// </summary>
template <typename T, typename TTuple>
struct is_type_convertible_to_one_from_tuple
{
private:
	template <class TTuple , size_t... Is >
	static constexpr bool testImpl(std::index_sequence<Is...>) {
		return (std::is_convertible_v<T, typename std::tuple_element<Is, TTuple>::type> || ...);
	}

	template <class TTuple>
	static constexpr bool test() {
		return testImpl<TTuple>(std::make_index_sequence<std::tuple_size<TTuple>::value>{});
	}

	template <>
	static constexpr bool test<std::tuple<>>() {
		return false;
	}

public:
	constexpr static bool value = test<TTuple>();
};

template <typename T, typename TTuple>
constexpr bool is_type_convertible_to_one_from_tuple_v = is_type_convertible_to_one_from_tuple<T, TTuple>::value;

}	// namespace BitSerializer
