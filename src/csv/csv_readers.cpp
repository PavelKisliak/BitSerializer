/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "csv_readers.h"
#include <algorithm>


namespace BitSerializer::Csv::Detail
{
	CCsvStringReader::CCsvStringReader(std::string_view inputString, bool withHeader, char separator)
		: mSourceString(inputString)
		, mWithHeader(withHeader)
		, mSeparator(separator)
	{
		if (withHeader)
		{
			if (ParseNextLine(mRowValuesMeta))
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

	bool CCsvStringReader::ReadValue(std::string_view key, std::string_view& out_value)
	{
		if (!mWithHeader) {
			return false;
		}

		if (++mValueIndex >= mHeaders.size() || mHeaders[mValueIndex] != key)
		{
			// If next column doesn't match, try to find across all headers
			const auto it = std::find(mHeaders.cbegin(), mHeaders.cend(), key);
			if (it == std::cend(mHeaders))
			{
				out_value = {};
				return false;
			}
			mValueIndex = it - mHeaders.cbegin();
		}

		const auto& valueMeta = mRowValuesMeta.at(mValueIndex);
		if (valueMeta.HasEscapedChars)
		{
			out_value = UnescapeValue(std::string_view(mSourceString.data() + valueMeta.Offset, valueMeta.Size));
		}
		else
		{
			out_value = std::string_view(mSourceString.data() + valueMeta.Offset, valueMeta.Size);
		}
		return true;
	}

	void CCsvStringReader::ReadValue(std::string_view& out_value)
	{
		if (mValueIndex < mRowValuesMeta.size())
		{
			const auto& valueMeta = mRowValuesMeta.at(mValueIndex);
			if (valueMeta.HasEscapedChars)
			{
				out_value = UnescapeValue(std::string_view(mSourceString.data() + valueMeta.Offset, valueMeta.Size));
			}
			else
			{
				out_value = std::string_view(mSourceString.data() + valueMeta.Offset, valueMeta.Size);
			}

			++mValueIndex;
			return;
		}
		throw SerializationException(SerializationErrorCode::OutOfRange, "There are no more values in the row");
	}

	bool CCsvStringReader::ParseNextRow()
	{
		if (ParseNextLine(mRowValuesMeta))
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
			const bool firstDataRow = mLineNumber == (mWithHeader ? 2 : 1);
			if (!firstDataRow)
			{
				++mRowIndex;
			}
			return true;
		}
		return false;
	}

	bool CCsvStringReader::ParseNextLine(std::vector<CValueMeta>& out_values)
	{
		const auto totalSize = mSourceString.size();
		if (mCurrentPos >= totalSize)
		{
			return false;
		}

		++mLineNumber;
		mPrevValuesCount = out_values.size();
		out_values.clear();
		mRowValuesMeta.clear();

		for (auto isEndLine = false; !isEndLine;)
		{
			const size_t startValuePos = mCurrentPos;
			size_t doubleQuotesCount = 0;
			size_t endValuePos = totalSize;
			size_t precedingCrPos = std::string::npos;

			while (mCurrentPos < totalSize)
			{
				const char sym = mSourceString[mCurrentPos];
				if (sym == '"')
				{
					++doubleQuotesCount;
				}
				// Handle delimiter
				else if (sym == mSeparator && doubleQuotesCount % 2 == 0)
				{
					endValuePos = mCurrentPos;
					++mCurrentPos;
					break;
				}
				// End of line (can be CRLF or just LF)
				else if (sym == '\r')
				{
					precedingCrPos = mCurrentPos;
				}
				else if (sym == '\n' && doubleQuotesCount % 2 == 0)
				{
					if (precedingCrPos == std::string::npos) {
						precedingCrPos = mCurrentPos;
					}
					endValuePos = (precedingCrPos == mCurrentPos - 1) ? precedingCrPos : mCurrentPos;
					++mCurrentPos;
					isEndLine = true;
					break;
				}
				++mCurrentPos;
			}

			// Extract values even line is empty (CSV can consist only one column, some values can be empty)
			out_values.emplace_back(startValuePos, endValuePos - startValuePos, doubleQuotesCount ? true : false);

			// Handle end of file (RFC: The last record in the file may or may not have an ending line break)
			if (mCurrentPos == mSourceString.size())
			{
				break;
			}
		}

		return !out_values.empty();
	}

	std::string_view CCsvStringReader::UnescapeValue(std::string_view value)
	{
		// Validate first and end double quotes
		if (value.empty() || value.front() != '"')
		{
			throw ParsingException("Missing starting double-quotes, line: " + Convert::ToString(mLineNumber), mLineNumber);
		}
		if (value.size() < 2 || value.back() != '"')
		{
			throw ParsingException("Missing trailing double-quotes, line: " + Convert::ToString(mLineNumber), mLineNumber);
		}

		// Reserve output buffer
		mTempValueBuffer.resize(value.size());

		// Copy with skip one of two double-quotes
		size_t outIndex = 0;
		size_t doubleQuotesCount = 0;
		const size_t endValuePos = value.size() - 1;
		for (size_t i = 1; i < endValuePos; ++i)
		{
			const char sym = value[i];
			if (sym == '"')
			{
				++doubleQuotesCount;
				if (doubleQuotesCount % 2 == 0)
				{
					continue;
				}
			}
			mTempValueBuffer[outIndex] = sym;
			++outIndex;
		}
		// Adjust buffer to actual value size
		mTempValueBuffer.resize(outIndex);

		return { mTempValueBuffer.data(), mTempValueBuffer.size() };
	}

	//------------------------------------------------------------------------------

	CCsvStreamReader::CCsvStreamReader(std::istream& inputStream, bool withHeader, char separator)
		: mEncodedStreamReader(inputStream)
		, mWithHeader(withHeader)
		, mSeparator(separator)
	{
		if (withHeader)
		{
			if (ParseNextLine(mRowValuesMeta))
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

	bool CCsvStreamReader::ReadValue(std::string_view key, std::string_view& out_value)
	{
		if (!mWithHeader) {
			return false;
		}

		if (++mValueIndex >= mHeaders.size() || mHeaders[mValueIndex] != key)
		{
			// If next column doesn't match, try to find across all headers
			const auto it = std::find(mHeaders.cbegin(), mHeaders.cend(), key);
			if (it == std::cend(mHeaders))
			{
				out_value = {};
				return false;
			}
			mValueIndex = it - mHeaders.cbegin();
		}

		const auto& valueMeta = mRowValuesMeta.at(mValueIndex);
		if (valueMeta.HasEscapedChars)
		{
			out_value = UnescapeValue(mDecodedBuffer.data() + valueMeta.Offset, mDecodedBuffer.data() + valueMeta.Size);
		}
		else
		{
			out_value = std::string_view(mDecodedBuffer.data() + valueMeta.Offset, valueMeta.Size);
		}
		return true;
	}

	void CCsvStreamReader::ReadValue(std::string_view& out_value)
	{
		if (mValueIndex < mRowValuesMeta.size())
		{
			const auto& valueMeta = mRowValuesMeta.at(mValueIndex);
			if (valueMeta.HasEscapedChars)
			{
				out_value = UnescapeValue(mDecodedBuffer.data() + valueMeta.Offset, mDecodedBuffer.data() + valueMeta.Offset + valueMeta.Size);
			}
			else
			{
				out_value = std::string_view(mDecodedBuffer.data() + valueMeta.Offset, valueMeta.Size);
			}

			++mValueIndex;
			return;
		}
		throw SerializationException(SerializationErrorCode::OutOfRange, "There are no more values in the row");
	}

	bool CCsvStreamReader::ParseNextRow()
	{
		if (ParseNextLine(mRowValuesMeta))
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
			const bool firstDataRow = mLineNumber == (mWithHeader ? 2 : 1);
			if (!firstDataRow)
			{
				++mRowIndex;
			}
			return true;
		}
		return false;
	}

	bool CCsvStreamReader::ParseNextLine(std::vector<CValueMeta>& out_values)
	{
		if (IsEnd())
		{
			return false;
		}

		++mLineNumber;
		mPrevValuesCount = out_values.size();
		out_values.clear();
		// Remove parsed string part
		if (mCurrentPos)
		{
			mDecodedBuffer.erase(0, mCurrentPos);
			mCurrentPos = 0;
		}

		for (auto isEndLine = false; !isEndLine;)
		{
			const size_t startValuePos = mCurrentPos;
			size_t doubleQuotesCount = 0;
			size_t endValuePos;
			size_t precedingCrPos = std::string::npos;

			while (true)
			{
				if (mCurrentPos == mDecodedBuffer.size())
				{
					if (!mEncodedStreamReader.ReadChunk(mDecodedBuffer))
					{
						// When reached end of file
						endValuePos = mDecodedBuffer.size();
						isEndLine = true;
						break;
					}
				}

				const char sym = mDecodedBuffer[mCurrentPos];
				if (sym == '"')
				{
					++doubleQuotesCount;
				}
				// Handle delimiter
				else if (sym == mSeparator && doubleQuotesCount % 2 == 0)
				{
					endValuePos = mCurrentPos;
					++mCurrentPos;
					break;
				}
				// End of line (can be CRLF or just LF)
				else if (sym == '\r')
				{
					precedingCrPos = mCurrentPos;
				}
				else if (sym == '\n' && doubleQuotesCount % 2 == 0)
				{
					if (precedingCrPos == std::string::npos) {
						precedingCrPos = mCurrentPos;
					}
					endValuePos = (precedingCrPos == mCurrentPos - 1) ? precedingCrPos : mCurrentPos;
					++mCurrentPos;
					isEndLine = true;
					break;
				}
				// End of file (RFC: The last record in the file may or may not have an ending line break)
				else if (mCurrentPos + 1 == mDecodedBuffer.size() && mEncodedStreamReader.IsEnd())
				{
					endValuePos = mCurrentPos = mDecodedBuffer.size();
					isEndLine = true;
					break;
				}
				++mCurrentPos;
			}

			// Extract values even line is empty (CSV can consist only one column, some values can be empty)
			out_values.emplace_back(startValuePos, endValuePos - startValuePos, doubleQuotesCount ? true : false);
		}

		// When entire buffer has been parsed, need to read next chunk for detect end of file
		if (mCurrentPos == mDecodedBuffer.size())
		{
			mEncodedStreamReader.ReadChunk(mDecodedBuffer);
		}

		return !out_values.empty();
	}

	std::string_view CCsvStreamReader::UnescapeValue(char* beginIt, char* endIt)
	{
		// Validate first and end double quotes
		if (*beginIt != '"')
		{
			throw ParsingException("Missing starting double-quotes, line: " + Convert::ToString(mLineNumber), mLineNumber);
		}
		--endIt;
		if (endIt - beginIt < 1 || *endIt != '"')
		{
			throw ParsingException("Missing trailing double-quotes, line: " + Convert::ToString(mLineNumber), mLineNumber);
		}

		// Decode to the same buffer
		char* decodedIt = beginIt;
		size_t doubleQuotesCount = 0;
		for (char* currentPos = beginIt + 1; currentPos != endIt; ++currentPos)
		{
			const char sym = *currentPos;
			if (sym == '"')
			{
				++doubleQuotesCount;
				// Skip one of two double quotes
				if (doubleQuotesCount % 2 == 0)
				{
					continue;
				}
			}
			*decodedIt = sym;
			++decodedIt;
		}

		return { beginIt, static_cast<std::string_view::size_type>(decodedIt - beginIt) };
	}
}
