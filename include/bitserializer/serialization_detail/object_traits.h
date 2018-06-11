/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "media_archive_base.h"

namespace BitSerializer {

/// <summary>
/// Checks that the class is serializable - has method Serialize().
/// </summary>
template <typename T>
struct is_serializable_class
{
private:
	template <typename U>
	static decltype(std::declval<U>().Serialize(std::declval<ArchiveScope<SerializeMode::Load>>), void(), std::true_type()) test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool is_serializable_class_v = is_serializable_class<T>::value;

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
/// <returns></returns>
template <typename TContainer>
inline size_t GetContainerSize(const TContainer& cont)
{
	if constexpr (has_size_v<TContainer>)
		return cont.size();
	else
		return std::distance(std::begin(cont), std::end(cont));
}

/// <summary>
/// Checks that is input stream.
/// </summary>
template <typename T>
struct is_input_stream
{
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
/// Checks that is output stream.
/// </summary>
template <typename T>
struct is_output_stream
{
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

}	// namespace BitSerializer
