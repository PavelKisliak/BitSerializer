/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <algorithm>
#include "bitserializer/conversion_detail/memory_utils.h"

#if defined(__cpp_lib_char8_t)
inline auto makeUtf8(const char8_t* s)
{
#pragma warning (disable: 26490)
	return reinterpret_cast<const char*>(s);
#pragma warning (default: 26490)
}
// Macros for definition a cross-platform UTF-8 string
#define UTF8(x) makeUtf8(u8##x)
#else
#define UTF8(x) u8##x
#endif


// Macros for definition a cross-platform path
#ifdef _WIN32
#define UPATH(x) L##x
#else
#define UPATH(x) UTF8(x)
#endif

// Makes a string from passed sequence of numbers.
template <typename TString = std::string, typename ...TArgs>
constexpr TString MakeStringFromSequence(TArgs... initArgs) noexcept
{
	return { static_cast<typename TString::value_type>(initArgs)... };
}

// Converts native c-string to big-endian representation
template<typename TSym, size_t ArraySize>
auto NativeStringToBigEndian(const TSym(&str)[ArraySize])
{
	std::basic_string<TSym> result;
	if constexpr (ArraySize != 0)
	{
		std::transform(std::cbegin(str), std::cbegin(str) + (ArraySize - 1), std::back_inserter(result), &BitSerializer::Memory::NativeToBigEndian<TSym>);
	}
	return result;
}

// Converts native std::string to big-endian representation
template<typename TChar, typename TAllocator>
auto NativeStringToBigEndian(const std::basic_string<TChar, std::char_traits<TChar>, TAllocator>& str)
{
	std::basic_string<TChar, std::char_traits<TChar>, TAllocator> result;
	std::transform(std::cbegin(str), std::cend(str), std::back_inserter(result), &BitSerializer::Memory::NativeToBigEndian<TChar>);
	return result;
}

// Converts native c-string to little-endian representation
template<typename TChar, size_t ArraySize>
auto NativeStringToLittleEndian(const TChar(&str)[ArraySize])
{
	std::basic_string<TChar> result;
	if constexpr (ArraySize != 0)
	{
		std::transform(std::cbegin(str), std::cbegin(str) + (ArraySize - 1), std::back_inserter(result), &BitSerializer::Memory::NativeToLittleEndian<TChar>);
	}
	return result;
}

// Converts native std::string to little-endian representation
template<typename TChar, typename TAllocator>
auto NativeStringToLittleEndian(const std::basic_string<TChar, std::char_traits<TChar>, TAllocator>& str)
{
	std::basic_string<TChar, std::char_traits<TChar>, TAllocator> result;
	std::transform(std::cbegin(str), std::cend(str), std::back_inserter(result), &BitSerializer::Memory::NativeToLittleEndian<TChar>);
	return result;
}
