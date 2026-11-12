/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/archive_stub.h"

#if defined(__cpp_lib_span) && __cpp_lib_span >= 202002L
#include <array>
#include <span>

#include "bitserializer/types/std/span.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for std::span
//-----------------------------------------------------------------------------
TEST(STD_Span, SerializeFixedExtent)
{
	const std::array<int, 3> expected{ 1, 2, 3 };
	std::span<const int, 3> source(expected);
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(source, outputArchive);

	std::array<int, 3> actual{};
	std::span<int, 3> target(actual);
	BitSerializer::LoadObject<ArchiveStub>(target, outputArchive);
	EXPECT_TRUE(expected == actual);
}

TEST(STD_Span, SerializeDynamicExtent)
{
	const std::array<int, 5> expected{ 1, 2, 3, 4, 5 };
	std::span<const int> source(expected);
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(source, outputArchive);

	std::array<int, 5> actual{};
	std::span<int> target(actual);
	BitSerializer::LoadObject<ArchiveStub>(target, outputArchive);
	EXPECT_TRUE(expected == actual);
}

TEST(STD_Span, SerializeConstSpan)
{
	const std::array<int, 3> expected{ 4, 5, 6 };
	std::span<const int> source(expected);
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(source, outputArchive);

	std::array<int, 3> actual{};
	std::span<int> target(actual);
	BitSerializer::LoadObject<ArchiveStub>(target, outputArchive);
	EXPECT_TRUE(expected == actual);
}

TEST(STD_Span, SerializeEmptySpan)
{
	std::span<const int> source;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(source, outputArchive);

	std::span<int> target;
	BitSerializer::LoadObject<ArchiveStub>(target, outputArchive);
	EXPECT_TRUE(target.empty());
}

TEST(STD_Span, ThrowWhenNumberOfItemsDoesNotMatch)
{
	const std::array<int, 3> expected{ 1, 2, 3 };
	std::span<const int> source(expected);
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(source, outputArchive);

	std::array<int, 2> actual{};
	std::span<int> target(actual);
	EXPECT_THROW(
		BitSerializer::LoadObject<ArchiveStub>(target, outputArchive),
		BitSerializer::SerializationException);
}

TEST(STD_Span, SerializeToBinaryArchive)
{
	const std::array<int, 4> expected{ 7, 8, 9, 10 };
	std::span<const int> source(expected);
	BinArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<BinArchiveStub>(source, outputArchive);

	std::array<int, 4> actual{};
	std::span<int> target(actual);
	BitSerializer::LoadObject<BinArchiveStub>(target, outputArchive);
	EXPECT_TRUE(expected == actual);
}
#endif
