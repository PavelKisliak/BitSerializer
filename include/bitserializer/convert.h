/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include "bitserializer/config.h"
#include "bitserializer/conversion_detail/convert_fundamental.h"
#include "bitserializer/conversion_detail/convert_detail.h"
#include "bitserializer/conversion_detail/convert_enum.h"
#if BITSERIALIZER_HAS_FILESYSTEM
#include "bitserializer/conversion_detail/convert_filesystem.h"
#endif
#include "bitserializer/conversion_detail/convert_chrono.h"
#include "bitserializer/conversion_detail/convert_traits.h"

// Suppress C4702 - unreachable code
#pragma warning(push)
#pragma warning(disable: 4702)

namespace BitSerializer::Convert
{
	/// <summary>
	/// Checks whether conversion from `TIn` to `TOut` is supported.
	/// </summary>
	template <typename TIn, typename TOut>
	constexpr bool IsConvertible()
	{
		return Detail::is_convert_supported_v<TIn, TOut>;
	}

	/// <summary>
	/// Generic function for converting a value.
	/// </summary>
	/// <param name="value">The source value.</param>
	/// <param name="initArgs">Arguments that are used to construct the output type (useful for passing an allocator or an existing string).</param>
	/// <returns>The converted value</returns>
	/// <exception cref="std::out_of_range">Thrown when overflow target value.</exception>
	/// <exception cref="std::invalid_argument">Thrown when input value has wrong format.</exception>
	template <typename TOut, typename TIn, typename... TInitArgs, std::enable_if_t<std::is_constructible_v<TOut, TInitArgs...>, int> = 0>
	TOut To(TIn&& value, TInitArgs... initArgs)
	{
		constexpr bool isSame = std::is_same_v<TOut, std::decay_t<TIn>>;
		constexpr bool isConvertible = IsConvertible<TIn, TOut>();

		// You may need to implement internal or external conversion functions for your types (see: "docs/bitserializer_convert.md").
		static_assert(isSame || isConvertible, "BitSerializer::Convert. Converting these types is not supported.");

		// Convert to the same type
		if constexpr (isSame && sizeof...(TInitArgs) == 0) {
			return std::forward<TIn>(value);
		}
		else if constexpr (isConvertible)
		{
			TOut result(std::forward<TInitArgs>(initArgs)...);

			using namespace Detail;
			if constexpr (is_convertible_to_string_view_v<TIn>) {
				// String types like std::basic_string and c-strings must be converted to string_view
				To(ToStringView(value), result);
			}
			else {
				To(std::forward<TIn>(value), result);
			}
			return result;
		}
		else
		{
			// Will never be reached due to static_assert (defined for analyzers)
			return {};
		}
	}

	/// <summary>
	/// Converts value to std::string, just syntax sugar of To<std::string>() function.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <param name="initArgs">Arguments that are used to construct the output type (useful for passing an allocator or an existing string).</param>
	/// <returns>UTF-8 string</returns>
	/// <exception cref="std::out_of_range">Thrown when overflow target value.</exception>
	/// <exception cref="std::invalid_argument">Thrown when input value has wrong format.</exception>
	template <typename TIn, typename... TInitArgs, std::enable_if_t<std::is_constructible_v<std::string, TInitArgs...>, int> = 0>
	std::string ToString(TIn&& value, TInitArgs... initArgs)
	{
		return To<std::string>(std::forward<TIn>(value), std::forward<TInitArgs>(initArgs)...);
	}

	/// <summary>
	/// Converts value to the wide string, just syntax sugar of To<std::wstring>() function.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <param name="initArgs">Arguments that are used to construct the output type (useful for passing an allocator or an existing string).</param>
	/// <returns>Wide string</returns>
	/// <exception cref="std::out_of_range">Thrown when overflow target value.</exception>
	/// <exception cref="std::invalid_argument">Thrown when input value has wrong format.</exception>
	template <typename TIn, typename... TInitArgs, std::enable_if_t<std::is_constructible_v<std::wstring, TInitArgs...>, int> = 0>
	std::wstring ToWString(TIn&& value, TInitArgs... initArgs)
	{
		return To<std::wstring>(std::forward<TIn>(value), std::forward<TInitArgs>(initArgs)...);
	}

	/// <summary>
	/// Generic function for converting a value (does not throw exceptions).
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <param name="initArgs">Arguments that are used to construct the output type (useful for passing an allocator or an existing string).</param>
	/// <returns>The converted value or empty when occurred an error</returns>
	template <typename TOut, typename TIn, typename... TInitArgs, std::enable_if_t<std::is_constructible_v<TOut, TInitArgs...>, int> = 0>
	std::optional<TOut> TryTo(TIn&& value, TInitArgs... initArgs) noexcept
	{
		try
		{
			return std::optional<TOut>(To<TOut>(std::forward<TIn>(value), std::forward<TInitArgs>(initArgs)...));
		}
		catch (const std::exception&)
		{
			return {};
		}
	}
}

#pragma warning(pop)
