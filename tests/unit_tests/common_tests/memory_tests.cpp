/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <string>

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
