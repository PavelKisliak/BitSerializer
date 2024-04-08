/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "binary_stream_reader.h"
#include <cassert>
#include <cstring>
#include <algorithm>

namespace BitSerializer::Detail
{
	CBinaryStreamReader::CBinaryStreamReader(std::istream& inputStream)
		: mStream(inputStream)
	{
		ReadNextChunk();
	}

	bool CBinaryStreamReader::IsEnd() const noexcept
	{
		return mStartDataPtr == mEndDataPtr && mStream.eof();
	}

	bool CBinaryStreamReader::IsFailed() const noexcept
	{
		return mStream.fail();
	}

	size_t CBinaryStreamReader::GetPosition() const noexcept
	{
		return mStreamPos - (mEndDataPtr - mStartDataPtr);
	}

	bool CBinaryStreamReader::SetPosition(size_t pos)
	{
		const size_t cachedSize = mEndDataPtr - mBuffer;
		if (pos >= mStreamPos - cachedSize && pos < mStreamPos)
		{
			const auto chunkOffset = pos - (mStreamPos - cachedSize);
			mStartDataPtr = mBuffer + chunkOffset;
			return true;
		}

		if (pos == mStreamPos || !mStream.seekg(static_cast<std::streamoff>(pos)).fail())
		{
			mStreamPos = pos;
			// Invalidate cache
			mStartDataPtr = mEndDataPtr = mBuffer;
			ReadNextChunk();
			return true;
		}

		return false;
	}

	std::optional<char> CBinaryStreamReader::PeekByte()
	{
		if (mStartDataPtr != mEndDataPtr || ReadNextChunk()) {
			return std::make_optional<char>(*mStartDataPtr);
		}
		return std::nullopt;
	}

	void CBinaryStreamReader::GotoNextByte()
	{
		if (mStartDataPtr != mEndDataPtr || ReadNextChunk())
		{
			if (++mStartDataPtr == mEndDataPtr) {
				ReadNextChunk();
			}
		}
	}

	std::optional<char> CBinaryStreamReader::ReadByte()
	{
		if (mStartDataPtr != mEndDataPtr || ReadNextChunk())
		{
			const auto result = std::make_optional<char>(*mStartDataPtr++);
			if (mStartDataPtr == mEndDataPtr) {
				ReadNextChunk();
			}
			return result;
		}
		return std::nullopt;
	}

	std::string_view CBinaryStreamReader::ReadSolidBlock(size_t blockSize)
	{
		if (blockSize > chunk_size)
		{
			assert(blockSize > chunk_size);
			return {};
		}

		if (mStartDataPtr + blockSize > mEndDataPtr)
		{
			if (!ReadNextChunk() || mStartDataPtr + blockSize > mEndDataPtr)
			{
				return {};
			}
		}

		const std::string_view block(mStartDataPtr, blockSize);
		mStartDataPtr += blockSize;
		if (mStartDataPtr == mEndDataPtr)
		{
			// For correctly work IsEnd()
			mStream.peek();
		}
		return block;
	}

	std::string_view CBinaryStreamReader::ReadByChunks(size_t remainingSize)
	{
		if (mStartDataPtr != mEndDataPtr || ReadNextChunk())
		{
			const size_t chunkSize = std::min(static_cast<size_t>(mEndDataPtr - mStartDataPtr), remainingSize);
			const std::string_view block(mStartDataPtr, chunkSize);
			mStartDataPtr += chunkSize;
			if (mStartDataPtr == mEndDataPtr)
			{
				// For correctly work IsEnd()
				mStream.peek();
			}
			return block;
		}
		return {};
	}

	bool CBinaryStreamReader::ReadNextChunk()
	{
		if (IsEnd())
		{
			return false;
		}

		if (mStartDataPtr == mEndBufferPtr)
		{
			mStartDataPtr = mEndDataPtr = mBuffer;
		}
		else if (mStartDataPtr != mBuffer)
		{
			// Squeeze buffer
			std::memcpy(mBuffer, mStartDataPtr, mEndDataPtr - mStartDataPtr);
			mEndDataPtr -= mStartDataPtr - mBuffer;
			mStartDataPtr = mBuffer;
		}

		// Read next chunk
		mStream.read(mEndDataPtr, mEndBufferPtr - mEndDataPtr);
		const auto lastReadSize = mStream.gcount();
		mEndDataPtr += lastReadSize;
		mStreamPos += lastReadSize;
		assert(mStartDataPtr >= mBuffer && mStartDataPtr <= mEndDataPtr);
		return lastReadSize != 0;
	}
}
