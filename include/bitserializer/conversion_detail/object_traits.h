/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>

namespace BitSerializer::Convert::Detail {

template <typename T, typename TOutStr>
struct has_to_string
{
private:
	template <typename TObj, typename TStr>
	static std::enable_if_t<std::is_same_v<typename TStr::value_type, char> &&
		std::is_same_v<decltype(std::declval<TObj>().ToString()), TStr>, std::true_type> test(int);

	template <typename TObj, typename TStr>
	static std::enable_if_t<std::is_same_v<typename TStr::value_type, wchar_t> &&
		std::is_same_v<decltype(std::declval<TObj>().ToWString()), TStr>, std::true_type> test(int);

	template <typename, typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T, TOutStr>(0)) type;
	enum { value = type::value };
};

template <typename T, typename TOutStr>
constexpr bool has_to_string_v = has_to_string<T, TOutStr>::value;

//------------------------------------------------------------------------------
template <typename T, typename TInStr>
struct has_from_string
{
private:
	template <typename TObj, typename TStr>
	static std::enable_if_t<std::is_same_v<typename TStr::value_type, char> &&
		std::is_void_v<decltype(std::declval<TObj>().FromString(std::declval<const TStr&>()))>, std::true_type> test(int);

	template <typename TObj, typename TStr>
	static std::enable_if_t<std::is_same_v<typename TStr::value_type, wchar_t> &&
		std::is_void_v<decltype(std::declval<TObj>().FromString(std::declval<const TStr&>()))>, std::true_type> test(int);

	template <typename, typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T, TInStr>(0)) type;
	enum { value = type::value };
};

template <typename T, typename TOutStr>
constexpr bool has_from_string_v = has_from_string<T, TOutStr>::value;

}	// BitSerializer::Convert::Detail
