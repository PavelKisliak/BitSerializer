/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

namespace BitSerializer {

template <typename T>
struct is_serializable_class
{
private:
	template <typename U>
	static decltype(std::declval<U>().Serialize(std::declval<MediaArchiveBase>), void(), std::true_type()) test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool is_serializable_class_v = is_serializable_class<T>::value;

//------------------------------------------------------------------------------
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
