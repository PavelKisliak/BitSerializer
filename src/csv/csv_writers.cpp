/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "csv_writers.h"
#include "common/simd/scan.h"


namespace
{
	using namespace BitSerializer;

	void WriteEscapedValue(const std::string_view& value, std::string& outputString, const char separator)
	{
		// Fast path: the value has no character that requires escaping (the common case).
		// The scan is SIMD-accelerated and the whole value is appended in a single call.
		const size_t firstEscape = BitSerializer::Simd::FindFirstOf<'"', '\n', '\r'>(value, 0, separator);
		if (firstEscape == std::string_view::npos)
		{
			outputString.append(value);
			return;
		}

		// RFC: Fields containing line breaks (CRLF), double quotes, and commas should be enclosed in double-quotes
		outputString.push_back('"');
		size_t pos = 0;
		size_t escapePos = firstEscape;
		do
		{
			outputString.append(value.data() + pos, escapePos - pos);
			const char sym = value[escapePos];
			if (sym == '"')
			{
				// RFC: Double-quote appearing inside a field must be escaped by preceding it with another double quote
				outputString.push_back('"');
			}
			outputString.push_back(sym);
			pos = escapePos + 1;
			// Inside the quoted field only a double quote still needs escaping.
			escapePos = BitSerializer::Simd::FindFirstOf<'"'>(value, pos);
		}
		while (escapePos != std::string_view::npos);

		if (pos < value.size())
		{
			outputString.append(value.data() + pos, value.size() - pos);
		}
		outputString.push_back('"');
	}
}


namespace BitSerializer::Csv::Detail
{
	CsvStringWriter::CsvStringWriter(std::string& outputString, bool withHeader, char separator)
		: mOutputString(outputString)
		, mWithHeader(withHeader)
		, mSeparator(separator)
	{
		mCurrentRow.reserve(256);
		mOutputString.reserve(256);
	}

	void CsvStringWriter::WriteValue(std::string_view key, std::string_view value)
	{
		BeginValue(key);
		WriteEscapedValue(value, mCurrentRow, mSeparator);
		EndValue();
	}

	void CsvStringWriter::WriteValue(std::string_view key, std::nullptr_t)
	{
		BeginValue(key);
		EndValue();
	}

	void CsvStringWriter::BeginValue(std::string_view key)
	{
		// Write keys only when it's first row
		if (mRowIndex == 0 && mWithHeader)
		{
			if (mValueIndex)
			{
				mOutputString.push_back(mSeparator);
			}
			WriteEscapedValue(key, mOutputString, mSeparator);
		}

		if (mValueIndex)
		{
			mCurrentRow.push_back(mSeparator);
		}
	}

	void CsvStringWriter::NextLine()
	{
		if (mRowIndex == 0)
		{
			if (mWithHeader)
			{
				mOutputString.push_back('\r');
				mOutputString.push_back('\n');
			}

			// Reserve output buffer
			if (mEstimatedSize != 0)
			{
				constexpr size_t CrLfSize = 2;
				const auto estimatedByteSize = mOutputString.size()
					+ static_cast<size_t>(static_cast<double>(mCurrentRow.size() + CrLfSize) * static_cast<double>(mEstimatedSize) * 1.2);
				mOutputString.reserve(estimatedByteSize);
			}
			mPrevValuesCount = mValueIndex;
		}
		else
		{
			// Compare number of values with previous row
			if (mValueIndex != mPrevValuesCount)
			{
				throw SerializationException(SerializationErrorCode::OutOfRange,
					"Number of values are different than in previous line");
			}
		}

		mOutputString.append(mCurrentRow);
		mOutputString.push_back('\r');
		mOutputString.push_back('\n');

		++mRowIndex;
		mValueIndex = 0;
		mCurrentRow.clear();
	}

	//------------------------------------------------------------------------------

	CsvStreamWriter::CsvStreamWriter(std::ostream& outputStream, bool withHeader, char separator,
		Convert::Utf::UtfEncodingErrorPolicy utfEncodingErrorPolicy, const StreamOptions& streamOptions)
		: mEncodedStream(outputStream, streamOptions.encoding, streamOptions.writeBom, utfEncodingErrorPolicy)
		, mWithHeader(withHeader)
		, mSeparator(separator)
	{
		mCsvHeader.reserve(256);
		mCurrentRow.reserve(256);
	}

	void CsvStreamWriter::WriteValue(std::string_view key, std::string_view value)
	{
		BeginValue(key);
		WriteEscapedValue(value, mCurrentRow, mSeparator);
		EndValue();
	}

	void CsvStreamWriter::WriteValue(std::string_view key, std::nullptr_t)
	{
		BeginValue(key);
		EndValue();
	}

	void CsvStreamWriter::BeginValue(std::string_view key)
	{
		// Write keys only when it's first row
		if (mRowIndex == 0 && mWithHeader)
		{
			if (mValueIndex)
			{
				mCsvHeader.push_back(mSeparator);
			}
			WriteEscapedValue(key, mCsvHeader, mSeparator);
		}

		if (mValueIndex)
		{
			mCurrentRow.push_back(mSeparator);
		}
	}

	void CsvStreamWriter::NextLine()
	{
		if (mRowIndex == 0)
		{
			if (mWithHeader)
			{
				mCsvHeader.push_back('\r');
				mCsvHeader.push_back('\n');
				if (mEncodedStream.Write(mCsvHeader) != Convert::Utf::UtfEncodingErrorCode::Success) {
					throw SerializationException(SerializationErrorCode::UtfEncodingError, "Unable to write CSV header, invalid UTF sequence in key(s)");
				}
			}
			mPrevValuesCount = mValueIndex;
		}
		else
		{
			// Compare number of values with previous row
			if (mValueIndex != mPrevValuesCount)
			{
				throw SerializationException(SerializationErrorCode::OutOfRange,
					"Number of values are different than in previous line");
			}
		}

		mCurrentRow.push_back('\r');
		mCurrentRow.push_back('\n');
		if (mEncodedStream.Write(mCurrentRow) != Convert::Utf::UtfEncodingErrorCode::Success) {
			throw SerializationException(SerializationErrorCode::UtfEncodingError, "Unable to write CSV row: " + Convert::ToString(mRowIndex) + ", invalid UTF sequence in value(s)");
		}

		++mRowIndex;
		mValueIndex = 0;
		mCurrentRow.clear();
	}
}
