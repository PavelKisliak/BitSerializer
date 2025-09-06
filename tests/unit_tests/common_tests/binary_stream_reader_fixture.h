/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include <gtest/gtest.h>
#include "common/binary_stream_reader.h"

class BinaryStreamReaderTest : public ::testing::Test
{
public:
	// Use minimal chunk size for simplify testing all streaming cases
	using reader_type = BitSerializer::Detail::CBinaryStreamReader;

	void PrepareStreamReader(size_t testSize)
	{
		mInputString.clear();
		for (size_t i = 0; i < testSize; ++i)
		{
			mInputString.push_back(static_cast < char>('A' + i % 26));
		}

		// Prepare stream reader
		mInputStream = std::stringstream(mInputString);
		mBinaryStreamReader = std::make_shared<reader_type>(mInputStream);
	}

	[[nodiscard]] std::string ReadByChunks(size_t testSize) const
	{
		std::string actual;
		const size_t maxIterations = testSize / reader_type::chunk_size + 1;
		size_t remainingSize = testSize;
		for (size_t i = 0; i < maxIterations; ++i)
		{
			if (std::string_view chunk = mBinaryStreamReader->ReadUpTo(remainingSize); !chunk.empty())
			{
				actual += chunk;
				remainingSize -= chunk.size();
			}
			else
			{
				break;
			}
		}
		return actual;
	}

protected:
	std::string mInputString;
	std::stringstream mInputStream;
	std::shared_ptr<reader_type> mBinaryStreamReader;
};
