/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "conversion_detail\convert_detail.h"

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
		Detail::ToString(value, ret_Str);
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
		Detail::ToString(value, ret_Str);
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
		Detail::FromString(str, retVal);
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
		Detail::FromString(str, retVal);
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
		if constexpr (std::is_same_v<TOut, TIn>)
			return std::move(value);

		TOut result;
		if constexpr (std::is_same_v<TOut, std::string> || std::is_same_v<TOut, std::wstring>) {
			Detail::ToString(std::move(value), result);
		}
		else if constexpr (std::is_same_v<TIn, std::string> || std::is_same_v<TIn, std::wstring>) {
			Detail::FromString(std::move(value), result);
		}
		else {
			static_assert(false, "BitSerializer::Convert::To(). The input or output value must be a string.");
		}
		return result;
	};

}	// namespace BitSerializer::Convert
