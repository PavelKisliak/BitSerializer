/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

namespace BitSerializer {

template <typename TMediaArchive, typename TValue>
struct can_serialize_value
{
private:
	template <typename TObj, typename TVal>
	static std::enable_if_t<std::is_void_v<decltype(std::declval<TObj>().LoadValue(std::declval<TVal&>()))> &&
		std::is_void_v<decltype(std::declval<TObj>().SaveValue(std::declval<TVal>()))>, std::true_type> test(int);

	template <typename, typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TMediaArchive, TValue>(0)) type;
	enum { value = type::value };
};

template <typename TMediaArchive, typename TValue>
constexpr bool can_serialize_value_v = can_serialize_value<TMediaArchive, TValue>::value;

template <typename TMediaArchive, typename TValue>
struct can_serialize_value_with_key
{
private:
	template <typename TObj, typename TVal>
	static std::enable_if_t<std::is_void_v<decltype(std::declval<TObj>().LoadValue(std::declval<const typename TMediaArchive::key_type&>(), std::declval<TVal&>()))> &&
		std::is_void_v<decltype(std::declval<TObj>().SaveValue(std::declval<const typename TMediaArchive::key_type&>(), std::declval<TVal>()))>, std::true_type> test(int);

	template <typename, typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TMediaArchive, TValue>(0)) type;
	enum { value = type::value };
};

template <typename TMediaArchive, typename TValue>
constexpr bool can_serialize_value_with_key_v = can_serialize_value_with_key<TMediaArchive, TValue>::value;

//------------------------------------------------------------------------------

template <typename TMediaArchive>
struct can_serialize_object
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForLoadObject())> &&
		!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForSaveObject())>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TMediaArchive>(0)) type;
	enum { value = type::value };
};

template <typename TMediaArchive>
constexpr bool can_serialize_object_v = can_serialize_object<TMediaArchive>::value;

template <typename TMediaArchive>
struct can_serialize_object_with_key
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForLoadObject(std::declval<const typename TMediaArchive::key_type&>()))> &&
		!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForSaveObject(std::declval<const typename TMediaArchive::key_type&>()))>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TMediaArchive>(0)) type;
	enum { value = type::value };
};

template <typename TMediaArchive>
constexpr bool can_serialize_object_with_key_v = can_serialize_object_with_key<TMediaArchive>::value;

//------------------------------------------------------------------------------

template <typename TMediaArchive>
struct can_serialize_array
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForLoadArray())> &&
		!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForSaveArray(std::declval<size_t>()))>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TMediaArchive>(0)) type;
	enum { value = type::value };
};

template <typename TMediaArchive>
constexpr bool can_serialize_array_v = can_serialize_array<TMediaArchive>::value;

template <typename TMediaArchive>
struct can_serialize_array_with_key
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForLoadArray(std::declval<const typename TMediaArchive::key_type&>()))> &&
		!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForSaveArray(std::declval<const typename TMediaArchive::key_type&>(), std::declval<size_t>()))>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TMediaArchive>(0)) type;
	enum { value = type::value };
};

template <typename TMediaArchive>
constexpr bool can_serialize_array_with_key_v = can_serialize_array_with_key<TMediaArchive>::value;


}	// namespace BitSerializer
