/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "conversion_detail/convert_detail.h"
#include "conversion_detail/convert_utf.h"

/// <summary>
/// Type conversions to/from string
/// </summary>
namespace BitSerializer::Convert
{
	/// <summary>
	/// Universal function for convert any value to/from string.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>The converted value</returns>
	template <typename TOut, typename TIn>
	TOut To(TIn&& value)
	{
		if constexpr (std::is_convertible_v<std::decay_t<TIn>, TOut>) {
			return std::forward<TIn>(value);
		}

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
	/// Overload for To() function for case when input and output strings are equal.
	/// </summary>
	/// <param name="value">The input string.</param>
	/// <returns>The constant reference to input string</returns>
	template<typename TSym, typename TAllocator>
	const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& To(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		return value;
	}

	/// <summary>
	/// Converts value to std::string, just syntax sugar of To() function.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>UTF-8 string</returns>
	template <typename TIn>
	std::string ToString(TIn&& value) {
		return To<std::string>(std::forward<TIn>(value));
	}

	/// <summary>
	/// Converts value to the wide string, just syntax sugar of To() function.
	/// </summary>
	/// <param name="value">The input value.</param>
	/// <returns>Unicode string</returns>
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

	/// <summary>
	/// Detects an encoding type by checking BOM.
	/// The BOM sequence will be skipped if it found in the stream.
	/// </summary>
	static UtfType DetectEncoding(std::istream& inputStream)
	{
		static constexpr size_t maxBomSize = 4;

		// Read first bytes for check BOM
		std::string buffer(maxBomSize, 0);
		const auto origPos = inputStream.tellg();
		inputStream.read(buffer.data(), maxBomSize);

		if (Utf8::StartsWithBom(buffer))
		{
			if constexpr (maxBomSize != sizeof Utf8::Bom) {
				inputStream.seekg(origPos + std::streamoff(sizeof Utf8::Bom));
			}
			return UtfType::Utf8;
		}

		// Get back to start position
		inputStream.seekg(origPos);
		return UtfType::Utf8;
	}

}	// namespace BitSerializer::Convert

//------------------------------------------------------------------------------

/// <summary>
/// Global operator << for out class to std::ostream.
/// </summary>
/// <param name="stream">The stream.</param>
/// <param name="value">The value.</param>
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
template <typename TIn, typename TSym, std::enable_if_t<std::is_enum_v<TIn>, int> = 0>
std::basic_ostream<TSym, std::char_traits<TSym>>& operator<<(std::basic_ostream<TSym, std::char_traits<TSym>>& stream, const TIn& value)
{
	std::basic_string<TSym, std::char_traits<TSym>> str;
	BitSerializer::Convert::Detail::ConvertEnum::ToString(value, str);
	stream << str;
	return stream;
}
