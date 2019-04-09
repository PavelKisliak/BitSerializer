/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "conversion_detail/convert_detail.h"

/// <summary>
/// Type conversions to/from string
/// </summary>
namespace BitSerializer::Convert
{
	/// <summary>
	/// Converts from string to specified value or from value to specified string (universal function).
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>The resulting value</returns>
	template <typename TOut, typename TIn>
	TOut To(TIn&& value)
	{
		if constexpr (std::is_convertible_v<std::decay_t<TIn>, TOut>)
			return std::forward<TIn>(value);

		TOut result;
		if constexpr (std::is_same_v<std::decay_t<TIn>, const char*>) {
			// Convert to std::string, as internal implementation does not support c-strings
			Detail::To(std::forward<std::string>(value), result);
		}
		else if constexpr (std::is_same_v<std::decay_t<TIn>, const wchar_t*>) {
			// Convert to std::wstring, as internal implementation does not support c-strings
			Detail::To(std::forward<std::wstring>(value), result);
		}
		else if constexpr (std::is_same_v<std::decay_t<TIn>, std::string_view>) {
			// Convert to std::string, as internal implementation does not support string_view
			Detail::To(std::string(value.data(), value.size()), result);
		}
		else if constexpr (std::is_same_v<std::decay_t<TIn>, std::wstring_view>) {
			// Convert to std::wstring, as internal implementation does not support string_view
			Detail::To(std::wstring(value.data(), value.size()), result);
		}
		else
		{
			Detail::To(std::forward<TIn>(value), result);
		}
		return result;
	}

	/// <summary>
	/// Converts value to the string.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>ANSI string</returns>
	template <typename TIn>
	std::string ToString(TIn&& value) {
		return To<std::string>(std::forward<TIn>(value));
	}

	/// <summary>
	/// Converts value to the wide string.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>Wide string</returns>
	template <typename TIn>
	std::wstring ToWString(TIn&& value) {
		return To<std::wstring>(std::forward<TIn>(value));
	}

	/// <summary>
	/// Converts ANSI string to specified value.
	/// </summary>
	/// <param name="str">The input string.</param>
	/// <returns>The resulting value</returns>
	template <typename T>
	T FromString(const std::string& str)
	{
		T retVal{};
		Detail::To(str, retVal);
		return retVal;
	}

	/// <summary>
	/// Converts wide string to specified value.
	/// </summary>
	/// <param name="str">The input string.</param>
	/// <returns>The resulting value</returns>
	template <typename T>
	T FromString(const std::wstring& str)
	{
		T retVal;
		Detail::To(str, retVal);
		return retVal;
	}

}	// namespace BitSerializer::Convert

//------------------------------------------------------------------------------

/// <summary>
/// Global operator << for out class to std::ostream.
/// </summary>
/// <param name="stream">The stream.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
template <typename TIn, std::enable_if_t<((std::is_class_v<TIn> || std::is_union_v<TIn>) &&
	BitSerializer::Convert::Detail::has_to_string_v<TIn, std::string>), int> = 0>
std::ostream& operator<<(std::ostream& stream, const TIn& value) {
	stream << value.ToString();
	return stream;
}

/// <summary>
/// Global operator << for out class to a std::wostream.
/// </summary>
/// <param name="stream">The stream.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
template <typename TIn, std::enable_if_t<((std::is_class_v<TIn> || std::is_union_v<TIn>) &&
	BitSerializer::Convert::Detail::has_to_string_v<TIn, std::wstring>), int> = 0>
std::wostream& operator<<(std::wostream& stream, const TIn& value) {
	stream << value.ToWString();
	return stream;
}

/// <summary>
/// Global operator << for out registered enum type to streams.
/// </summary>
/// <param name="stream">The stream.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
template <typename TIn, typename TSym, std::enable_if_t<std::is_enum_v<TIn>, int> = 0>
std::basic_ostream<TSym, std::char_traits<TSym>>& operator<<(std::basic_ostream<TSym, std::char_traits<TSym>>& stream, const TIn& value)
{
	std::basic_string<TSym, std::char_traits<TSym>> str;
	BitSerializer::Convert::Detail::ConvertEnum::ToString(value, str);
	stream << str;
	return stream;
}
