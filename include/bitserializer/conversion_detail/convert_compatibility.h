/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cerrno>
#include <climits>
#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include "bitserializer/common/text.h"
#include "bitserializer/conversion_detail/convert_utf.h"

namespace BitSerializer::Convert::Detail
{
	/**
	 * @brief Result of decimal-point normalization.
	 */
	struct NormalizedNumber
	{
		const char* data;
		size_t size;
	};

	/**
	 * @brief Replaces '.' in the token with the current locale's decimal separator.
	 *
	 * Any locale separator other than '.' terminates the number, so only '.'
	 * is accepted as the decimal separator (matching the C locale).
	 *
	 * @param buffer      Mutable buffer containing the token and enough room for the
	 *                    locale separator expansion.
	 * @param tokenSize   Number of characters in the token.
	 * @param bufferSize  Total size of the buffer, including the null terminator.
	 * @return Normalized token; `data` is null-terminated.
	 */
	inline NormalizedNumber NormalizeDecimalPoint(char* buffer, size_t tokenSize, size_t bufferSize)
	{
		if (tokenSize >= bufferSize) {
			throw std::invalid_argument("The number buffer is too small");
		}

		const std::lconv* localeInfo = std::localeconv();
		const char* decimalPoint = localeInfo != nullptr ? localeInfo->decimal_point : nullptr;
		if (decimalPoint == nullptr || (decimalPoint[0] == '.' && decimalPoint[1] == '\0')) {
			buffer[tokenSize] = '\0';
			return {buffer, tokenSize};
		}
		const size_t decimalPointLength = std::strlen(decimalPoint);
		if (decimalPointLength == 0) {
			buffer[tokenSize] = '\0';
			return {buffer, tokenSize};
		}

		const std::string_view token(buffer, tokenSize);
		const size_t nativePos = token.find(decimalPoint, 0, decimalPointLength);
		const size_t tokenLength = nativePos == std::string_view::npos ? tokenSize : nativePos;

		if (decimalPointLength == 1)
		{
			for (size_t i = 0; i < tokenLength; ++i) {
				if (buffer[i] == '.') {
					buffer[i] = decimalPoint[0];
				}
			}
			buffer[tokenLength] = '\0';
			return {buffer, tokenLength};
		}

		const size_t dotPos = token.find('.');
		if (dotPos == std::string_view::npos || dotPos >= tokenLength) {
			buffer[tokenLength] = '\0';
			return {buffer, tokenLength};
		}
		const size_t normalizedLength = tokenLength + decimalPointLength - 1;
		if (normalizedLength >= bufferSize) {
			throw std::invalid_argument("The locale's decimal separator is too long");
		}
		std::memmove(buffer + dotPos + decimalPointLength, buffer + dotPos + 1, tokenLength - dotPos);
		std::memcpy(buffer + dotPos, decimalPoint, decimalPointLength);
		buffer[normalizedLength] = '\0';
		return {buffer, normalizedLength};
	}

	/**
	 * @brief Replaces the current locale's decimal separator with '.' in a formatted number.
	 *
	 * @param buffer  Null-terminated formatted number.
	 * @param length  Length of the formatted number.
	 * @return New length after replacing the separator.
	 */
	inline size_t ReplaceLocaleDecimalPointWithDot(char* buffer, size_t length)
	{
		const std::lconv* localeInfo = std::localeconv();
		const char* decimalPoint = localeInfo != nullptr ? localeInfo->decimal_point : nullptr;
		if (decimalPoint == nullptr || (decimalPoint[0] == '.' && decimalPoint[1] == '\0')) {
			return length;
		}
		const size_t decimalPointLength = std::strlen(decimalPoint);
		if (decimalPointLength == 0) {
			return length;
		}
		char* const pos = std::strstr(buffer, decimalPoint);
		if (pos == nullptr) {
			return length;
		}
		const size_t posOffset = static_cast<size_t>(pos - buffer);
		*pos = '.';
		if (decimalPointLength > 1)
		{
			std::memmove(pos + 1, pos + decimalPointLength, length - posOffset - decimalPointLength + 1);
			length -= decimalPointLength - 1;
		}
		return length;
	}

	/**
	 * @brief Parses a normalized floating-point token.
	 */
	template <typename T>
	T ParseFloat(const char* str, char** outEnd)
	{
		if constexpr (std::is_same_v<T, float>) {
			return std::strtof(str, outEnd);
		}
		else if constexpr (std::is_same_v<T, double>) {
			return std::strtod(str, outEnd);
		}
		else {
			static_assert(std::is_same_v<T, long double>, "Unsupported floating-point type");
			return std::strtold(str, outEnd);
		}
	}

	/**
	 * @brief Compares an ASCII character case-insensitively.
	 *
	 * @note `expected` must be a lowercase ASCII letter. Non-ASCII input never matches.
	 */
	template <typename TChar>
	constexpr bool IsAsciiCaseInsensitive(TChar ch, char expected)
	{
		using unsigned_char_type = std::make_unsigned_t<TChar>;
		return (static_cast<unsigned_char_type>(ch) | 0x20) == static_cast<unsigned_char_type>(expected);
	}

