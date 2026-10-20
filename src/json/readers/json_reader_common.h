/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include "bitserializer/serialization_detail/errors_handling.h"
#include "common/simd/scan.h"

namespace BitSerializer::Json::Detail
{
	/// Maximum number of characters required to parse any valid JSON number
	/// (including sign, decimal point and exponent). Used as the scanning window
	/// in both string and stream readers to keep number handling consistent.
	inline constexpr size_t MaxNumberLength = 64;

	/// Byte classes used by the number scanner and the hot-path validation.
	enum JsonCharFlags : uint8_t
	{
		JsonCharDigit = 1u << 0,
		JsonCharTerminator = 1u << 1,
		JsonCharFloatStart = 1u << 2
	};

	/// Single 256-byte lookup table shared by the hot validation path (cheaper than chains of compares).
	inline constexpr std::array<uint8_t, 256> JsonCharClasses = []
	{
		std::array<uint8_t, 256> table{};
		for (int c = '0'; c <= '9'; ++c)
		{
			table[static_cast<size_t>(c)] |= JsonCharDigit;
		}
		table[static_cast<size_t>(' ')] |= JsonCharTerminator;
		table[static_cast<size_t>('\t')] |= JsonCharTerminator;
		table[static_cast<size_t>('\r')] |= JsonCharTerminator;
		table[static_cast<size_t>('\n')] |= JsonCharTerminator;
		table[static_cast<size_t>(',')] |= JsonCharTerminator;
		table[static_cast<size_t>(']')] |= JsonCharTerminator;
		table[static_cast<size_t>('}')] |= JsonCharTerminator;
		table[static_cast<size_t>('/')] |= JsonCharTerminator;
		table[static_cast<size_t>('.')] |= JsonCharFloatStart;
		table[static_cast<size_t>('e')] |= JsonCharFloatStart;
		table[static_cast<size_t>('E')] |= JsonCharFloatStart;
		return table;
	}();

	[[nodiscard]] constexpr bool IsJsonDigit(char c) noexcept
	{
		return (JsonCharClasses[static_cast<uint8_t>(c)] & JsonCharDigit) != 0;
	}

	/// Returns `true` when `c` can legally follow a JSON number
	/// (whitespace, a structural delimiter, a comment start (JSONC) or the end of input).
	/// Any other character would make the number part of a larger, invalid token such as `0x10` or `12abc`.
	[[nodiscard]] constexpr bool IsJsonNumberTerminator(char c) noexcept
	{
		return (JsonCharClasses[static_cast<uint8_t>(c)] & JsonCharTerminator) != 0;
	}

	/// Returns `true` for `.`, `e` and `E`, i.e. the characters that make an integral token a floating-point one.
	[[nodiscard]] constexpr bool IsJsonFloatStart(char c) noexcept
	{
		return (JsonCharClasses[static_cast<uint8_t>(c)] & JsonCharFloatStart) != 0;
	}

	/// Result of a JSON number scan, see `ScanJsonNumber()`.
	enum class JsonNumberScanResult
	{
		Valid,
		Invalid,
		NeedMore
	};

	/// Details of a scanned JSON number, see `ScanJsonNumber()`.
	struct JsonNumberInfo
	{
		size_t length = 0;
		size_t errorOffset = 0;
	};

	/**
	 * @brief Scans a JSON number (RFC 8259) at the beginning of `data`.
	 *
	 * Unlike `std::from_chars`, this validates the strict JSON grammar: no leading `+`,
	 * no leading zeros, no `Infinity`/`NaN`, and a digit is required on both sides of
	 * the decimal point and in the exponent.
	 *
	 * @param data       Input buffer starting at the first character of the number.
	 * @param isComplete `true` when `data` extends to the end of the input, so a number
	 *                   ending at `data.size()` is complete. `false` when more data may
	 *                   follow (the scanner then reports `NeedMore` instead of guessing).
	 * @param info       Populated with the token length and error offset.
	 * @return `Valid`, `Invalid` or `NeedMore` (only when `isComplete == false`).
	 */
	[[nodiscard]] inline JsonNumberScanResult ScanJsonNumber(std::string_view data, bool isComplete, JsonNumberInfo& info) noexcept
	{
		info = JsonNumberInfo{};
		const size_t size = data.size();
		size_t pos = 0;

		if (pos < size && data[pos] == '-')
		{
			++pos;
		}
		if (pos == size)
		{
			info.errorOffset = pos;
			return isComplete ? JsonNumberScanResult::Invalid : JsonNumberScanResult::NeedMore;
		}

		// Integer part
		if (data[pos] == '0')
		{
			++pos;
			if (pos < size && IsJsonDigit(data[pos]))
			{
				info.errorOffset = pos;
				return JsonNumberScanResult::Invalid; // Leading zero
			}
		}
		else if (data[pos] >= '1' && data[pos] <= '9')
		{
			++pos;
			while (pos < size && IsJsonDigit(data[pos]))
			{
				++pos;
			}
		}
		else
		{
			info.errorOffset = pos;
			return JsonNumberScanResult::Invalid;
		}

		if (pos == size)
		{
			if (!isComplete)
			{
				return JsonNumberScanResult::NeedMore;
			}
			info.length = pos;
			return JsonNumberScanResult::Valid;
		}

		// Fraction part
		if (data[pos] == '.')
		{
			++pos;
			if (pos == size)
			{
				info.errorOffset = pos;
				return isComplete ? JsonNumberScanResult::Invalid : JsonNumberScanResult::NeedMore;
			}
			if (!IsJsonDigit(data[pos]))
			{
				info.errorOffset = pos;
				return JsonNumberScanResult::Invalid;
			}
			while (pos < size && IsJsonDigit(data[pos]))
			{
				++pos;
			}
			if (pos == size)
			{
				if (!isComplete)
				{
					return JsonNumberScanResult::NeedMore;
				}
				info.length = pos;
				return JsonNumberScanResult::Valid;
			}
		}

		// Exponent part
		if (data[pos] == 'e' || data[pos] == 'E')
		{
			++pos;
			if (pos < size && (data[pos] == '+' || data[pos] == '-'))
			{
				++pos;
			}
			if (pos == size)
			{
				info.errorOffset = pos;
				return isComplete ? JsonNumberScanResult::Invalid : JsonNumberScanResult::NeedMore;
			}
			if (!IsJsonDigit(data[pos]))
			{
				info.errorOffset = pos;
				return JsonNumberScanResult::Invalid;
			}
			while (pos < size && IsJsonDigit(data[pos]))
			{
				++pos;
			}
			if (pos == size)
			{
				if (!isComplete)
				{
					return JsonNumberScanResult::NeedMore;
				}
				info.length = pos;
				return JsonNumberScanResult::Valid;
			}
		}

		// The number must be followed by a legal terminator, otherwise it is part of
		// a larger invalid token (e.g. "0x10", "12abc" or "1.2.3").
		if (!IsJsonNumberTerminator(data[pos]))
		{
			info.errorOffset = pos;
			return JsonNumberScanResult::Invalid;
		}

		info.length = pos;
		return JsonNumberScanResult::Valid;
	}

