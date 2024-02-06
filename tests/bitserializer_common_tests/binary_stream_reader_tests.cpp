/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "testing_tools/common_test_entities.h"
#include "binary_stream_reader_fixture.h"

using namespace BitSerializer::Detail;

static_assert((CBinaryStreamReader::chunk_size % 8) == 0, "Chunk size must be a multiple of 8");

//-----------------------------------------------------------------------------
TEST_F(BinaryStreamReaderTest, ShouldCheckIsEndWhenEmptyInputStream)
{
	// Arrange
	PrepareStreamReader(0);

	// Act / Assert
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldCheckIsEndWhenItInTheCachedChunk)
{
	// Arrange
	PrepareStreamReader(2);

	// Act / Assert
	ASSERT_FALSE(mBinaryStreamReader->IsEnd());
	ASSERT_TRUE(mBinaryStreamReader->ReadByte());
	ASSERT_FALSE(mBinaryStreamReader->IsEnd());
	ASSERT_TRUE(mBinaryStreamReader->ReadByte());
	ASSERT_TRUE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldCheckIsEndWhenInputDataGreaterThanChunk)
{
	// Arrange
	PrepareStreamReader(reader_type::chunk_size + 1);
	const auto skippedChunk = mBinaryStreamReader->ReadSolidBlock(reader_type::chunk_size);

	// Act / Assert
	ASSERT_FALSE(skippedChunk.empty());
	ASSERT_FALSE(mBinaryStreamReader->IsEnd());
	ASSERT_TRUE(mBinaryStreamReader->ReadByte());
	ASSERT_TRUE(mBinaryStreamReader->IsEnd());
}

//-----------------------------------------------------------------------------
TEST_F(BinaryStreamReaderTest, ShouldCheckIsFailedWhenSetWrongPosition)
{
	// Arrange
	PrepareStreamReader(0);

	// Act
	const bool result = mBinaryStreamReader->SetPosition(1);

	// Assert
	ASSERT_FALSE(result);
	EXPECT_EQ(true, mBinaryStreamReader->IsFailed());
}

//-----------------------------------------------------------------------------

TEST_F(BinaryStreamReaderTest, ShouldGetPositionAtStart)
{
	// Arrange
	PrepareStreamReader(1);

	// Act
	const auto actual = mBinaryStreamReader->GetPosition();

	// Assert
	ASSERT_EQ(0, actual);
}

TEST_F(BinaryStreamReaderTest, ShouldGetPositionAtMiddle)
{
	// Arrange
	PrepareStreamReader(4);
	const auto solidBlock = mBinaryStreamReader->ReadSolidBlock(2);

	// Act
	const auto actual = mBinaryStreamReader->GetPosition();

	// Assert
	ASSERT_FALSE(solidBlock.empty());
	ASSERT_EQ(2, actual);
}


TEST_F(BinaryStreamReaderTest, ShouldGetPositionAtEnd)
{
	// Arrange
	PrepareStreamReader(4);
	const auto solidBlock = mBinaryStreamReader->ReadSolidBlock(4);

	// Act
	const auto actual = mBinaryStreamReader->GetPosition();

	// Assert
	ASSERT_FALSE(solidBlock.empty());
	ASSERT_EQ(4, actual);
}

//-----------------------------------------------------------------------------

TEST_F(BinaryStreamReaderTest, ShouldSetPositionWhenItInTheCachedChunk)
{
	constexpr size_t testSize = 5;
	for (size_t testPos = 0; testPos < testSize; ++testPos)
	{
		// Arrange
		PrepareStreamReader(testSize);

		// Act
		const auto solidBlock = mBinaryStreamReader->ReadSolidBlock(2);
		const bool result = mBinaryStreamReader->SetPosition(testPos);
		const auto actualByte = mBinaryStreamReader->PeekByte();

		// Assert
		ASSERT_TRUE(result);
		ASSERT_FALSE(solidBlock.empty());
		EXPECT_EQ(testPos, mBinaryStreamReader->GetPosition());
		ASSERT_TRUE(actualByte);
		EXPECT_EQ(mInputString[testPos], *actualByte);
	}
}

TEST_F(BinaryStreamReaderTest, ShouldSetPositionWhenItAfterCachedChunk)
{
	// Arrange
	constexpr size_t testPos = reader_type::chunk_size + 1;
	PrepareStreamReader(testPos + 1);

	// Act
	const bool result = mBinaryStreamReader->SetPosition(testPos);
	const auto actualByte = mBinaryStreamReader->PeekByte();

	// Assert
	ASSERT_TRUE(result);
	EXPECT_EQ(testPos, mBinaryStreamReader->GetPosition());
	ASSERT_TRUE(actualByte);
	EXPECT_EQ(mInputString[testPos], *actualByte);
}