	/**
	 * @brief Converts a UTF string to a floating-point number.
	 *
	 * @param in Input string view containing the numeric value.
	 * @param out Output variable to store the parsed value.
	 * @throws std::invalid_argument If parsing fails or input is not a valid number.
	 * @throws std::out_of_range If result exceeds representable range.
	 */
	template <typename T, typename TSym, std::enable_if_t<(std::is_floating_point_v<T>), int> = 0>
	void To(std::basic_string_view<TSym> in, T& out)
	{
		if (in.empty()) {
			throw std::invalid_argument("Input string is not a number");
		}
		const auto* start = in.data();
		const auto* end = start + in.size();

		// Skip leading spaces
		for (; (start != end) && Text::IsWhitespace(*start); ++start) {}
		const size_t size = end - start;
		if (size == 0) {
			throw std::invalid_argument("Input string is not a number");
		}

		// Handle special floating-point numbers (inf/nan)
		if (size >= 3)
		{
			auto it = start;
			const bool isNegative = *it == '-';
			if (isNegative || *it == '+') {
				++it;
			}

			if (end - it >= 3)
			{
				// inf
				if constexpr (std::numeric_limits<T>::has_infinity)
				{
					if (IsAsciiCaseInsensitive(it[0], 'i') && IsAsciiCaseInsensitive(it[1], 'n') && IsAsciiCaseInsensitive(it[2], 'f'))
					{
						out = isNegative ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::infinity();
						return;
					}
				}

				// nan
				if constexpr (std::numeric_limits<T>::has_quiet_NaN)
				{
					if (IsAsciiCaseInsensitive(it[0], 'n') && IsAsciiCaseInsensitive(it[1], 'a') && IsAsciiCaseInsensitive(it[2], 'n'))
					{
						out = isNegative ? -std::numeric_limits<T>::quiet_NaN() : std::numeric_limits<T>::quiet_NaN();
						return;
					}
				}
			}
		}

		T result;
		// The parsed prefix limit is intentionally larger than max_digits10:
		// strto* accepts longer inputs and rounds them.
		constexpr size_t maxTokenLength = 64;
		// Headroom for a multi-byte locale decimal separator (e.g. U+066B in UTF-8)
		constexpr size_t maxBufSize = maxTokenLength + MB_LEN_MAX;
		size_t parseSize = size < maxTokenLength ? size : maxTokenLength;
		char buf[maxBufSize];
		if constexpr (std::is_same_v<char, TSym>)
		{
			// Copy to temporary buffer for prepare null-terminated c-string
			std::memcpy(buf, &*start, parseSize);
		}
		else
		{
			std::string utf8Str;
			Utf::Utf8::Encode(start, end, utf8Str);
			const size_t utf8ParseSize = utf8Str.size() < parseSize ? utf8Str.size() : parseSize;
			std::memcpy(buf, utf8Str.data(), utf8ParseSize);
			parseSize = utf8ParseSize;
		}
		buf[parseSize] = '\0';

		const NormalizedNumber normalized = NormalizeDecimalPoint(buf, parseSize, sizeof(buf));
		errno = 0;
		char* endPos = nullptr;
		result = ParseFloat<T>(normalized.data, &endPos);
		const bool isParseFailed = normalized.data == endPos;

		if (errno == ERANGE) {
			throw std::out_of_range("Numeric overflow");
		}
		if (isParseFailed) {
			throw std::invalid_argument("Input string is not a number");
		}
		out = result;
	}

	/**
	 * @brief Returns the printf format for the floating-point type.
	 */
	template <typename T>
	constexpr const char* GetFloatFormat()
	{
		if constexpr (std::is_same_v<T, float>) {
			return "%.9g";
		}
		else if constexpr (std::is_same_v<T, double>) {
			return "%.17g";
		}
		else
		{
			static_assert(std::is_same_v<T, long double>, "Unsupported floating-point type");
			constexpr int p = std::numeric_limits<long double>::max_digits10;
			if constexpr (p == 18) { return "%.18Lg"; }  // MSVC / ARM (long double == double)
			if constexpr (p == 21) { return "%.21Lg"; }  // x86 Extended Precision (80-bit)
			if constexpr (p >= 33) { return "%.36Lg"; }  // Quad precision (128-bit)
			return "%.21Lg";                             // Fallback
		}
	}

	/**
	 * @brief Converts a floating-point number to a UTF string representation.
	 *
	 * @param in Value to convert.
	 * @param out Output string object.
	 * @throws std::overflow_error If internal buffer is insufficient.
	 */
	template <class T, typename TSym, typename TAllocator, std::enable_if_t<(std::is_floating_point_v<T>), int> = 0>
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

		if constexpr (std::numeric_limits<T>::has_infinity)
		{
			// Avoid locale-dependent formatting of infinity
			if (std::isinf(in))
			{
				if (std::signbit(in)) {
					out.append({ '-', 'i', 'n', 'f' });
				}
				else {
					out.append({ 'i', 'n', 'f' });
				}
				return;
			}
		}

		// Headroom for a multi-byte locale decimal separator
		constexpr auto bufSize = std::numeric_limits<T>::digits + 1 + MB_LEN_MAX;
		char buf[bufSize];
		const int result = std::snprintf(buf, bufSize, GetFloatFormat<T>(), in);
		if (result < 0 || result >= bufSize) {
			throw std::overflow_error("Internal error");
		}
		const size_t length = ReplaceLocaleDecimalPointWithDot(buf, static_cast<size_t>(result));
		if constexpr (std::is_same_v<char, TSym>) {
			out.append(buf, buf + length);
		}
		else {
			Utf::Utf8::Decode(buf, buf + length, out);
		}
	}
}
