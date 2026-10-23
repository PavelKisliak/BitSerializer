/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <array>
#include <string>
#include <type_traits>

#include <gtest/gtest.h>
#include "bitserializer/common/memory.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test EqualBytes() for runtime-sized ranges
//-----------------------------------------------------------------------------
TEST(MemoryEqualBytesTest, ShouldCompareRangesForAllLengthsAndMismatchPositions)
{
	constexpr size_t maxLength = 40;
	for (size_t length = 0; length <= maxLength; ++length)
	{
		const std::string lhs(length, 'a');
		const std::string rhs(length, 'a');
		EXPECT_TRUE(Memory::EqualBytes(lhs.data(), rhs.data(), length)) << "length " << length;

		for (size_t pos = 0; pos < length; ++pos)
		{
			std::string different = rhs;
			different[pos] = 'b';
			EXPECT_FALSE(Memory::EqualBytes(lhs.data(), different.data(), length))
				<< "length " << length << ", pos " << pos;
		}
	}
}

TEST(MemoryEqualBytesTest, ShouldHandleZeroLength)
{
	EXPECT_TRUE(Memory::EqualBytes(nullptr, nullptr, 0));
}

//-----------------------------------------------------------------------------
// Test EqualBytes<Count>() for compile-time-sized ranges
//-----------------------------------------------------------------------------
TEST(MemoryEqualBytesTest, ShouldCompareFixedSizeRanges)
{
	EXPECT_TRUE(Memory::EqualBytes<0>("", ""));

	EXPECT_TRUE(Memory::EqualBytes<7>("Member7", "Member7"));
	EXPECT_FALSE(Memory::EqualBytes<7>("Member7", "Member8"));

	EXPECT_TRUE(Memory::EqualBytes<8>("Member_0", "Member_0"));
	EXPECT_FALSE(Memory::EqualBytes<8>("Member_0", "Member_1"));

	EXPECT_TRUE(Memory::EqualBytes<9>("Member_01", "Member_01"));
	EXPECT_FALSE(Memory::EqualBytes<9>("Member_01", "Member_02"));

	EXPECT_TRUE(Memory::EqualBytes<16>("UnsignedIntValue", "UnsignedIntValue"));
	EXPECT_FALSE(Memory::EqualBytes<16>("UnsignedIntValue", "UnsignedIntValuE"));

	EXPECT_TRUE(Memory::EqualBytes<26>("StringWithLongKeyAndValue", "StringWithLongKeyAndValue"));
	EXPECT_FALSE(Memory::EqualBytes<26>("StringWithLongKeyAndValue", "StringWithLongKeyAndValuE"));
}

TEST(MemoryEqualBytesTest, ShouldCompareSseAndMemcmpBoundaries)
{
	std::array<char, 300> lhs{};
	std::array<char, 300> rhs{};
	lhs.fill('a');
	rhs = lhs;

	auto check = [&](auto countTag)
	{
		constexpr size_t Count = decltype(countTag)::value;
		EXPECT_TRUE(Memory::EqualBytes<Count>(lhs.data(), rhs.data())) << "count " << Count;

		for (const size_t pos : { size_t(0), Count / 2, Count - 1 })
		{
			rhs[pos] = 'b';
			EXPECT_FALSE(Memory::EqualBytes<Count>(lhs.data(), rhs.data()))
				<< "count " << Count << ", pos " << pos;
			rhs = lhs;
		}
	};

	// Clang-specific `memcmp` range and the scalar/SSE2 boundaries.
	check(std::integral_constant<size_t, 17>{});
	check(std::integral_constant<size_t, 31>{});

	// SSE2 path, including non-multiple-of-16 tails.
	check(std::integral_constant<size_t, 32>{});
	check(std::integral_constant<size_t, 33>{});
	check(std::integral_constant<size_t, 63>{});
	check(std::integral_constant<size_t, 64>{});
	check(std::integral_constant<size_t, 255>{});
	check(std::integral_constant<size_t, 256>{});

	// `memcmp` fallback for very long ranges.
	check(std::integral_constant<size_t, 257>{});
	check(std::integral_constant<size_t, 300>{});
}
