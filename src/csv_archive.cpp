#include "bitserializer/csv_archive.h"

using namespace BitSerializer;

namespace
{
	void WriteEscapedValue(const std::string_view& value, std::string& outputString, const char separator)
	{
		if (value.cend() == std::find_if(value.cbegin(), value.cend(),
			[separator](const char sym) { return sym == '"' || sym == separator || sym == '\n'; }))
		{
			outputString.append(value);
			return;
		}

		// RFC: Fields containing line breaks (CRLF), double quotes, and commas should be enclosed in double-quotes
		outputString.push_back('"');
		for (const char sym : value)
		{
			if (sym == '"')
			{
				// RFC: Double-quote appearing inside a field must be escaped by preceding it with another double quote
				outputString.push_back(sym);
			}
			outputString.push_back(sym);
		}
		outputString.push_back('"');
	}

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

		const size_t startLinePos = mCurrentPos;
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
					endValuePos = (precedingCrPos == mCurrentPos - 1) ? precedingCrPos : mCurrentPos;
					++mCurrentPos;
					isEndLine = true;
					break;
				}
				// End of file (RFC: The last record in the file may or may not have an ending line break)
				else if (mCurrentPos + 1 == mSourceString.size())
				{
					endValuePos = mCurrentPos = mSourceString.size();
					isEndLine = true;
					break;
				}
				++mCurrentPos;
			}

			// Don't try to extract value when line is empty (e.g. last line)
			if (startLinePos != endValuePos)
			{
				if (valueQuotesCount)
				{
					out_values.emplace_back(
						UnescapeValue(std::string_view(mSourceString.data() + startValuePos, endValuePos - startValuePos), mLineNumber + 1));
				}
				else
				{
					out_values.emplace_back(mSourceString.data() + startValuePos, endValuePos - startValuePos);
				}
			}
		}

		return !out_values.empty();
	}

	//------------------------------------------------------------------------------

	CCsvStreamReader::CCsvStreamReader(std::istream& inputStream, bool withHeader, char separator, size_t chunkSize)
		: mInput(inputStream)
		, mWithHeader(withHeader)
		, mSeparator(separator)
		, mChunkSize(chunkSize)
	{
		const auto utfType = Convert::DetectEncoding(inputStream);
		if (utfType != Convert::UtfType::Utf8) {
			throw SerializationException(SerializationErrorCode::UnsupportedEncoding, "The archive does not support encoding: " + Convert::ToString(utfType));
		}

		ReadNextChunk();

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
		return mCurrentPos >= mBuffer.size() && mInput.eof();
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
		const size_t startLinePos = mCurrentPos;

		if (mInput.eof() && mCurrentPos >= mBuffer.size())
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
				if (mCurrentPos == mBuffer.size())
				{
					if (!ReadNextChunk())
					{
						endValuePos = mBuffer.size();
						isEndLine = true;
						break;
					}
				}

				const char sym = mBuffer[mCurrentPos];
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
					endValuePos = (precedingCrPos == mCurrentPos - 1) ? precedingCrPos : mCurrentPos;
					++mCurrentPos;
					isEndLine = true;
					break;
				}
				// End of file (RFC: The last record in the file may or may not have an ending line break)
				else if (mCurrentPos + 1 == mBuffer.size() && mInput.eof())
				{
					endValuePos = mCurrentPos = mBuffer.size();
					isEndLine = true;
					break;
				}
				++mCurrentPos;
			}

			// Don't try to extract value when line is empty (e.g. last line)
			if (startLinePos != endValuePos)
			{
				if (valueQuotesCount)
				{
					out_values.emplace_back(
						UnescapeValue(std::string_view(mBuffer.data() + startValuePos, endValuePos - startValuePos), mLineNumber + 1));
				}
				else
				{
					out_values.emplace_back(mBuffer.data() + startValuePos, endValuePos - startValuePos);
				}
			}
		}

		// Remove parsed part of string (should be called after parse whole line)
		RemoveParsedStringPart();

		// When entire buffer has been parsed, need to read next chunk for detect end of file
		if (mCurrentPos == mBuffer.size())
		{
			ReadNextChunk();
		}

		return !out_values.empty();
	}

	bool CCsvStreamReader::ReadNextChunk()
	{
		if (mInput.eof())
		{
			return false;
		}

		const size_t prevSize = mBuffer.size();
		mBuffer.resize(prevSize + mChunkSize);
		mInput.read(&mBuffer[prevSize], static_cast<std::streamsize>(mChunkSize));
		mBuffer.resize(prevSize + mInput.gcount());
		return prevSize != mBuffer.size();
	}

	void CCsvStreamReader::RemoveParsedStringPart()
	{
		if (mCurrentPos)
		{
			mBuffer.erase(0, mCurrentPos);
			mCurrentPos = 0;
		}
	}

	//------------------------------------------------------------------------------

	CCsvStringWriter::CCsvStringWriter(std::string& outputString, bool withHeader, char separator)
		: mOutputString(outputString)
		, mWithHeader(withHeader)
		, mSeparator(separator)
	{ }

	void CCsvStringWriter::SetEstimatedSize(size_t size)
	{
		mEstimatedSize = size;
	}

	void CCsvStringWriter::WriteValue(std::string_view key, const std::string& value)
	{
		// Write keys only when it's first row
		if (mWithHeader && mRowIndex == 0)
		{
			if (!mCsvHeader.empty())
			{
				mCsvHeader.push_back(mSeparator);
			}
			WriteEscapedValue(key, mCsvHeader, mSeparator);
		}

		if (!mCurrentRow.empty())
		{
			mCurrentRow.push_back(mSeparator);
		}
		WriteEscapedValue(value, mCurrentRow, mSeparator);
		++mValueIndex;
	}

	void CCsvStringWriter::NextLine()
	{
		// Compare number of values with previous row
		if (mRowIndex >= 1 && mValueIndex != mPrevValuesCount)
		{
			throw SerializationException(SerializationErrorCode::OutOfRange,
				"Number of values are different than in previous line");
		}

		if (mRowIndex == 0)
		{
			// Reserve output buffer
			if (mEstimatedSize != 0)
			{
				constexpr size_t CrLfSize = 2;
				const auto estimatedByteSize = mCsvHeader.size() + CrLfSize
					+ static_cast<size_t>(static_cast<double>(mCurrentRow.size() + CrLfSize) * static_cast<double>(mEstimatedSize) * 1.1);
				mOutputString.reserve(estimatedByteSize);
			}
			if (mWithHeader)
			{
				mOutputString.append(mCsvHeader);
				mOutputString.append({ '\r', '\n' });
			}
		}

		mOutputString.append(mCurrentRow);
		mOutputString.append({ '\r', '\n' });

		++mRowIndex;
		mPrevValuesCount = mValueIndex;
		mValueIndex = 0;
		mCurrentRow.clear();
	}

	//------------------------------------------------------------------------------

	CCsvStreamWriter::CCsvStreamWriter(std::ostream& outputStream, bool withHeader, char separator, const StreamOptions& streamOptions)
		: mOutputStream(outputStream)
		, mWithHeader(withHeader)
		, mSeparator(separator)
		, mStreamOptions(streamOptions)
	{
		if (mStreamOptions.encoding != Convert::UtfType::Utf8)
		{
			const auto strEncodingType = Convert::TryTo<std::string>(streamOptions.encoding);
			throw SerializationException(SerializationErrorCode::UnsupportedEncoding,
				"The archive does not support encoding: " +
				(strEncodingType.has_value() ? strEncodingType.value() : std::to_string(static_cast<int>(streamOptions.encoding))));
		}

		if (mStreamOptions.writeBom)
		{
			mOutputStream.write(Convert::Utf8::bom, sizeof Convert::Utf8::bom);
		}
	}

	void CCsvStreamWriter::WriteValue(std::string_view key, const std::string& value)
	{
		// Write keys only when it's first row
		if (mWithHeader && mRowIndex == 0)
		{
			if (!mCsvHeader.empty())
			{
				mCsvHeader.push_back(mSeparator);
			}
			WriteEscapedValue(key, mCsvHeader, mSeparator);
		}

		if (!mCurrentRow.empty())
		{
			mCurrentRow.push_back(mSeparator);
		}
		WriteEscapedValue(value, mCurrentRow, mSeparator);
		++mValueIndex;
	}

	void CCsvStreamWriter::NextLine()
	{
		// Compare number of values with previous row
		if (mRowIndex >= 1 && mValueIndex != mPrevValuesCount)
		{
			throw SerializationException(SerializationErrorCode::OutOfRange,
				"Number of values are different than in previous line");
		}

		if (mRowIndex == 0)
		{
			if (mWithHeader)
			{
				mCsvHeader.append({ '\r', '\n' });
				mOutputStream.write(mCsvHeader.data(), static_cast<std::streamsize>(mCsvHeader.size()));
			}
		}

		mCurrentRow.append({ '\r', '\n' });
		mOutputStream.write(mCurrentRow.data(), static_cast<std::streamsize>(mCurrentRow.size()));

		++mRowIndex;
		mPrevValuesCount = mValueIndex;
		mValueIndex = 0;
		mCurrentRow.clear();
	}
}
