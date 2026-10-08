/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <string_view>
#include "common/byte_scan.h"

using BitSerializer::Detail::FindFirstOf;

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
}

// JSON: quote and backslash.
TEST(ByteScan, JsonQuoteAndBackslash)
{
	for (const auto& input : TestInputs)
	{
		const std::string_view s(input);
		ExpectMatches(s, "\"\\", [](std::string_view v, size_t p) { return FindFirstOf<'"', '\\'>(v, p); });
	}
}

// Single character.
TEST(ByteScan, SingleChar)
{
	for (const auto& input : TestInputs)
	{
		const std::string_view s(input);
		ExpectMatches(s, ",", [](std::string_view v, size_t p) { return FindFirstOf<','>(v, p); });
	}
}

// CSV-like set (comma, quote, CR, LF).
TEST(ByteScan, CsvLikeSet)
{
	for (const auto& input : TestInputs)
	{
		const std::string_view s(input);
		ExpectMatches(s, ",\"\r\n", [](std::string_view v, size_t p) { return FindFirstOf<',', '"', '\r', '\n'>(v, p); });
	}
}

// Empty range and positions at/after the end must return npos.
TEST(ByteScan, EdgeCases)
{
	EXPECT_EQ(npos, (FindFirstOf<'"', '\\'>(std::string_view{}, 0)));
	EXPECT_EQ(npos, (FindFirstOf<'"', '\\'>(std::string_view("abc"), 3)));
	EXPECT_EQ(npos, (FindFirstOf<'"', '\\'>(std::string_view("abc"), 100)));
	EXPECT_EQ(0u, (FindFirstOf<'"', '\\'>(std::string_view("\"abc"), 0)));
	EXPECT_EQ(2u, (FindFirstOf<'"', '\\'>(std::string_view("ab\\c"), 0)));
	EXPECT_EQ(npos, (FindFirstOf<'"', '\\'>(std::string_view("abc"), 1)));
}

// Pseudo-random inputs with occasional special characters.
TEST(ByteScan, Randomized)
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
