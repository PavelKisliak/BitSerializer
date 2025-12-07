/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "csv_readers.h"


namespace BitSerializer::Csv::Detail
{
	CCsvStringReader::CCsvStringReader(std::string_view inputString, bool withHeader, char separator)
		: mSourceString(inputString)
		, mWithHeader(withHeader)
		, mSeparator(separator)
	{
		if (withHeader)
		{
			if (ParseNextLine())
			{
				mHeaders.resize(mRowValuesMeta.size());
				for (auto& header : mHeaders)
				{
					ReadValue(header);
				}
				// Lock the part of the buffer used to store decoded headers
				mLockedBufferSize = mTempValueBuffer.size();
			}
			else
			{
				throw ParsingException("Input string is empty, expected at least a header line");
			}
		}
	}

	bool CCsvStringReader::SeekToHeader(size_t headerIndex, std::string_view& out_header) noexcept
	{
		if (mWithHeader)
		{
			if (headerIndex < mRowValuesMeta.size())
			{
				out_header = mHeaders[headerIndex];
				mValueIndex = headerIndex;
				return true;
			}
		}
		return false;
	}

	bool CCsvStringReader::ReadValue(std::string_view key, std::string_view& out_value) noexcept
	{
		if (!mWithHeader) {
			return false;
		}

		if (mValueIndex >= mHeaders.size() || mHeaders[mValueIndex] != key)
		{
			// If next column doesn't match, try to find across all headers
			for (mValueIndex = 0; mValueIndex < mHeaders.size(); ++mValueIndex)
			{
				if (mHeaders[mValueIndex] == key)
				{
					break;
				}
			}
			if (mValueIndex == mHeaders.size())
			{
				out_value = {};
				return false;
			}
		}

		const auto& valueMeta = mRowValuesMeta[mValueIndex];
		out_value = std::string_view((valueMeta.InOriginalData ? mSourceString.data() : mTempValueBuffer.data()) + valueMeta.Offset, valueMeta.Size);
		++mValueIndex;
		return true;
	}

	void CCsvStringReader::ReadValue(std::string_view& out_value)
	{
		if (mValueIndex < mRowValuesMeta.size())
		{
			const auto& valueMeta = mRowValuesMeta[mValueIndex];
			out_value = std::string_view((valueMeta.InOriginalData ? mSourceString.data() : mTempValueBuffer.data()) + valueMeta.Offset, valueMeta.Size);
			++mValueIndex;
			return;
		}
		throw SerializationException(SerializationErrorCode::OutOfRange, "There are no more values in the row");
	}

	bool CCsvStringReader::ParseNextRow()
	{
		if (ParseNextLine())
		{
			if (mWithHeader)
			{
				if (mHeaders.size() != mRowValuesMeta.size())
				{
					throw ParsingException("Number of values are different than in header, line: "
						+ Convert::ToString(mLineNumber), mLineNumber);
				}
			}
			else if (mLineNumber >= 2 && mPrevValuesCount != mRowValuesMeta.size())
			{
				throw ParsingException("Number of values are different than in previous line, line: "
					+ Convert::ToString(mLineNumber), mLineNumber);
			}

			mValueIndex = 0;
			// Header is not counted as data row
			const bool firstDataRow = mLineNumber == (mWithHeader ? 2u : 1u);
			if (!firstDataRow)
			{
				++mRowIndex;
			}
			return true;
		}
		return false;
	}

