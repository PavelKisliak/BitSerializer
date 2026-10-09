/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <string_view>
#include "common/simd/scan.h"

using BitSerializer::Detail::FindFirstOf;
using BitSerializer::Detail::FindFirstOfOrLessThan;

namespace
{
	constexpr size_t npos = std::string_view::npos;

	// Verifies that `fn` (a specific `FindFirstOf<...>` instantiation) returns exactly the same
	// result as the generic `std::string_view::find_first_of` for every start position.
	template <typename TCallable>
	void ExpectMatches(std::string_view input, std::string_view needle, TCallable fn)
	{
		for (size_t pos = 0; pos <= input.size() + 1; ++pos)
		{
			EXPECT_EQ(input.find_first_of(needle, pos), fn(input, pos))
				<< "input=\"" << std::string(input) << "\" pos=" << pos;
		}
	}

	// Reference implementation of `FindFirstOfOrLessThan` (scalar, no SIMD).
	size_t RefFindFirstOfOrLessThan(std::string_view input, unsigned char limit, std::string_view chars, size_t pos)
	{
		for (size_t i = pos; i < input.size(); ++i)
		{
			const unsigned char uc = static_cast<unsigned char>(input[i]);
			if (uc < limit || chars.find(input[i]) != std::string_view::npos) {
				return i;
			}
		}
		return npos;
	}

	template <typename TCallable>
	void ExpectLessThanMatches(std::string_view input, unsigned char limit, std::string_view chars, TCallable fn)
	{
		for (size_t pos = 0; pos <= input.size() + 1; ++pos)
		{
			EXPECT_EQ(RefFindFirstOfOrLessThan(input, limit, chars, pos), fn(input, pos))
				<< "len=" << input.size() << " pos=" << pos;
		}
	}

	const std::string TestInputs[] = {
		"",
		"a",
		"\"",
		"\\",
		",",
		"abc",
		"abc\"def",
		"abc\\def",
		"\\\"",
		"a\\b\"c",
		"a,b\nc\rd\"e",
		std::string(15, 'a') + "\"",                             // match at offset 15
		std::string(16, 'a') + "\"",                             // match at offset 16 (2nd block)
		std::string(17, 'a') + "\\" + "tail",                    // match at offset 17
		std::string(1000, 'a') + "\"",                           // long, no escapes
		std::string(1000, 'a') + "\\" + std::string(50, 'b'),    // long, escape late
		"\\" + std::string(1000, 'a') + "\"",                    // long, escape early
		"0123456789abcdefghijklmnopqrstuvwxyz",                  // no match
	};

	// Inputs with control bytes (< 0x20), the 0x1F/0x20 boundary, DEL (0x7F) and high bytes (>= 0x80).
	const std::string EscapeInputs[] = {
		std::string(1, '\x00'),
		std::string(1, '\x1f'),
		std::string("abc") + '\x1f',
		std::string("abc") + '\x20',                             // 0x20 is not a match
		std::string("abc") + '\x7f',                             // DEL is not a match
		std::string("abc") + static_cast<char>(0x80),            // high byte is not a match
		std::string("abc") + static_cast<char>(0xff),
		std::string(15, 'a') + '\x01',                           // match at offset 15
		std::string(16, 'a') + '\x01',                           // match at offset 16
		std::string(17, 'a') + '\t' + "tail",                    // match at offset 17
		std::string(1000, 'a') + '\x02',                         // long, no match until the end
		std::string(1000, 'a') + static_cast<char>(0x80) + '"',  // high bytes ignored, quote near the end
	};
}

// JSON: quote and backslash.
TEST(SimdScan, JsonQuoteAndBackslash)
{
	for (const auto& input : TestInputs)
	{
		const std::string_view s(input);
		ExpectMatches(s, "\"\\", [](std::string_view v, size_t p) { return FindFirstOf<'"', '\\'>(v, p); });
	}
}

// Single character.
TEST(SimdScan, SingleChar)
{
	for (const auto& input : TestInputs)
	{
		const std::string_view s(input);
		ExpectMatches(s, ",", [](std::string_view v, size_t p) { return FindFirstOf<','>(v, p); });
	}
}

// CSV-like set (comma, quote, CR, LF).
TEST(SimdScan, CsvLikeSet)
{
	for (const auto& input : TestInputs)
	{
		const std::string_view s(input);
		ExpectMatches(s, ",\"\r\n", [](std::string_view v, size_t p) { return FindFirstOf<',', '"', '\r', '\n'>(v, p); });
	}
}

// Empty range and positions at/after the end must return npos.
TEST(SimdScan, EdgeCases)
{
	EXPECT_EQ(npos, (FindFirstOf<'"', '\\'>(std::string_view{}, 0)));
	EXPECT_EQ(npos, (FindFirstOf<'"', '\\'>(std::string_view("abc"), 3)));
	EXPECT_EQ(npos, (FindFirstOf<'"', '\\'>(std::string_view("abc"), 100)));
	EXPECT_EQ(0u, (FindFirstOf<'"', '\\'>(std::string_view("\"abc"), 0)));
	EXPECT_EQ(2u, (FindFirstOf<'"', '\\'>(std::string_view("ab\\c"), 0)));
	EXPECT_EQ(npos, (FindFirstOf<'"', '\\'>(std::string_view("abc"), 1)));
}

