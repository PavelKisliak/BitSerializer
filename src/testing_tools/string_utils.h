/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <algorithm>
#include "bitserializer/conversion_detail/memory_utils.h"

// Helper for handling UTF-8 strings across compilers with different support for char8_t
#if defined(__cpp_lib_char8_t)
inline auto makeUtf8(const char8_t* s)
{
#pragma warning (disable: 26490)
	return reinterpret_cast<const char*>(s);
#pragma warning (default: 26490)
}
/**
 * @brief Macro for defining UTF-8 encoded string literals in a cross-platform manner.
 */
#define UTF8(x) makeUtf8(u8##x)
#else
/**
 * @brief Macro for defining UTF-8 encoded string literals in a cross-platform manner.
 */
#define UTF8(x) u8##x
#endif

 /**
  * @brief Macro for defining path literals compatible with the current platform.
  *
  * On Windows, expands to a wide-character string literal (L"x").
  * On other systems, expands to a UTF-8 string literal.
  */
#ifdef _WIN32
#define UPATH(x) L##x
#else
#define UPATH(x) UTF8(x)
#endif

  /**
   * @brief Constructs a string from a sequence of character or numeric values.
   *
   * @tparam TString The target string type (defaults to `std::string`).
   * @tparam TArgs Types of the arguments (must be convertible to the string's value_type).
   * @param initArgs Character or numeric values to initialize the string with.
   * @return A string composed of the provided characters or converted numeric values.
   */
template <typename TString = std::string, typename ...TArgs>
constexpr TString MakeStringFromSequence(TArgs... initArgs)
{
	return { static_cast<typename TString::value_type>(initArgs)... };
}

/**
 * @brief Converts a native C-style string to big-endian representation.
 *
 * @tparam TChar Character type of the input string.
 * @tparam ArraySize Size of the array.
 * @param str Input null-terminated character array.
 * @return A new string with characters converted to big-endian byte order.
 */
template<typename TChar, size_t ArraySize>
auto NativeStringToBigEndian(const TChar(&str)[ArraySize])
{
	std::basic_string<TChar> result;
	if constexpr (ArraySize != 0)
	{
		std::transform(std::cbegin(str), std::cbegin(str) + (ArraySize - 1),
			std::back_inserter(result), &BitSerializer::Memory::NativeToBigEndian<TChar>);
	}
	return result;
}

/**
 * @brief Converts a native `std::basic_string` to big-endian representation.
 *
 * @tparam TChar Character type of the input string.
 * @tparam TAllocator Allocator type used by the input string.
 * @param str Input string.
 * @return A new string with characters converted to big-endian byte order.
 */
template<typename TChar, typename TAllocator>
auto NativeStringToBigEndian(const std::basic_string<TChar, std::char_traits<TChar>, TAllocator>& str)
{
	std::basic_string<TChar, std::char_traits<TChar>, TAllocator> result;
	std::transform(std::cbegin(str), std::cend(str),
		std::back_inserter(result), &BitSerializer::Memory::NativeToBigEndian<TChar>);
	return result;
}

/**
 * @brief Converts a native C-style string to little-endian representation.
 *
 * @tparam TChar Character type of the input string.
 * @tparam ArraySize Size of the array.
 * @param str Input null-terminated character array.
 * @return A new string with characters converted to little-endian byte order.
 */
template<typename TChar, size_t ArraySize>
auto NativeStringToLittleEndian(const TChar(&str)[ArraySize])
{
	std::basic_string<TChar> result;
	if constexpr (ArraySize != 0)
	{
		std::transform(std::cbegin(str), std::cbegin(str) + (ArraySize - 1),
			std::back_inserter(result), &BitSerializer::Memory::NativeToLittleEndian<TChar>);
	}
	return result;
}

/**
 * @brief Converts a native `std::basic_string` to little-endian representation.
 *
 * @tparam TChar Character type of the input string.
 * @tparam TAllocator Allocator type used by the input string.
 * @param str Input string.
 * @return A new string with characters converted to little-endian byte order.
 */
template<typename TChar, typename TAllocator>
auto NativeStringToLittleEndian(const std::basic_string<TChar, std::char_traits<TChar>, TAllocator>& str)
{
	std::basic_string<TChar, std::char_traits<TChar>, TAllocator> result;
	std::transform(std::cbegin(str), std::cend(str),
		std::back_inserter(result), &BitSerializer::Memory::NativeToLittleEndian<TChar>);
	return result;
}