	bool CCsvStringReader::ParseNextLine()
	{
		const auto totalSize = mSourceString.size();
		if (mCurrentPos >= totalSize)
		{
			return false;
		}

		++mLineNumber;
		mTempValueBuffer.resize(mLockedBufferSize);
		mPrevValuesCount = mRowValuesMeta.size();
		mRowValuesMeta.clear();

		for (auto isEndLine = false; !isEndLine;)
		{
			const size_t startValuePos = mCurrentPos;
			uint_fast32_t doubleQuotesCount = 0u;
			bool inDoubleQuotes = false;
			size_t endValuePos = totalSize;

			while (mCurrentPos < totalSize)
			{
				const char sym = mSourceString[mCurrentPos];
				if (sym == '"')
				{
					++doubleQuotesCount;
					inDoubleQuotes = doubleQuotesCount & 1u;
				}

				if (!inDoubleQuotes)
				{
					// Handle delimiter
					if (sym == mSeparator)
					{
						endValuePos = mCurrentPos;
						++mCurrentPos;
						break;
					}
					if (sym == '\r')
					{
						// Check for CRLF
						endValuePos = mCurrentPos;
						++mCurrentPos;
						if (mCurrentPos < totalSize && mSourceString[mCurrentPos] == '\n') {
							++mCurrentPos;
						}
						isEndLine = true;
						break;
					}
					if (sym == '\n')
					{
						endValuePos = mCurrentPos;
						++mCurrentPos;
						isEndLine = true;
						break;
					}
				}
				++mCurrentPos;
			}

			// Extract values even line is empty (CSV can consist only one column, some values can be empty)
			if (doubleQuotesCount == 0u)
			{
				mRowValuesMeta.emplace_back(startValuePos, endValuePos - startValuePos, true);
			}
			else
			{
				UnescapeValue(std::string_view(mSourceString.data() + startValuePos, endValuePos - startValuePos));
			}

			// Handle end of file (RFC: The last record in the file may or may not have an ending line break)
			if (mCurrentPos == totalSize)
			{
				break;
			}
		}

		return !mRowValuesMeta.empty();
	}

	void CCsvStringReader::UnescapeValue(std::string_view value)
	{
		// Validate first and end double quotes
		if (value.front() != '"')
		{
			throw ParsingException("Missing starting double quotes, line: " + Convert::ToString(mLineNumber), mLineNumber);
		}
		if (value.size() < 2 || value.back() != '"')
		{
			throw ParsingException("Missing trailing double quotes, line: " + Convert::ToString(mLineNumber), mLineNumber);
		}

		// Reserve output buffer
		const size_t startIndex = mTempValueBuffer.size();
		size_t outIndex = startIndex;
		mTempValueBuffer.resize(outIndex + value.size());

		// Copy with skip one of two double quotes
		size_t lastDoubleQuotes = 0;
		const size_t endValuePos = value.size() - 1;
		for (size_t i = 1; i < endValuePos; ++i)
		{
			const char sym = value[i];
			if (sym == '"')
			{
				if (lastDoubleQuotes == 0)
				{
					// Skip one of two double quotes
					lastDoubleQuotes = i;
					continue;
				}
				// Check for single (unescaped) double quotes
				if (lastDoubleQuotes + 1 != i)
				{
					break;
				}
				lastDoubleQuotes = 0;
			}
			mTempValueBuffer[outIndex] = sym;
			++outIndex;
		}

		if (lastDoubleQuotes)
		{
			throw ParsingException("Unescaped double quotes, line: " + Convert::ToString(mLineNumber), mLineNumber);
		}

		// Adjust buffer to actual value size
		mTempValueBuffer.resize(outIndex);

		mRowValuesMeta.emplace_back(startIndex, outIndex - startIndex, false);
	}

	//------------------------------------------------------------------------------

	CCsvStreamReader::CCsvStreamReader(std::istream& inputStream, bool withHeader, char separator)
		: mEncodedStreamReader(inputStream)
		, mWithHeader(withHeader)
		, mSeparator(separator)
	{
		if (withHeader)
		{
			if (ParseNextLine())
			{
				std::string_view val;
				mHeaders.resize(mRowValuesMeta.size());
				for (auto& header : mHeaders)
				{
					ReadValue(val);
					header = val;
				}
			}
			else
			{
				throw ParsingException("Input string is empty, expected at least a header line");
			}
		}
	}

