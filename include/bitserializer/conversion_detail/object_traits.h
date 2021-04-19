/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>

namespace BitSerializer::Convert::Detail
{
	template <typename T, typename TOutStr>
	struct has_global_to_string
	{
	private:
		template <typename TObj, typename TStr>
		static std::enable_if_t<std::is_same_v<decltype(to_string(std::declval<const TObj&>())), TStr>, std::true_type> test(int);

		template <typename TObj, typename TStr>
		static std::enable_if_t<std::is_same_v<decltype(to_wstring(std::declval<const TObj&>())), TStr>, std::true_type> test(int);

		template <typename, typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<T, TOutStr>(0)) type;
		enum { value = type::value };
	};

	template <typename T, typename TOutStr>
	constexpr bool has_global_to_string_v = has_global_to_string<T, TOutStr>::value;

	//------------------------------------------------------------------------------

	template <typename T, typename TOutStr>
	struct has_internal_ToString
	{
	private:
		template <typename TObj, typename TStr>
		static std::enable_if_t<sizeof(typename TStr::value_type) == sizeof(char) &&
			std::is_convertible_v<decltype(std::declval<TObj>().ToString()), TStr>, std::true_type> test(int);

		template <typename TObj, typename TStr>
		static std::enable_if_t<sizeof(typename TStr::value_type) == sizeof(char16_t) &&
			std::is_convertible_v<decltype(std::declval<TObj>().ToU16String()), TStr>, std::true_type> test(int);

		template <typename TObj, typename TStr>
		static std::enable_if_t<sizeof(typename TStr::value_type) == sizeof(char32_t) &&
			std::is_convertible_v<decltype(std::declval<TObj>().ToU32String()), TStr>, std::true_type> test(int);

		template <typename, typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<T, TOutStr>(0)) type;
		enum { value = type::value };
	};

	template <typename T, typename TOutStr>
	constexpr bool has_internal_ToString_v = has_internal_ToString<T, TOutStr>::value;


	template <typename T, typename TInStr>
	struct has_internal_FromString
	{
	private:
		template <typename TObj, typename TStr>
		static decltype(std::declval<TObj>().FromString(std::declval<const TStr&>()), void(), std::true_type()) test(int);

		template <typename, typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<T, TInStr>(0)) type;
		enum { value = type::value };
	};

	template <typename T, typename TInStr>
	constexpr bool has_internal_FromString_v = has_internal_FromString<T, TInStr>::value;

	//------------------------------------------------------------------------------

	template <typename T>
	struct is_convertible_to_string_view
	{
	private:
		template <typename TSym, typename TAllocator>
		static std::basic_string_view<TSym> ToStringView(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& in);

		template <typename TSym>
		static std::basic_string_view<TSym> ToStringView(const TSym* in);

		template <typename TObj>
		static decltype(ToStringView(std::declval<TObj>()), void(), std::true_type()) test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<T>(0)) type;
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool is_convertible_to_string_view_v = is_convertible_to_string_view<T>::value;
}
