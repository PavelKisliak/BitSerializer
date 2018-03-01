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

}	// namespace BitSerializer
