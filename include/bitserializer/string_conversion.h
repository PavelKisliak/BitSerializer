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
	/// Converts value to the string.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>ANSI string</returns>
	template <typename T>
	inline std::string ToString(T value)
	{
		std::string ret_Str;
		Detail::To(value, ret_Str);
		return ret_Str;
	};

	/// <summary>
	/// Converts value to the wide string.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>Wide string</returns>
	template <typename T>
	inline std::wstring ToWString(T value)
	{
		std::wstring ret_Str;
		Detail::To(value, ret_Str);
		return ret_Str;
	};

	/// <summary>
	/// Converts ANSI string to specified value.
	/// </summary>
	/// <param name="str">The input string.</param>
	/// <returns>The resulting value</returns>
	template <typename T>
	inline T FromString(const std::string& str)
	{
		T retVal;
		Detail::To(str, retVal);
		return retVal;
	};

	/// <summary>
	/// Converts wide string to specified value.
	/// </summary>
	/// <param name="str">The input string.</param>
	/// <returns>The resulting value</returns>
	template <typename T>
	inline T FromString(const std::wstring& str)
	{
		T retVal;
		Detail::To(str, retVal);
		return retVal;
	};

	/// <summary>
	/// Converts from string to specified value or from value to specified string (universal function).
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>The resulting value</returns>
	template <typename TOut, typename TIn>
	inline TOut To(TIn&& value)
	{
		if constexpr (std::is_same_v<TOut, std::decay_t<TIn>>)
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
		else
		{
			Detail::To(std::forward<TIn>(value), result);
		}
		return result;
	};

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
inline std::ostream& operator<<(std::ostream& stream, const TIn& value) {
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
inline std::wostream& operator<<(std::wostream& stream, const TIn& value) {
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
inline std::basic_ostream<TSym, std::char_traits<TSym>>& operator<<(std::basic_ostream<TSym, std::char_traits<TSym>>& stream, const TIn& value)
{
	std::basic_string<TSym, std::char_traits<TSym>> str;
	BitSerializer::Convert::Detail::ConvertEnum::ToString(value, str);
	stream << str;
	return stream;
}
