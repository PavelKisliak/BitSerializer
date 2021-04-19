/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include "conversion_detail/convert_fundamental.h"
#include "conversion_detail/convert_detail.h"
#include "conversion_detail/convert_std.h"

namespace BitSerializer::Convert
{
	/// <summary>
	/// Universal function for convert value.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>The converted value</returns>
	/// <exception cref="std::out_of_range">Thrown when value cannot be converted.</exception>
	template <typename TOut, typename TIn>
	TOut To(TIn&& value)
	{
		// Convert to the same type
		if constexpr (std::is_same_v<TOut, std::decay_t<TIn>>) {
			return value;
		}
		else
		{
			TOut result;
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
	}

	/// <summary>
	/// Converts value to std::string, just syntax sugar of To<std::string>() function.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>UTF-8 string</returns>
	/// <exception cref="std::out_of_range">Thrown when value cannot be converted.</exception>
	template <typename TIn>
	std::string ToString(TIn&& value) {
		return To<std::string>(std::forward<TIn>(value));
	}

	/// <summary>
	/// Converts value to the wide string (UTF-16), just syntax sugar of To<std::wstring>() function.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>UTF-16 string</returns>
	/// <exception cref="std::out_of_range">Thrown when value cannot be converted.</exception>
	template <typename TIn>
	std::wstring ToWString(TIn&& value) {
		return To<std::wstring>(std::forward<TIn>(value));
	}

	/// <summary>
	/// Universal function for convert value which no throws exceptions.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>The converted value or empty when occurred an error</returns>
	template <typename TOut, typename TIn>
	std::optional<TOut> TryTo(TIn&& value) noexcept
	{
		try
		{
			return std::optional<TOut>(To<TOut>(std::forward<TIn>(value)));
		}
		catch (const std::exception&)
		{
			return {};
		}
	}
}

//------------------------------------------------------------------------------

/// <summary>
/// Global operator << for out class to std::ostream.
/// </summary>
/// <param name="stream">The stream.</param>
/// <param name="value">The value.</param>
template <typename TIn, std::enable_if_t<((std::is_class_v<TIn> || std::is_union_v<TIn>) &&
	BitSerializer::Convert::Detail::has_internal_ToString_v<TIn, std::basic_string<char, std::char_traits<char>>>), int> = 0>
std::ostream& operator<<(std::ostream& stream, const TIn& value) {
	stream << value.ToString();
	return stream;
}

/// <summary>
/// Global operator << for out registered enum type to streams.
/// </summary>
/// <param name="stream">The stream.</param>
/// <param name="value">The value.</param>
template <typename TIn, typename TSym, std::enable_if_t<std::is_enum_v<TIn>, int> = 0>
std::basic_ostream<TSym, std::char_traits<TSym>>& operator<<(std::basic_ostream<TSym, std::char_traits<TSym>>& stream, const TIn& value)
{
	std::basic_string<TSym, std::char_traits<TSym>> str;
	BitSerializer::Convert::Detail::ConvertEnum::ToString(value, str);
	stream << str;
	return stream;
}