// Pseudo-random inputs with occasional special characters.
TEST(SimdScan, Randomized)
{
	uint32_t state = 0x12345678u;
	auto rnd = [&state]() { state = state * 1664525u + 1013904223u; return state; };
	for (int iter = 0; iter < 20000; ++iter)
	{
		const size_t len = rnd() % 200;
		std::string s;
		s.reserve(len);
		for (size_t i = 0; i < len; ++i)
		{
			const uint32_t r = rnd() % 100;
			if (r == 0)
			{
				s.push_back('"');
			}
			else if (r == 1)
			{
				s.push_back('\\');
			}
			else if (r == 2)
			{
				s.push_back(',');
			}
			else if (r == 3)
			{
				s.push_back('\n');
			}
			else
			{
				s.push_back(static_cast<char>('a' + (r % 26)));
			}
		}
		const size_t pos = len ? (rnd() % (len + 1)) : 0;
		const std::string_view v(s);
		EXPECT_EQ(v.find_first_of("\"\\", pos), (FindFirstOf<'"', '\\'>(v, pos)));
		EXPECT_EQ(v.find_first_of(",\"\r\n", pos), (FindFirstOf<',', '"', '\r', '\n'>(v, pos)));
	}
}

//------------------------------------------------------------------------------
// FindFirstOfOrLessThan
//------------------------------------------------------------------------------

// JSON escaping predicate: `"`, `\` or a control byte below 0x20.
TEST(SimdScan, LessThanJsonEscapeSet)
{
	for (const auto& input : TestInputs)
	{
		const std::string_view s(input);
		ExpectLessThanMatches(s, 0x20, "\"\\", [](std::string_view v, size_t p) { return FindFirstOfOrLessThan<0x20, '"', '\\'>(v, p); });
	}
}

// Control bytes, the 0x1F/0x20 boundary, DEL (0x7F) and high bytes (>= 0x80).
TEST(SimdScan, LessThanControlAndHighBytes)
{
	for (const auto& input : EscapeInputs)
	{
		const std::string_view s(input);
		ExpectLessThanMatches(s, 0x20, "\"\\", [](std::string_view v, size_t p) { return FindFirstOfOrLessThan<0x20, '"', '\\'>(v, p); });
	}
}

// Zero limit: only the explicit characters can match.
TEST(SimdScan, LessThanZeroLimit)
{
	for (const auto& input : TestInputs)
	{
		const std::string_view s(input);
		ExpectLessThanMatches(s, 0x00, "\",", [](std::string_view v, size_t p) { return FindFirstOfOrLessThan<0, '"', ','>(v, p); });
	}
}

// Empty character set: only the `< limit` part can match.
TEST(SimdScan, LessThanEmptySet)
{
	for (const auto& input : EscapeInputs)
	{
		const std::string_view s(input);
		ExpectLessThanMatches(s, 0x20, "", [](std::string_view v, size_t p) { return FindFirstOfOrLessThan<0x20>(v, p); });
	}
}

// Empty range and positions at/after the end must return npos.
TEST(SimdScan, LessThanEdgeCases)
{
	EXPECT_EQ(npos, (FindFirstOfOrLessThan<0x20, '"', '\\'>(std::string_view{}, 0)));
	EXPECT_EQ(npos, (FindFirstOfOrLessThan<0x20, '"', '\\'>(std::string_view("abc"), 3)));
	EXPECT_EQ(npos, (FindFirstOfOrLessThan<0x20, '"', '\\'>(std::string_view("abc"), 100)));
	EXPECT_EQ(npos, (FindFirstOfOrLessThan<0x20, '"', '\\'>(std::string_view("\x20\x7f"), 0)));
	EXPECT_EQ(0u, (FindFirstOfOrLessThan<0x20, '"', '\\'>(std::string_view("\x1f"), 0)));
}

// Pseudo-random inputs mixing controls, high bytes, quotes and letters.
TEST(SimdScan, LessThanRandomized)
{
	uint32_t state = 0x9E3779B9u;
	auto rnd = [&state]() { state = state * 1664525u + 1013904223u; return state; };
	for (int iter = 0; iter < 20000; ++iter)
	{
		const size_t len = rnd() % 200;
		std::string s;
		s.reserve(len);
		for (size_t i = 0; i < len; ++i)
		{
			const uint32_t r = rnd() % 100;
			if (r < 10) {
				s.push_back(static_cast<char>(rnd() % 0x20));                        // control byte
			}
			else if (r < 20) {
				s.push_back(static_cast<char>(0x7f + (rnd() % 0x80)));               // DEL / high byte
			}
			else if (r < 25) {
				s.push_back((rnd() & 1) ? '"' : '\\');                               // quote / backslash
			}
			else {
				s.push_back(static_cast<char>('a' + (rnd() % 26)));
			}
		}
		const size_t pos = len ? (rnd() % (len + 1)) : 0;
		const std::string_view v(s);
		EXPECT_EQ(RefFindFirstOfOrLessThan(v, 0x20, "\"\\", pos), (FindFirstOfOrLessThan<0x20, '"', '\\'>(v, pos)));
	}
}
