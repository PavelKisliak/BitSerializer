/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "csv_readers.h"
#include <algorithm>


namespace
{
	using namespace BitSerializer;

	std::string UnescapeValue(std::string_view value, size_t lineNumber)
	{
		size_t startValuePos = 0, endValuePos = value.size();

		// Skip first double-quotes
		if (value[startValuePos] == '"')
		{
			++startValuePos;
		}
		else
		{
			throw SerializationException(SerializationErrorCode::ParsingError,
				"Missing starting double-quotes, line: " + Convert::ToString(lineNumber));
		}
		if (value[endValuePos - 1] == '"')
		{
			--endValuePos;
		}
		else
		{
			throw SerializationException(SerializationErrorCode::ParsingError,
				"Missing trailing double-quotes, line: " + Convert::ToString(lineNumber));
		}

		// Copy with skip one of two double-quotes
		std::string result;
		result.reserve(value.size());
		size_t valueQuotesCount = 0;
		for (auto i = startValuePos; i < endValuePos; ++i)
		{
			const char sym = value[i];
			if (sym == '"')
			{
				++valueQuotesCount;
				if (valueQuotesCount % 2 == 0)
				{
					continue;
				}
			}
			result.push_back(sym);
		}

		return result;
	}
}

namespace BitSerializer::Csv::Detail
{
	CCsvStringReader::CCsvStringReader(std::string_view inputString, bool withHeader, char separator)
		: mSourceString(inputString)
		, mWithHeader(withHeader)
		, mSeparator(separator)
	{
		if (withHeader)
		{
			if (ParseLine(mHeader))
			{
				++mLineNumber;
			}
			else
			{
				throw SerializationException(SerializationErrorCode::ParsingError, "Input string is empty, expected at least a header line");
			}
		}
	}

	bool CCsvStringReader::IsEnd() const
	{
		return mCurrentPos >= mSourceString.size();
	}

	bool CCsvStringReader::ReadValue(std::string_view key, std::string& value)
	{
		if (mWithHeader)
		{
			const auto it = std::find(mHeader.cbegin(), mHeader.cend(), key);
			if (it != std::cend(mHeader))
			{
				mValueIndex = it - mHeader.cbegin();
				value = std::move(mRowValues.at(mValueIndex));
				return true;
			}
		}

		value.clear();
		return false;
	}

	void CCsvStringReader::ReadValue(std::string& value)
	{
		if (mValueIndex < mRowValues.size())
		{
			value = std::move(mRowValues[mValueIndex]);
			++mValueIndex;
			return;
		}
		throw SerializationException(SerializationErrorCode::OutOfRange, "There are no more values in the row");
	}

	bool CCsvStringReader::ParseNextRow()
	{
		if (IsEnd())
		{
			return false;
		}

		const bool firstDataRow = mLineNumber == (mWithHeader ? 1 : 0);
		if (ParseLine(mRowValues))
		{
			if (mWithHeader)
			{
				if (mHeader.size() != mRowValues.size())
				{
					throw SerializationException(SerializationErrorCode::ParsingError,
						"Number of values are different than in header, line: " + Convert::ToString(mLineNumber));
				}
			}
			else if (mLineNumber >= 1 && mPrevValuesCount != mRowValues.size())
			{
				throw SerializationException(SerializationErrorCode::ParsingError,
					"Number of values are different than in previous line, line: " + Convert::ToString(mLineNumber));
			}

			++mLineNumber;
			mValueIndex = 0;
			if (!firstDataRow)
			{
				++mRowIndex;
			}
			return true;
		}
		return false;
	}

