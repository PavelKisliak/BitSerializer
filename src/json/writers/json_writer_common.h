/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <string_view>
#include "common/simd/scan.h"

namespace BitSerializer::Json::Detail
{
	/**
	 * @brief Returns the index of the first byte in `source` at or after `pos` that must be escaped inside
	 * a JSON string, i.e. `"`, `\` or a control byte below 0x20, or `std::string_view::npos` if there is none.
	 */
	[[nodiscard]] inline size_t FindFirstEscape(std::string_view source, size_t pos) noexcept
	{
		return BitSerializer::Detail::FindFirstOfOrLessThan<0x20, '"', '\\'>(source, pos);
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
		// to "\u00XX", i.e. 6 bytes) so that no reallocation happens while emitting.
		target.reserve(origSize + 2 + source.size() + 5 * (source.size() - firstEscape));
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
