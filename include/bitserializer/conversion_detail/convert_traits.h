/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>

namespace BitSerializer::Convert::Detail
{
	/**
	 * @brief Determines whether a type can be converted to `std::basic_string_view`.
	 *
	 * Checks if the type `T` supports conversion via the `ToStringView(...)` function.
	 */
	template <typename T>
	struct is_convertible_to_string_view
	{
	private:
		template <typename TObj>
		static decltype(ToStringView(std::declval<TObj>()), std::true_type()) test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<T>(0)) type;
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool is_convertible_to_string_view_v = is_convertible_to_string_view<T>::value;

	/**
	 * @brief Determines whether a conversion from `TSource` to `TTarget` is supported.
	 */
	template <typename TSource, typename TTarget>
	struct is_convert_supported
	{
	private:
		template <typename TIn, typename TOut>
		static std::enable_if_t<is_convertible_to_string_view_v<TIn> &&
			std::is_same_v<void, decltype(To(ToStringView(std::declval<TIn&>()), std::declval<TOut&>()))>, std::true_type> test(int);

		template <typename TIn, typename TOut>
		static std::enable_if_t<!is_convertible_to_string_view_v<TIn> &&
			std::is_same_v<void, decltype(To(std::declval<TIn>(), std::declval<TOut&>()))>, std::true_type> test(int);

		template <typename, typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<TSource, TTarget>(0)) type;
		enum { value = type::value };
	};

	template <typename TSource, typename TTarget>
	constexpr bool is_convert_supported_v = is_convert_supported<TSource, TTarget>::value;
}