TEST_F(BinaryStreamReaderTest, ShouldSetPositionWhenItBeforeCachedChunk)
{
	// Arrange
	constexpr size_t testPos = 0;
	PrepareStreamReader(testPos + 1);

	// Act
	const auto solidBlock = mBinaryStreamReader->ReadSolidBlock(reader_type::chunk_size);
	const bool result = mBinaryStreamReader->SetPosition(testPos);
	const auto actualByte = mBinaryStreamReader->PeekByte();

	// Assert
	ASSERT_TRUE(result);
	ASSERT_TRUE(solidBlock.empty());
	EXPECT_EQ(testPos, mBinaryStreamReader->GetPosition());
	ASSERT_TRUE(actualByte);
	EXPECT_EQ(mInputString[testPos], *actualByte);
}

TEST_F(BinaryStreamReaderTest, ShouldSetPositionFailWhenItAfterTheEnd)
{
	// Arrange
	constexpr size_t testPos = 10;
	PrepareStreamReader(testPos - 1);

	// Act
	const bool result = mBinaryStreamReader->SetPosition(testPos);

	// Assert
	ASSERT_FALSE(result);
	EXPECT_EQ(0, mBinaryStreamReader->GetPosition());
}

//-----------------------------------------------------------------------------

TEST_F(BinaryStreamReaderTest, ShouldPeekByte)
{
	// Arrange
	PrepareStreamReader(2);

	// Act
	const auto actual1stPeek = mBinaryStreamReader->PeekByte();
	const auto actual2stPeek = mBinaryStreamReader->PeekByte();

	// Assert
	ASSERT_TRUE(actual1stPeek.has_value());
	ASSERT_TRUE(actual2stPeek.has_value());
	EXPECT_EQ(*actual1stPeek, *actual1stPeek);
	EXPECT_EQ(0, mBinaryStreamReader->GetPosition());
	EXPECT_FALSE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldPeekByteEmptyWhenNoMoreData)
{
	// Arrange
	PrepareStreamReader(0);

	// Act
	const auto actual = mBinaryStreamReader->PeekByte();

	// Assert
	ASSERT_FALSE(actual.has_value());
	EXPECT_EQ(0, mBinaryStreamReader->GetPosition());
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

//-----------------------------------------------------------------------------
TEST_F(BinaryStreamReaderTest, ShouldGotoNextByte)
{
	// Arrange
	PrepareStreamReader(2);

	// Act
	const auto actual1stPeek = mBinaryStreamReader->PeekByte();
	mBinaryStreamReader->GotoNextByte();
	const auto actual2stPeek = mBinaryStreamReader->PeekByte();

	// Assert
	ASSERT_TRUE(actual1stPeek.has_value());
	ASSERT_TRUE(actual2stPeek.has_value());
	EXPECT_EQ(*actual1stPeek, *actual1stPeek);
	EXPECT_EQ(1, mBinaryStreamReader->GetPosition());
	EXPECT_FALSE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldGotoNextByteWhenNoMoreData)
{
	// Arrange
	PrepareStreamReader(1);

	// Act
	const auto actual = mBinaryStreamReader->PeekByte();
	mBinaryStreamReader->GotoNextByte();

	// Assert
	ASSERT_TRUE(actual.has_value());
	EXPECT_EQ(1, mBinaryStreamReader->GetPosition());
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

//-----------------------------------------------------------------------------

TEST_F(BinaryStreamReaderTest, ShouldReadByte)
{
	// Arrange
	PrepareStreamReader(2);

	// Act
	const auto actual1stRead = mBinaryStreamReader->ReadByte();
	const auto actual2stRead = mBinaryStreamReader->ReadByte();

	// Assert
	ASSERT_TRUE(actual1stRead.has_value());
	ASSERT_TRUE(actual2stRead.has_value());
	EXPECT_NE(*actual1stRead, *actual2stRead);
	EXPECT_EQ(2, mBinaryStreamReader->GetPosition());
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldReadByteEmptyWhenEmptyStream)
{
	// Arrange
	PrepareStreamReader(0);

	// Act
	const auto actual = mBinaryStreamReader->ReadByte();

	// Assert
	ASSERT_FALSE(actual.has_value());
	EXPECT_EQ(0, mBinaryStreamReader->GetPosition());
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldReadByteEmptyWhenReachedEnd)
{
	// Arrange
	PrepareStreamReader(1);

	// Act
	const auto actual1stRead = mBinaryStreamReader->ReadByte();
	const auto actual2stRead = mBinaryStreamReader->ReadByte();

	// Assert
	ASSERT_TRUE(actual1stRead.has_value());
	ASSERT_FALSE(actual2stRead.has_value());
	EXPECT_EQ(1, mBinaryStreamReader->GetPosition());
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

//-----------------------------------------------------------------------------
TEST_F(BinaryStreamReaderTest, ShouldReadSolidBlockWhenSizeEqualToChunk)
{
	// Arrange
	PrepareStreamReader(reader_type::chunk_size);

	// Act
	const std::string actual(mBinaryStreamReader->ReadSolidBlock(reader_type::chunk_size));

	// Assert
	ASSERT_EQ(mInputString.size(), actual.size());
	EXPECT_EQ(mInputString, actual);
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldReadSolidBlockWhenSizeLessThanChunk)
{
	// Arrange
	constexpr size_t testBlockSize = 8;
	PrepareStreamReader(testBlockSize + 1);
	const std::string expected = mInputString.substr(0, testBlockSize);

	// Act
	const std::string actual(mBinaryStreamReader->ReadSolidBlock(testBlockSize));

	// Assert
	ASSERT_EQ(testBlockSize, actual.size());
	EXPECT_EQ(expected, actual);
	EXPECT_FALSE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldReadSolidBlockEmptyWhenNoMoreData)
{
	// Arrange
	PrepareStreamReader(0);

	// Act
	const std::string actual(mBinaryStreamReader->ReadSolidBlock(1));

	// Assert
	ASSERT_TRUE(actual.empty());
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldReadSolidBlockEmptyWhenInputDataSizeIsLess)
{
	// Arrange
	PrepareStreamReader(1);

	// Act
	const std::string actual(mBinaryStreamReader->ReadSolidBlock(2));

	// Assert
	ASSERT_TRUE(actual.empty());
	EXPECT_FALSE(mBinaryStreamReader->IsEnd());
}

//-----------------------------------------------------------------------------

TEST_F(BinaryStreamReaderTest, ShouldReadByChunkWhenSizeEqualToChunk)
{
	// Arrange
	constexpr size_t testSize = reader_type::chunk_size;
	PrepareStreamReader(testSize);

	// Act
	const std::string actual = ReadByChunks(testSize);

	// Assert
	ASSERT_FALSE(actual.empty());
	EXPECT_EQ(mInputString, actual);
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldReadByChunkWhenSizeLessThanChunk)
{
	// Arrange
	constexpr size_t testSize = reader_type::chunk_size - 1;
	PrepareStreamReader(testSize);

	// Act
	const std::string actual = ReadByChunks(testSize);

	// Assert
	ASSERT_FALSE(actual.empty());
	EXPECT_EQ(mInputString, actual);
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldReadByChunkWhenSizeGreaterThanChunk)
{
	// Arrange
	constexpr size_t testSize = reader_type::chunk_size + 1;
	PrepareStreamReader(testSize);

	// Act
	const std::string actual = ReadByChunks(testSize);

	// Assert
	ASSERT_FALSE(actual.empty());
	ASSERT_EQ(mInputString.size(), actual.size());
	EXPECT_EQ(mInputString, actual);
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldReadByChunkWhenMultipleChunks)
{
	// Arrange
	constexpr size_t testSize = reader_type::chunk_size * 3 + reader_type::chunk_size - 1;
	PrepareStreamReader(testSize);

	// Act
	const std::string actual = ReadByChunks(testSize);

	// Assert
	ASSERT_FALSE(actual.empty());
	ASSERT_EQ(mInputString.size(), actual.size());
	EXPECT_EQ(mInputString, actual);
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldReadByChunkEmptyWhenNoMoreData)
{
	// Arrange
	PrepareStreamReader(0);

	// Act
	const std::string actual = ReadByChunks(1);

	// Assert
	ASSERT_TRUE(actual.empty());
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}

TEST_F(BinaryStreamReaderTest, ShouldReadByChunkEmptyWhenInputDataSizeIsLess)
{
	// Arrange
	PrepareStreamReader(1);

	// Act
	const std::string actual = ReadByChunks(2);

	// Assert
	ASSERT_EQ(1, actual.size());
	EXPECT_EQ(mInputString, actual);
	EXPECT_TRUE(mBinaryStreamReader->IsEnd());
}
