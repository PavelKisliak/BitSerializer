/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <charconv>
#include <cmath>
#include <limits>
#include <stdexcept>
#include "bitserializer/config.h"
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS == 0
#include "bitserializer/conversion_detail/convert_compatibility.h"
#endif
#include "bitserializer/conversion_detail/convert_utf.h"

namespace BitSerializer::Convert::Detail
{
	/**
	 * @brief Converts between fundamental arithmetic types (integers, floating points, bool).
	 *
	 * This function safely converts one arithmetic type to another, checking for overflow or precision loss.
	 * Boolean conversions are also supported with strict range checks.
	 *
	 * @param[in] sourceValue The value to convert from.
	 * @param[out] targetValue The converted value will be stored here.
	 * @throws std::out_of_range If the conversion resulted in data loss.
	 * @throws std::invalid_argument If float-to-integer conversion is attempted.
	 */
	template <typename TSource, typename TTarget, std::enable_if_t<std::is_arithmetic_v<TSource>&& std::is_arithmetic_v<TTarget>, int> = 0>
	void To(const TSource& sourceValue, TTarget& targetValue)
	{
		if constexpr (std::is_same_v<TSource, TTarget>)
		{
			targetValue = sourceValue;
		}
		else
		{
			if constexpr (std::is_floating_point_v<TSource>)
			{
				if constexpr (std::is_floating_point_v<TTarget>)
				{
					if constexpr (sizeof(TTarget) > sizeof(TSource)) {
						targetValue = sourceValue;
					}
					else if (sourceValue >= std::numeric_limits<TTarget>::lowest() && sourceValue <= (std::numeric_limits<TTarget>::max)()) {
						targetValue = static_cast<TTarget>(sourceValue);
					}
					else {
						throw std::out_of_range("Target type size is insufficient");
					}
				}
				else {
					throw std::invalid_argument("Floating point number cannot be converted to integer without losing precision");
				}
			}
			else if constexpr (std::is_same_v<bool, TSource> || std::is_same_v<bool, TTarget>)
			{
				auto value = static_cast<TTarget>(sourceValue);
				if (static_cast<TSource>(value) == sourceValue) {
					targetValue = value;
				}
				else {
					throw std::out_of_range("Target type size is insufficient");
				}
			}
			else
			{
				auto value = static_cast<TTarget>(sourceValue);
				if (static_cast<TSource>(value) == sourceValue && !((value > 0 && sourceValue < 0) || (value < 0 && sourceValue > 0))) {
					targetValue = value;
				}
				else {
					throw std::out_of_range("Target type size is insufficient");
				}
			}
		}
	}

	/**
	 * @brief Converts a UTF string to an integral or floating-point numeric value.
	 *
	 * @param[in] in Input string to parse.
	 * @param[out] out Parsed numeric value.
	 * @throws std::invalid_argument On invalid numeric format.
	 * @throws std::out_of_range If parsed value exceeds the range of T.
	 */
	template <typename T, typename TSym, std::enable_if_t<(std::is_integral_v<T>
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS
			|| std::is_floating_point_v<T>
#endif
			), int> = 0>
	void To(std::basic_string_view<TSym> in, T& out)
	{
		const auto* it = in.data();
		const auto* end = it + in.size();

		// Skip leading spaces
		for (; (it != end) && (*it == 0x20 || *it == 0x09); ++it) {}

		const auto validateResult = [](std::from_chars_result rc, [[maybe_unused]]std::string_view str)
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
			validateResult(std::from_chars(it, end, out), in);
		}
		else
		{
			std::string utf8Str;
			Utf::Utf8::Encode(it, end, utf8Str);
			validateResult(std::from_chars(utf8Str.data(), utf8Str.data() + utf8Str.size(), out), utf8Str);
		}
	}

	/**
	 * @brief Converts a UTF string to a boolean value.
	 *
	 * Accepts:
	 * - "true", "True", "TRUE", etc.
	 * - "false", "False", "FALSE", etc.
	 * - "1" or "0"
	 *
	 * Leading whitespace is ignored. Any additional characters after the recognized value cause failure.
	 *
	 * @param[in] in Input string to parse.
	 * @param[out] ret_Val Resulting boolean value.
	 * @throws std::invalid_argument If input is not a valid boolean representation.
	 */
	template <typename TSym>
	void To(std::basic_string_view<TSym> in, bool& ret_Val)
	{
		const auto* startIt = in.data();
		const auto* endIt = startIt + in.size();

		// Skip leading spaces
		for (; (startIt != endIt) && (*startIt == 0x20 || *startIt == 0x09); ++startIt) {}

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

	/**
	 * @brief Converts a numeric value (integral or floating-point) to a UTF string.
	 *
	 * The output is formatted as a decimal string without scientific notation.
	 *
	 * @param[in] in Value to convert.
	 * @param[out] out Output string containing the UTF-encoded representation.
	 */
	template <class T, typename TSym, typename TAllocator, std::enable_if_t<(std::is_integral_v<T>
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS
			|| std::is_floating_point_v<T>
#endif
			), int> = 0>
	void To(const T& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		if constexpr (std::numeric_limits<T>::has_quiet_NaN)
		{
			// Handle NAN for cross-platform compatibility
			if (std::isnan(in))
			{
				if (std::signbit(in)) {
					out.append({ '-', 'n', 'a', 'n' });
				}
				else {
					out.append({ 'n', 'a', 'n' });
				}
				return;
			}
		}

		char buf[std::numeric_limits<T>::digits];
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
			Utf::Utf8::Decode(buf, rc.ptr, out);
		}
	}

	/**
	 * @brief Converts a boolean type to any UTF string.
	 *
	 * The output representation is "true|false", please cast your boolean type to `int` if you would like to represent it as "1|0".
	 *
	 * @param[in] in Boolean value to convert.
	 * @param[out] out Output string with "true" or "false".
	 */
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
