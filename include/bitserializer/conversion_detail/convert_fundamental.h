/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <charconv>
#include <limits>
#include <stdexcept>
#include "bitserializer/config.h"
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS == 0
#include "bitserializer/conversion_detail/convert_compatibility.h"
#endif
#include "bitserializer/conversion_detail/convert_utf.h"

namespace BitSerializer::Convert::Detail
{
	/// <summary>
	/// Converts any of fundamental types to any other fundamental type.
	/// </summary>
	template <typename TSource, typename TTarget, std::enable_if_t<std::is_arithmetic_v<TSource> && std::is_arithmetic_v<TTarget>, int> = 0>
	void To(const TSource& sourceValue, TTarget& targetValue)
	{
		if constexpr (std::is_same_v<TSource, TTarget>)
		{
			targetValue = sourceValue;
		}
		else
		{
			bool result;
			if constexpr (std::is_floating_point_v<TSource>)
			{
				if constexpr (std::is_floating_point_v<TTarget>)
				{
					if (result = sizeof(TTarget) > sizeof(TSource)
						|| (sourceValue >= std::numeric_limits<TTarget>::lowest() && sourceValue <= std::numeric_limits<TTarget>::max()); result)
					{
						targetValue = static_cast<TTarget>(sourceValue);
					}
				}
				else
				{
					throw std::invalid_argument("Floating point number cannot be converted to integer without losing precision");
				}
			}
			else if constexpr (std::is_same_v<bool, TSource> || std::is_same_v<bool, TTarget>)
			{
				auto value = static_cast<TTarget>(sourceValue);
				if (result = static_cast<TSource>(value) == sourceValue; result) {
					targetValue = value;
				}
			}
			else
			{
				auto value = static_cast<TTarget>(sourceValue);
				result = (static_cast<TSource>(value) == sourceValue) && !((value > 0 && sourceValue < 0) || (value < 0 && sourceValue > 0));
				if (result) {
					targetValue = value;
				}
			}

			if (!result)
			{
				throw std::out_of_range("Target type size is insufficient");
			}
		}
	}

	/// <summary>
	/// Converts any UTF string to integer or floating types (except compatibility mode).
	/// </summary>
	template <typename T, typename TSym, std::enable_if_t<(std::is_integral_v<T>
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS
	|| std::is_floating_point_v<T>
#endif
		), int> = 0>
	void To(std::basic_string_view<TSym> in, T& out)
	{
		const auto* it = in.data();
		const auto* end = it + in.size();

		// ReSharper disable once CppPossiblyErroneousEmptyStatements
		for (; (it != end) && (*it == 0x20 || *it == 0x09); ++it);	// Skip spaces

		const auto validateResult = [](std::from_chars_result rc, std::string_view str)
		{
			if (rc.ec != std::errc())
			{
				if (rc.ec == std::errc::result_out_of_range) {
					throw std::out_of_range("Argument out of range");
				}
				if (rc.ec == std::errc::invalid_argument) {
					throw std::invalid_argument("Input string is not a number");
				}
				throw std::runtime_error("Unknown error");
			}

			// Check that string does not contain decimal fractions (parsing a float number to integer is not allowed)
			if constexpr (std::is_integral_v<T>)
			{
				if (rc.ptr + 1 < str.data() + str.size() && *rc.ptr == '.' && std::isdigit(*(rc.ptr + 1)))
				{
					throw std::invalid_argument("Unable to convert string with float number to integer");
				}
			}
		};

		if constexpr (sizeof(TSym) == sizeof(char)) {
			// Uses from_chars only for convert integers as GCC does not support floating types
			validateResult(std::from_chars(it, end, out), in);
		}
		else {
			std::string utf8Str;
			Utf8::Encode(it, end, utf8Str);
			validateResult(std::from_chars(utf8Str.data(), utf8Str.data() + utf8Str.size(), out), utf8Str);
		}
	}

	/// <summary>
	/// Converts any UTF string to boolean. Supports conversion from "0|1" and "true|false" strings.
	/// </summary>
	template <typename TSym>
	void To(std::basic_string_view<TSym> in, bool& ret_Val)
	{
		const auto* startIt = in.data();
		const auto* endIt = startIt + in.size();

		// ReSharper disable once CppPossiblyErroneousEmptyStatements
		for (; (startIt != endIt) && (*startIt == 0x20 || *startIt == 0x09); ++startIt);	// Skip spaces

		const auto size = endIt - startIt;
		if (size >= 1)
		{
			if (std::isdigit(*startIt))
			{
				if (*startIt == '1' && (size == 1 || !std::isdigit(startIt[1])))
				{
					ret_Val = true;
					return;
				}

				if (*startIt == '0' && (size == 1 || !std::isdigit(startIt[1])))
				{
					ret_Val = false;
					return;
				}

				throw std::out_of_range("Argument out of range");
			}

			if (size >= 4 &&
				(startIt[0] == 't' || startIt[0] == 'T') &&
				(startIt[1] == 'r' || startIt[1] == 'R') &&
				(startIt[2] == 'u' || startIt[2] == 'U') &&
				(startIt[3] == 'e' || startIt[3] == 'E'))
			{
				ret_Val = true;
				return;
			}

			if (size >= 5 &&
				(startIt[0] == 'f' || startIt[0] == 'F') &&
				(startIt[1] == 'a' || startIt[1] == 'A') &&
				(startIt[2] == 'l' || startIt[2] == 'L') &&
				(startIt[3] == 's' || startIt[3] == 'S') &&
				(startIt[4] == 'e' || startIt[4] == 'E'))
			{
				ret_Val = false;
				return;
			}
		}

		throw std::invalid_argument("Input string is not a boolean");
	}

	//------------------------------------------------------------------------------

	/// <summary>
	/// Converts any integer and floating types (except compatibility mode) to any UTF string.
	/// </summary>
	template <class T, typename TSym, typename TAllocator, std::enable_if_t<(std::is_integral_v<T>
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS
	|| std::is_floating_point_v<T>
#endif
		), int> = 0>
	void To(const T& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		char buf[32];
		const std::to_chars_result rc = std::to_chars(buf, buf + sizeof(buf), in);
		if (rc.ec != std::errc())
		{
			throw std::runtime_error("Internal error, insufficient buffer size");
		}
		if constexpr (std::is_same_v<char, TSym>)
		{
			out.append(buf, rc.ptr);
		}
		else
		{
			Utf8::Decode(buf, rc.ptr, out);
		}
	}

	/// <summary>
	/// Converts boolean type to any UTF string.
	/// The output representation is "true|false", please cast your boolean type to <int> if you would like to represent it as "1|0".
	/// </summary>
	template <typename TSym, typename TAllocator>
	void To(const bool& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		if (in)
		{
			out.append({ static_cast<TSym>('t'), static_cast<TSym>('r'), static_cast<TSym>('u'), static_cast<TSym>('e') });
		}
		else
		{
			out.append({ static_cast<TSym>('f'), static_cast<TSym>('a'), static_cast<TSym>('l'), static_cast<TSym>('s'), static_cast<TSym>('e') });
		}
	}
}
