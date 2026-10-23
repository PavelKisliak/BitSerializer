/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "csv_readers.h"
#include "bitserializer/common/text.h"
#include "common/simd/scan.h"


namespace BitSerializer::Csv::Detail
{
	CsvStringReader::CsvStringReader(std::string_view inputString, bool withHeader, char separator)
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

	bool CsvStringReader::SeekToHeader(size_t headerIndex, std::string_view& out_header) noexcept
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

	bool CsvStringReader::ReadValue(std::string_view key, std::string_view& out_value) noexcept
	{
		if (!mWithHeader) {
			return false;
		}

		if (mValueIndex >= mHeaders.size() || !Text::Equal(mHeaders[mValueIndex], key))
		{
			// If next column doesn't match, try to find across all headers
			for (mValueIndex = 0; mValueIndex < mHeaders.size(); ++mValueIndex)
			{
				if (Text::Equal(mHeaders[mValueIndex], key))
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

	void CsvStringReader::ReadValue(std::string_view& out_value)
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

	bool CsvStringReader::ParseNextRow()
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

	bool CsvStringReader::ParseNextLine()
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
			bool hasDoubleQuotes = false;
			bool inDoubleQuotes = false;
			size_t endValuePos = totalSize;

			while (mCurrentPos < totalSize)
			{
				// Skip the run of ordinary bytes: inside quotes only '"' is special,
				// otherwise '"', the separator, CR and LF.
				const size_t foundPos = inDoubleQuotes
					? BitSerializer::Simd::FindFirstOf<'"'>(mSourceString, mCurrentPos)
					: BitSerializer::Simd::FindFirstOf<'"', '\r', '\n'>(mSourceString, mCurrentPos, mSeparator);
				if (foundPos == std::string_view::npos)
				{
					mCurrentPos = totalSize;
					break;
				}

				const char sym = mSourceString[foundPos];
				if (sym == '"')
				{
					hasDoubleQuotes = true;
					inDoubleQuotes = !inDoubleQuotes;
					mCurrentPos = foundPos + 1;
					continue;
				}

				// Outside quotes only a separator or a line break can reach this point.
				endValuePos = foundPos;
				mCurrentPos = foundPos + 1;
				if (sym == mSeparator)
				{
					break;
				}
				if (sym == '\r' && mCurrentPos < totalSize && mSourceString[mCurrentPos] == '\n') {
					++mCurrentPos;
				}
				isEndLine = true;
				break;
			}

			// Extract values even line is empty (CSV can consist only one column, some values can be empty)
			if (!hasDoubleQuotes)
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

	void CsvStringReader::UnescapeValue(std::string_view value)
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

	CsvStreamReader::CsvStreamReader(std::istream& inputStream, bool withHeader, char separator, Convert::Utf::UtfEncodingErrorPolicy encodingErrorPolicy)
		: mEncodedStreamReader(inputStream, encodingErrorPolicy)
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

	bool CsvStreamReader::SeekToHeader(size_t headerIndex, std::string_view& out_header) noexcept
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

	bool CsvStreamReader::ReadValue(std::string_view key, std::string_view& out_value) noexcept
	{
		if (!mWithHeader) {
			return false;
		}

		if (mValueIndex >= mHeaders.size() || !Text::Equal(mHeaders[mValueIndex], key))
		{
			// If next column doesn't match, try to find across all headers
			for (mValueIndex = 0; mValueIndex < mHeaders.size(); ++mValueIndex)
			{
				if (Text::Equal(mHeaders[mValueIndex], key))
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
		out_value = std::string_view((valueMeta.InOriginalData ? mRowDataBase : mTempValueBuffer.data()) + valueMeta.Offset, valueMeta.Size);
		++mValueIndex;
		return true;
	}

	void CsvStreamReader::ReadValue(std::string_view& out_value)
	{
		if (mValueIndex < mRowValuesMeta.size())
		{
			const auto& valueMeta = mRowValuesMeta[mValueIndex];
			out_value = std::string_view((valueMeta.InOriginalData ? mRowDataBase : mTempValueBuffer.data()) + valueMeta.Offset, valueMeta.Size);

			++mValueIndex;
			return;
		}
		throw SerializationException(SerializationErrorCode::OutOfRange, "There are no more values in the row");
	}

	bool CsvStreamReader::ParseNextRow()
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

	bool CsvStreamReader::ParseNextLine()
	{
		if (IsEnd())
		{
			return false;
		}

		++mLineNumber;
		mPrevValuesCount = mRowValuesMeta.size();
		mRowValuesMeta.clear();
		mTempValueBuffer.clear();

		// Trim consumed data in the internal stream reader (no-op when less than one chunk was consumed)
		mEncodedStreamReader.TrimConsumedData();

		auto readChunk = [this](size_t minChars) -> std::string_view
		{
			try
			{
				return mEncodedStreamReader.PeekChars(minChars);
			}
			catch (const std::invalid_argument&)
			{
				throw ParsingException("Invalid UTF sequence detected in the stream, line: "
					+ Convert::ToString(mLineNumber), mLineNumber);
			}
		};

		// Parse the line directly from the stream reader buffer, growing the peeked window when
		// the end of the available data is reached. Value offsets are relative to the buffer start,
		// which is stable until the next TrimConsumedData.
		std::string_view window = readChunk(1);
		size_t pos = 0;
		bool pendingCR = false;
		for (auto isEndLine = false; !isEndLine;)
		{
			const size_t startValuePos = pos;
			uint_fast32_t doubleQuotesCount = 0;
			bool inDoubleQuotes = false;
			size_t endValuePos;

			while (true)
			{
				if (pos == window.size())
				{
					// Grow the window to fetch the next chunk
					const auto prevSize = window.size();
					window = readChunk(prevSize + 1);
					if (window.size() == prevSize)
					{
						// End of stream
						endValuePos = pos;
						if (pendingCR && endValuePos > 0) {
							--endValuePos;
						}
						isEndLine = true;
						break;
					}

					// After reading a new chunk, check if pending \r is followed by \n
					if (pendingCR)
					{
						endValuePos = pos - 1;
						if (pos < window.size() && window[pos] == '\n')
						{
							++pos;
						}
						isEndLine = true;
						break;
					}
				}

				// Skip the run of ordinary bytes within the currently peeked window:
				// inside quotes only '"' is special, otherwise '"', the separator, CR and LF are.
				const size_t foundPos = inDoubleQuotes
					? BitSerializer::Simd::FindFirstOf<'"'>(window, pos)
					: BitSerializer::Simd::FindFirstOf<'"', '\r', '\n'>(window, pos, mSeparator);
				if (foundPos == std::string_view::npos)
				{
					// No special byte in the remaining window - grow it and rescan
					pos = window.size();
					continue;
				}

				const char sym = window[foundPos];
				if (sym == '"')
				{
					++doubleQuotesCount;
					inDoubleQuotes = (doubleQuotesCount & 1u) == 1u;
					pos = foundPos + 1;
					continue;
				}

				// Outside quotes only a separator or a line break can reach this point.
				if (sym == mSeparator)
				{
					endValuePos = foundPos;
					pos = foundPos + 1;
					break;
				}
				if (sym == '\r')
				{
					if (foundPos == window.size() - 1)
					{
						// At the end of the window - defer to know whether the next byte is '\n'
						pendingCR = true;
						pos = foundPos + 1;
						continue;
					}
					if (window[foundPos + 1] == '\n')
					{
						endValuePos = foundPos;
						pos = foundPos + 2;
						isEndLine = true;
						break;
					}
					endValuePos = foundPos;
					pos = foundPos + 1;
					isEndLine = true;
					break;
				}
				// sym == '\n'
				endValuePos = foundPos;
				pos = foundPos + 1;
				isEndLine = true;
				break;
			}

			// Extract values even when the line is empty (CSV can consist of one column with some empty values)
			if (doubleQuotesCount == 0u)
			{
				mRowValuesMeta.emplace_back(startValuePos, endValuePos - startValuePos, true);
			}
			else
			{
				UnescapeValue(window.data() + startValuePos, window.data() + endValuePos);
			}
		}

		// Advance the stream reader position to the end of the parsed line
		mEncodedStreamReader.SkipChars(pos);

		if (!mRowValuesMeta.empty())
		{
			// Read ahead to detect end of stream and pin the row data base pointer,
			// which is stable until the next ParseNextLine call
			mRowDataBase = readChunk(1).data() - pos;
		}

		return !mRowValuesMeta.empty();
	}

	void CsvStreamReader::UnescapeValue(const char* beginIt, const char* endIt)
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

		// Copy to the temp buffer with skipping one of two double quotes
		const size_t startIndex = mTempValueBuffer.size();
		size_t outIndex = startIndex;
		mTempValueBuffer.resize(outIndex + static_cast<size_t>(endIt - beginIt));

		const char* lastDoubleQuotes = nullptr;
		for (const char* currentPos = beginIt + 1; currentPos != endIt; ++currentPos)
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
}
