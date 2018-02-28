/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "media_archive_base.h"

// ToDo: add traits for string serialization

namespace BitSerializer {

template <typename T>
struct is_archive_scope
{
	constexpr static bool value = std::is_base_of<ArchiveScope<SerializeMode::Load>, T>::value ||
		std::is_base_of<ArchiveScope<SerializeMode::Save>, T>::value;
};

template <typename T>
constexpr bool is_archive_scope_v = is_archive_scope<T>::value;

//------------------------------------------------------------------------------
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

template <typename TArchive, typename TValue>
struct can_serialize_value_with_key
{
private:
	template <typename TObj, typename TVal>
	static std::enable_if_t<std::is_void_v<decltype(std::declval<TObj>().SerializeValue(std::declval<const typename TArchive::key_type&>(), std::declval<TVal&>()))>, std::true_type> test(int);

	template <typename, typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive, TValue>(0)) type;
	enum { value = type::value };
};

template <typename TArchive, typename TValue>
constexpr bool can_serialize_value_with_key_v = can_serialize_value_with_key<TArchive, TValue>::value;

//------------------------------------------------------------------------------

template <typename TArchive>
struct can_serialize_object
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForSerializeObject())>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive>(0)) type;
	enum { value = type::value };
};

template <typename TArchive>
constexpr bool can_serialize_object_v = can_serialize_object<TArchive>::value;

template <typename TArchive>
struct can_serialize_object_with_key
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForSerializeObject(std::declval<const typename TArchive::key_type&>()))>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive>(0)) type;
	enum { value = type::value };
};

template <typename TArchive>
constexpr bool can_serialize_object_with_key_v = can_serialize_object_with_key<TArchive>::value;

//------------------------------------------------------------------------------

template <typename TArchive>
struct can_serialize_array
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForLoadArray())> &&
		!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForSaveArray(std::declval<size_t>()))>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive>(0)) type;
	enum { value = type::value };
};

template <typename TArchive>
constexpr bool can_serialize_array_v = can_serialize_array<TArchive>::value;

template <typename TArchive>
struct can_serialize_array_with_key
{
private:
	template <typename TObj>
	static std::enable_if_t<!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForLoadArray(std::declval<const typename TArchive::key_type&>()))> &&
		!std::is_void_v<decltype(std::declval<TObj>().OpenScopeForSaveArray(std::declval<const typename TArchive::key_type&>(), std::declval<size_t>()))>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<TArchive>(0)) type;
	enum { value = type::value };
};

template <typename TArchive>
constexpr bool can_serialize_array_with_key_v = can_serialize_array_with_key<TArchive>::value;


}	// namespace BitSerializer