	bool CCsvStreamReader::SeekToHeader(size_t headerIndex, std::string_view& out_header) noexcept
	{
		if (mWithHeader)
		{
			if (headerIndex < mHeaders.size())
			{
				out_header = mHeaders[headerIndex];
				mValueIndex = headerIndex;
				return true;
			}
		}
		return false;
	}

	bool CCsvStreamReader::ReadValue(std::string_view key, std::string_view& out_value) noexcept
	{
		if (!mWithHeader) {
			return false;
		}

		if (mValueIndex >= mHeaders.size() || mHeaders[mValueIndex] != key)
		{
			// If next column doesn't match, try to find across all headers
			for (mValueIndex = 0; mValueIndex < mHeaders.size(); ++mValueIndex)
			{
				if (mHeaders[mValueIndex] == key)
				{
					break;
				}
			}
			if (mValueIndex == mHeaders.size())
			{
				out_value = {};
				return false;
			}
		}

		const auto& valueMeta = mRowValuesMeta[mValueIndex];
		out_value = std::string_view(mDecodedBuffer.data() + valueMeta.Offset, valueMeta.Size);
		++mValueIndex;
		return true;
	}

	void CCsvStreamReader::ReadValue(std::string_view& out_value)
	{
		if (mValueIndex < mRowValuesMeta.size())
		{
			const auto& valueMeta = mRowValuesMeta[mValueIndex];
			out_value = std::string_view(mDecodedBuffer.data() + valueMeta.Offset, valueMeta.Size);

			++mValueIndex;
			return;
		}
		throw SerializationException(SerializationErrorCode::OutOfRange, "There are no more values in the row");
	}

	bool CCsvStreamReader::ParseNextRow()
	{
		if (ParseNextLine())
		{
			if (mWithHeader)
			{
				if (mHeaders.size() != mRowValuesMeta.size())
				{
					throw ParsingException("Number of values is different from that in the header, line: "
						+ Convert::ToString(mLineNumber), mLineNumber);
				}
			}
			else if (mLineNumber >= 2 && mPrevValuesCount != mRowValuesMeta.size())
			{
				throw ParsingException("Number of values are different than in previous line, line: "
					+ Convert::ToString(mLineNumber), mLineNumber);
			}

			mValueIndex = 0;
			// Header is not counted as data row
			const bool firstDataRow = mLineNumber == (mWithHeader ? 2u : 1u);
			if (!firstDataRow)
			{
				++mRowIndex;
			}
			return true;
		}
		return false;
	}