	/**
	 * @brief Validates a number that was already consumed by `std::from_chars`/`strtod`.
	 *
	 * Those converters follow the more permissive C grammar, so a few JSON-specific rules still
	 * have to be checked: no leading zeros, no `inf`/`nan`, a digit after the decimal point,
	 * and a legal terminator after the token. The checks are O(1); the decimal point is located
	 * by a short forward scan that normally stops after a couple of characters, so this keeps
	 * the hot path cheaper than a full grammar scan.
	 *
	 * @tparam IsFloatTarget `true` when the target type is floating-point.
	 * @param data           Buffer starting at the first character of the number.
	 * @param available      Number of characters available in the buffer (for the terminator check).
	 * @param length         Number of characters consumed by the converter.
	 * @param[out] errorOffset Offset of the first invalid character on failure.
	 * @return `true` when the token is a strictly valid JSON number.
	 */
	template <bool IsFloatTarget>
	[[nodiscard]] inline bool IsValidParsedNumber(const char* data, size_t available, size_t length, size_t& errorOffset) noexcept
	{
		errorOffset = 0;
		if (length == 0)
		{
			return false;
		}

		const size_t digitStart = data[0] == '-' ? 1 : 0;

		if constexpr (IsFloatTarget)
		{
			// The converter accepts "-inf" and "-.5", which are not JSON numbers.
			if (digitStart >= length || !IsJsonDigit(data[digitStart]))
			{
				errorOffset = digitStart;
				return false;
			}
		}

		// Leading zeros are not allowed (e.g. "007").
		if (data[digitStart] == '0' && digitStart + 1 < length && IsJsonDigit(data[digitStart + 1]))
		{
			errorOffset = digitStart + 1;
			return false;
		}

		// The number must be followed by a legal terminator, otherwise it is part of a larger
		// invalid token (e.g. "0x10", "12abc" or "1.2.3").
		if (length < available && !IsJsonNumberTerminator(data[length]))
		{
			errorOffset = length;
			return false;
		}

		if constexpr (IsFloatTarget)
		{
			// `from_chars` accepts "1." and "1.e5"; a fraction must have a digit after the point.
			// The loop stops at the first '.', so it is usually just a couple of iterations.
			const char* const end = data + length;
			for (const char* p = data + digitStart; p != end; ++p)
			{
				if (*p == '.')
				{
					if (p + 1 == end || !IsJsonDigit(p[1]))
					{
						errorOffset = static_cast<size_t>(p - data);
						return false;
					}
					break;
				}
			}
		}

		return true;
	}

	/// Lookup table for converting hex characters (0-9, a-f, A-F) to values 0-15.
	/// Non-hex characters map to 0xFF.
	inline constexpr uint8_t HexLUT[256] =
	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x7,  0x8,  0x9,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	0xFF,
		0xA,  0xB,  0xC,  0xD,  0xE,  0xF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xA,  0xB,  0xC,  0xD,  0xE,  0xF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

	[[nodiscard]] inline uint16_t ParseHex4(std::string_view input, size_t pos, size_t lineNumber)
	{
		if (pos + 4 > input.size()) {
			throw ParsingException("Unexpected end of input in string escape sequence", lineNumber, pos);
		}

		uint16_t codepoint = 0;
		for (int i = 0; i < 4; ++i)
		{
			const uint8_t hexVal = HexLUT[static_cast<uint8_t>(input[pos + i])];
			if (hexVal == 0xFF) {
				throw ParsingException("Invalid hex digit in \\u escape", lineNumber, pos + i);
			}
			codepoint = (codepoint << 4) | hexVal;
		}
		return codepoint;
	}

	inline void AppendUtf8Codepoint(std::string& buffer, uint32_t codepoint)
	{
		if (codepoint < 0x80)
		{
			buffer += static_cast<char>(codepoint);
		}
		else if (codepoint < 0x800)
		{
			buffer += static_cast<char>(0xC0 | (codepoint >> 6));
			buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else if (codepoint < 0x10000)
		{
			// BMP: 3 bytes
			buffer += static_cast<char>(0xE0 | (codepoint >> 12));
			buffer += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else
		{
			// Supplementary plane (surrogate pairs decoded): 4 bytes
			buffer += static_cast<char>(0xF0 | (codepoint >> 18));
			buffer += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
			buffer += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
	}
}
