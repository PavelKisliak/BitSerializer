/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>
#include <string>
#include <optional>
#include "archive_base.h"

namespace BitSerializer {

/// <summary>
/// Checks that the class is serializable - has internal Serialize() method.
/// </summary>
template <typename T>
struct has_serialize_method
{
private:
	template <typename U>
	static decltype(std::declval<U>().Serialize(std::declval<TArchiveScope<SerializeMode::Load>&()>), std::true_type()) test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_serialize_method_v = has_serialize_method<T>::value;


/// <summary>
/// Checks that the class is serializable - has globally defined SerializeObject() function.
/// </summary>
template <typename T>
struct has_global_serialize_object
{
private:
	template <typename TObj>
	static std::enable_if_t<std::is_same_v<decltype(SerializeObject(std::declval<TArchiveScope<SerializeMode::Load>&>(), std::declval<TObj&>())), void>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_global_serialize_object_v = has_global_serialize_object<T>::value;


/// <summary>
/// Checks that the class is serializable - has globally defined SerializeArray() function.
/// </summary>
template <typename T>
struct has_global_serialize_array
{
private:
	template <typename TObj>
	static std::enable_if_t<std::is_same_v<decltype(SerializeArray(std::declval<TArchiveScope<SerializeMode::Load>&>(), std::declval<TObj&>())), void>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_global_serialize_array_v = has_global_serialize_array<T>::value;


/// <summary>
/// Checks that the container is resizable, by checking existence of resize() method.
/// </summary>
template <typename T>
struct is_resizeable_cont
{
private:
	template <typename U>
	static decltype(std::declval<U>().resize(std::declval<const size_t>()), void(), std::true_type()) test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool is_resizeable_cont_v = is_resizeable_cont<T>::value;


/// <summary>
/// Checks that the container has size() method.
/// </summary>
template <typename T>
struct has_size
{
private:
	template <typename U>
	static decltype(std::declval<U>().size(), void(), std::true_type()) test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_size_v = has_size<T>::value;


/// <summary>
/// Gets the size of the container.
/// </summary>
template <typename TContainer>
size_t GetContainerSize(const TContainer& cont)
{
	if constexpr (has_size_v<TContainer>)
		return cont.size();
	else
		return std::distance(std::begin(cont), std::end(cont));
}


/// <summary>
/// Checks that it is input stream.
/// </summary>
template <typename T>
struct is_input_stream
{
private:
	template <typename TObj>
	static std::enable_if_t<std::is_base_of_v<std::basic_istream<typename TObj::char_type, std::char_traits<typename TObj::char_type>>, TObj>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool is_input_stream_v = is_input_stream<T>::value;


/// <summary>
/// Checks that it is output stream.
/// </summary>
template <typename T>
struct is_output_stream
{
private:
	template <typename TObj>
	static std::enable_if_t<std::is_base_of_v<std::basic_ostream<typename TObj::char_type, std::char_traits<typename TObj::char_type>>, TObj>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool is_output_stream_v = is_output_stream<T>::value;


/// <summary>
/// Checks that it is a validator
/// </summary>
template <typename T, typename TValue>
struct is_validator
{
private:
	template <typename TObj>
	static std::enable_if_t<std::is_same_v<std::optional<std::string>,
		decltype(std::declval<TObj>().operator()(std::declval<const TValue&>(), std::declval<const bool>()))>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T, typename TValue>
constexpr bool is_validator_v = is_validator<T, TValue>::value;

}