	bool CCsvStreamReader::ParseNextLine()
	{
		if (IsEnd())
		{
			return false;
		}

		++mLineNumber;
		mPrevValuesCount = mRowValuesMeta.size();
		mRowValuesMeta.clear();

		// Remove parsed part if processed more than half of the buffer
		constexpr size_t minSizeToSqueeze = Convert::Utf::CEncodedStreamReader<char>::chunk_size >> 1;
		if (mDecodedBuffer.size() >= Convert::Utf::CEncodedStreamReader<char>::chunk_size && mCurrentPos >= minSizeToSqueeze)
		{
			mDecodedBuffer.erase(0, mCurrentPos);
			mCurrentPos = 0;
		}

		bool pendingCR = false;
		for (auto isEndLine = false; !isEndLine;)
		{
			const size_t startValuePos = mCurrentPos;
			uint_fast32_t doubleQuotesCount = 0;
			bool inDoubleQuotes = false;
			size_t endValuePos;

			while (true)
			{
				if (mCurrentPos == mDecodedBuffer.size())
				{
					const auto result = mEncodedStreamReader.ReadChunk(mDecodedBuffer);
					// When reached end of file
					if (result == Convert::Utf::EncodedStreamReadResult::EndFile)
					{
						endValuePos = mDecodedBuffer.size();
						if (pendingCR && endValuePos > 0) {
							--endValuePos;
						}
						isEndLine = true;
						break;
					}
					if (result == Convert::Utf::EncodedStreamReadResult::DecodeError) {
						throw SerializationException(SerializationErrorCode::UtfEncodingError, "The input stream might be corrupted, unable to decode UTF");
					}

					// After reading new chunk, check if pending \r is followed by \n
					if (pendingCR)
					{
						endValuePos = mCurrentPos - 1;
						if (mCurrentPos < mDecodedBuffer.size() && mDecodedBuffer[mCurrentPos] == '\n')
						{
							++mCurrentPos;
						}
						isEndLine = true;
						break;
					}
				}

				const char sym = mDecodedBuffer[mCurrentPos];
				if (sym == '"')
				{
					++doubleQuotesCount;
					inDoubleQuotes = (doubleQuotesCount & 1u) == 1u;
				}

				if (!inDoubleQuotes)
				{
					// Handle delimiter
					if (sym == mSeparator)
					{
						endValuePos = mCurrentPos;
						++mCurrentPos;
						break;
					}
					if (sym == '\r')
					{
						if (mCurrentPos == mDecodedBuffer.size() - 1)
						{
							// At the end of chunk - defer
							pendingCR = true;
							++mCurrentPos;
							continue;
						}
						if (mDecodedBuffer[mCurrentPos + 1] == '\n')
						{
							endValuePos = mCurrentPos;
							mCurrentPos += 2;
							isEndLine = true;
							break;
						}
						endValuePos = mCurrentPos;
						++mCurrentPos;
						isEndLine = true;
						break;
					}
					if (sym == '\n')
					{
						endValuePos = mCurrentPos;
						++mCurrentPos;
						isEndLine = true;
						break;
					}
				}
				++mCurrentPos;
			}

			// Extract values even line is empty (CSV can consist only one column, some values can be empty)
			if (doubleQuotesCount == 0u)
			{
				mRowValuesMeta.emplace_back(startValuePos, endValuePos - startValuePos);
			}
			else
			{
				UnescapeValue(mDecodedBuffer.data() + startValuePos, mDecodedBuffer.data() + endValuePos);
			}
		}

		// When entire buffer has been parsed, need to read next chunk for detect end of file
		if (mCurrentPos == mDecodedBuffer.size())
		{
			mEncodedStreamReader.ReadChunk(mDecodedBuffer);
		}

		return !mRowValuesMeta.empty();
	}

	void CCsvStreamReader::UnescapeValue(char* beginIt, const char* endIt)
	{
		// Validate first and end double quotes
		if (*beginIt != '"')
		{
			throw ParsingException("Missing starting double quotes, line: " + Convert::ToString(mLineNumber), mLineNumber);
		}
		--endIt;
		if (endIt - beginIt < 1 || *endIt != '"')
		{
			throw ParsingException("Missing trailing double quotes, line: " + Convert::ToString(mLineNumber), mLineNumber);
		}

		// Decode to the same buffer
		char* decodedIt = beginIt;
		const char* lastDoubleQuotes = nullptr;
		for (char* currentPos = beginIt + 1; currentPos != endIt; ++currentPos)
		{
			const char sym = *currentPos;
			if (sym == '"')
			{
				if (!lastDoubleQuotes)
				{
					// Skip one of two double quotes
					lastDoubleQuotes = currentPos;
					continue;
				}
				// Check for single (unescaped) double quotes
				if (lastDoubleQuotes + 1 != currentPos)
				{
					break;
				}
				lastDoubleQuotes = nullptr;
			}
			*decodedIt = sym;
			++decodedIt;
		}

		if (lastDoubleQuotes)
		{
			throw ParsingException("Unescaped double quotes, line: " + Convert::ToString(mLineNumber), mLineNumber);
		}

		mRowValuesMeta.emplace_back(beginIt - mDecodedBuffer.data(), decodedIt - beginIt);
	}
}