	bool CCsvStringReader::ParseLine(std::vector<std::string>& out_values)
	{
		mPrevValuesCount = out_values.size();
		out_values.clear();

		const auto totalSize = mSourceString.size();
		if (mCurrentPos >= totalSize)
		{
			return false;
		}

		for (auto isEndLine = false; !isEndLine;)
		{
			const size_t startValuePos = mCurrentPos;
			size_t valueQuotesCount = 0;
			size_t endValuePos = totalSize;
			size_t precedingCrPos = std::string::npos;

			while (mCurrentPos < totalSize)
			{
				const char sym = mSourceString[mCurrentPos];
				if (sym == '"')
				{
					++valueQuotesCount;
				}
				// Handle delimiter
				else if (sym == mSeparator && valueQuotesCount % 2 == 0)
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
				else if (sym == '\n' && valueQuotesCount % 2 == 0)
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
			if (valueQuotesCount)
			{
				out_values.emplace_back(
					UnescapeValue(std::string_view(mSourceString.data() + startValuePos, endValuePos - startValuePos), mLineNumber + 1));
			}
			else
			{
				out_values.emplace_back(mSourceString.data() + startValuePos, endValuePos - startValuePos);
			}

			// Handle end of file (RFC: The last record in the file may or may not have an ending line break)
			if (mCurrentPos == mSourceString.size())
			{
				break;
			}
		}

		return !out_values.empty();
	}

	//------------------------------------------------------------------------------

	CCsvStreamReader::CCsvStreamReader(std::istream& inputStream, bool withHeader, char separator)
		: mEncodedStreamReader(inputStream)
		, mWithHeader(withHeader)
		, mSeparator(separator)
	{
		if (withHeader)
		{
			if (ParseLine(mHeader))
			{
				++mLineNumber;
			}
			else
			{
				throw SerializationException(SerializationErrorCode::ParsingError, "Input stream is empty, expected at least a header line");
			}
		}
	}

	bool CCsvStreamReader::IsEnd() const
	{
		return mCurrentPos >= mDecodedBuffer.size() && mEncodedStreamReader.IsEnd();
	}

	bool CCsvStreamReader::ReadValue(std::string_view key, std::string& value)
	{
		if (mWithHeader)
		{
			const auto it = std::find(mHeader.cbegin(), mHeader.cend(), key);
			if (it != std::cend(mHeader))
			{
				mValueIndex = it - mHeader.cbegin();
				value = std::move(mRowValues.at(mValueIndex));
				return true;
			}
		}

		value.clear();
		return false;
	}

	void CCsvStreamReader::ReadValue(std::string& value)
	{
		if (mValueIndex < mRowValues.size())
		{
			value = std::move(mRowValues[mValueIndex]);
			++mValueIndex;
			return;
		}
		throw SerializationException(SerializationErrorCode::OutOfRange, "There are no more values in the row");
	}

	bool CCsvStreamReader::ParseNextRow()
	{
		if (IsEnd())
		{
			return false;
		}

		const bool firstDataRow = mLineNumber == (mWithHeader ? 1 : 0);
		if (ParseLine(mRowValues))
		{
			if (mWithHeader)
			{
				if (mHeader.size() != mRowValues.size())
				{
					throw SerializationException(SerializationErrorCode::ParsingError,
						"Number of values are different than in header, line: " + Convert::ToString(mLineNumber));
				}
			}
			else if (mLineNumber >= 1 && mPrevValuesCount != mRowValues.size())
			{
				throw SerializationException(SerializationErrorCode::ParsingError,
					"Number of values are different than in previous line, line: " + Convert::ToString(mLineNumber));
			}

			++mLineNumber;
			mValueIndex = 0;
			if (!firstDataRow)
			{
				++mRowIndex;
			}
			return true;
		}
		return false;
	}

	bool CCsvStreamReader::ParseLine(std::vector<std::string>& out_values)
	{
		mPrevValuesCount = out_values.size();
		out_values.clear();

		if (IsEnd())
		{
			return false;
		}

		for (auto isEndLine = false; !isEndLine;)
		{
			const size_t startValuePos = mCurrentPos;
			size_t valueQuotesCount = 0;
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
					++valueQuotesCount;
				}
				// Handle delimiter
				else if (sym == mSeparator && valueQuotesCount % 2 == 0)
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
				else if (sym == '\n' && valueQuotesCount % 2 == 0)
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
			if (valueQuotesCount)
			{
				out_values.emplace_back(
					UnescapeValue(std::string_view(mDecodedBuffer.data() + startValuePos, endValuePos - startValuePos), mLineNumber + 1));
			}
			else
			{
				out_values.emplace_back(mDecodedBuffer.data() + startValuePos, endValuePos - startValuePos);
			}
		}

		// Remove parsed part of string (should be called after parse whole line)
		RemoveParsedStringPart();

		// When entire buffer has been parsed, need to read next chunk for detect end of file
		if (mCurrentPos == mDecodedBuffer.size())
		{
			mCurrentPos = 0;
			mDecodedBuffer.clear();
			mEncodedStreamReader.ReadChunk(mDecodedBuffer);
		}

		return !out_values.empty();
	}

	void CCsvStreamReader::RemoveParsedStringPart()
	{
		if (mCurrentPos)
		{
			mDecodedBuffer.erase(0, mCurrentPos);
			mCurrentPos = 0;
		}
	}
}
