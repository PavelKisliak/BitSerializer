/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "binary_stream_reader.h"
#include <cassert>
#include <cstring>
#include <istream>

namespace BitSerializer::Detail
{
	BinaryStreamReader::BinaryStreamReader(std::istream& inputStream)
		: mStream(inputStream)
	{
		ReadNextChunk();
	}

	bool BinaryStreamReader::IsEnd() const noexcept
	{
		return mStartDataPtr == mEndDataPtr && mStream.eof();
	}

	bool BinaryStreamReader::IsFailed() const noexcept
	{
		return mStream.fail();
	}

	size_t BinaryStreamReader::GetPosition() const noexcept
	{
		return mStreamPos - (mEndDataPtr - mStartDataPtr);
	}

	bool BinaryStreamReader::SetPosition(size_t pos)
	{
		const std::streamsize cachedSize = mEndDataPtr - mBuffer;
		if (pos >= mStreamPos - cachedSize && pos < mStreamPos)
		{
			const auto chunkOffset = pos - (mStreamPos - cachedSize);
			mStartDataPtr = mBuffer + chunkOffset;
			return true;
		}

		if (pos != mStreamPos)
		{
			// The stream may be in the eof/fail state after reading up to the end,
			// which would make seekg() a no-op, so reset the state before seeking.
			mStream.clear();
			if (mStream.seekg(static_cast<std::streamoff>(pos)).fail()) {
				return false;
			}
		}
		mStreamPos = pos;
		// Invalidate cache
		mStartDataPtr = mEndDataPtr = mBuffer;
		ReadNextChunk();
		return true;
	}

	std::string_view BinaryStreamReader::ReadExactly(size_t blockSize)
	{
		assert(blockSize <= chunk_size);
		if (blockSize > chunk_size) {
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

	std::string_view BinaryStreamReader::ReadUpTo(size_t maxBytesToRead)
	{
		if (mStartDataPtr != mEndDataPtr || ReadNextChunk())
		{
			const size_t chunkSize = std::min(static_cast<size_t>(mEndDataPtr - mStartDataPtr), maxBytesToRead);
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

	bool BinaryStreamReader::ReadNextChunk()
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
		const auto lastReadSize = static_cast<size_t>(mStream.gcount());
		mEndDataPtr += lastReadSize;
		mStreamPos += lastReadSize;
		assert(mStartDataPtr >= mBuffer && mStartDataPtr <= mEndDataPtr);
		return lastReadSize != 0;
	}
}
