/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <charconv>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include "common/simd/scan.h"
#include "common/string_buffer.h"
#include "bitserializer/config.h"
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS == 0
#include "bitserializer/conversion_detail/convert_compatibility.h"
#endif

namespace BitSerializer::Json::Detail
{
	using BitSerializer::Detail::StringBuffer;

	/**
	 * @brief Returns `true` for NaN and positive/negative infinity.
	 *
	 * Uses comparisons for infinity instead of `std::isfinite()` because Valgrind 3.18
	 * misclassifies `long double` infinity on x87.
	 */
	template <typename T>
	[[nodiscard]] bool IsNonFinite(T value) noexcept
	{
		if constexpr (std::numeric_limits<T>::has_quiet_NaN)
		{
			if (std::isnan(value)) {
				return true;
			}
		}
		if constexpr (std::numeric_limits<T>::has_infinity)
		{
			if (value == std::numeric_limits<T>::infinity() || value == -std::numeric_limits<T>::infinity()) {
				return true;
			}
		}
		return false;
	}

	/**
	 * @brief Returns the index of the first byte in `source` at or after `pos` that must be escaped inside
	 * a JSON string, i.e. `"`, `\` or a control byte below 0x20, or `std::string_view::npos` if there is none.
	 */
	[[nodiscard]] inline size_t FindFirstEscape(std::string_view source, size_t pos) noexcept
	{
		return BitSerializer::Simd::FindFirstOfOrLessThan<0x20, '"', '\\'>(source, pos);
	}

	template <typename T>
	void WriteNumberChars(StringBuffer& target, char* out, T value)
	{
		constexpr size_t maxNumberLength = std::numeric_limits<T>::digits + 1 + MB_LEN_MAX;
		const auto result = std::to_chars(out, out + maxNumberLength, value);
		if (result.ec != std::errc()) {
			throw std::runtime_error("Internal error, insufficient number buffer");
		}
		target.Commit(static_cast<size_t>(result.ptr - out));
	}

	template <typename T>
	void WriteNumberTo(StringBuffer& target, T value)
	{
		constexpr size_t maxNumberLength = std::numeric_limits<T>::digits + 1 + MB_LEN_MAX;
		target.Ensure(maxNumberLength);
		char* const out = target.Tail();

#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS
		WriteNumberChars(target, out, value);
#else
		if constexpr (std::is_floating_point_v<T>)
		{
			// Some older standard libraries (e.g. GCC 9) do not provide floating-point std::to_chars.
			const int length = std::snprintf(out, maxNumberLength, Convert::Detail::GetFloatFormat<T>(), value);
			if (length < 0 || static_cast<size_t>(length) >= maxNumberLength) {
				throw std::runtime_error("Internal error, insufficient number buffer");
			}
			target.Commit(Convert::Detail::ReplaceLocaleDecimalPointWithDot(out, static_cast<size_t>(length)));
		}
		else
		{
			WriteNumberChars(target, out, value);
		}
#endif
	}

	inline void WriteString(std::string_view source, StringBuffer& target)
	{
		static constexpr char hexChars[] = "0123456789ABCDEF";
		const size_t firstEscape = FindFirstEscape(source, 0);

		if (firstEscape == std::string_view::npos)
		{
			const size_t size = source.size() + 2;
			target.Ensure(size);
			char* out = target.Tail();
			const char* const start = out;
			*out++ = '"';
			if (!source.empty()) {
				std::memcpy(out, source.data(), source.size());
			}
			out += source.size();
			*out++ = '"';
			target.Commit(static_cast<size_t>(out - start));
			return;
		}

		// Worst case is 5 extra bytes per escaped byte; shift+add avoids a multiply.
		const size_t escapeRange = source.size() - firstEscape;
		const size_t maxSize = source.size() + 2 + (escapeRange << 2) + escapeRange;
		target.Ensure(maxSize);
		char* const start = target.Tail();
		char* out = start;
		*out++ = '"';

		size_t pos = 0;
		size_t escapePos = firstEscape;
		do
		{
			if (escapePos > pos)
			{
				const size_t chunkSize = escapePos - pos;
				std::memcpy(out, source.data() + pos, chunkSize);
				out += chunkSize;
			}

			const unsigned char uc = static_cast<unsigned char>(source[escapePos]);
			switch (uc)
			{
			case '"':  *out++ = '\\'; *out++ = '"';  break;
			case '\\': *out++ = '\\'; *out++ = '\\'; break;
			case '\b': *out++ = '\\'; *out++ = 'b';  break;
			case '\f': *out++ = '\\'; *out++ = 'f';  break;
			case '\n': *out++ = '\\'; *out++ = 'n';  break;
			case '\r': *out++ = '\\'; *out++ = 'r';  break;
			case '\t': *out++ = '\\'; *out++ = 't';  break;
			default:
				*out++ = '\\';
				*out++ = 'u';
				*out++ = '0';
				*out++ = '0';
				*out++ = hexChars[uc >> 4];
				*out++ = hexChars[uc & 0x0F];
				break;
			}

			pos = escapePos + 1;
			escapePos = FindFirstEscape(source, pos);
		} while (escapePos != std::string_view::npos);

		if (pos < source.size())
		{
			const size_t chunkSize = source.size() - pos;
			std::memcpy(out, source.data() + pos, chunkSize);
			out += chunkSize;
		}
		*out++ = '"';
		target.Commit(static_cast<size_t>(out - start));
	}

	inline void WriteString(std::string_view source, std::string& target)
	{
		static constexpr char hexChars[] = "0123456789ABCDEF";
		const size_t origSize = target.size();

		// Fast path: nothing to escape (the common case). Reserve the exact size and copy once.
		const size_t firstEscape = FindFirstEscape(source, 0);
		if (firstEscape == std::string_view::npos)
		{
			target.reserve(origSize + 2 + source.size());
			target.push_back('"');
			target.append(source);
			target.push_back('"');
			return;
		}

		// Escaped path: reserve for the worst case (every byte at or after the first escape expands
		// to "\u00XX", i.e. 6 bytes, or 5 extra bytes) so that no reallocation happens while emitting.
		const size_t escapeRange = source.size() - firstEscape;
		target.reserve(origSize + 2 + source.size() + (escapeRange << 2) + escapeRange);
		target.push_back('"');

		size_t pos = 0;
		size_t escapePos = firstEscape;
		do
		{
			if (escapePos > pos) {
				target.append(source.data() + pos, escapePos - pos);
			}
			const unsigned char uc = static_cast<unsigned char>(source[escapePos]);
			switch (uc)
			{
			case '"':  target.append("\\\""); break;
			case '\\': target.append("\\\\"); break;
			case '\b': target.append("\\b"); break;
			case '\f': target.append("\\f"); break;
			case '\n': target.append("\\n"); break;
			case '\r': target.append("\\r"); break;
			case '\t': target.append("\\t"); break;
			default:
				{
					const char unicodeEscape[6] = { '\\', 'u', '0', '0', hexChars[uc >> 4], hexChars[uc & 0x0F] };
					target.append(unicodeEscape, 6);
				}
				break;
			}
			pos = escapePos + 1;
			escapePos = FindFirstEscape(source, pos);
		}
		while (escapePos != std::string_view::npos);

		if (pos < source.size()) {
			target.append(source.data() + pos, source.size() - pos);
		}
		target.push_back('"');
	}
}
