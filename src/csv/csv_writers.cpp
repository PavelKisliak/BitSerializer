/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "csv_writers.h"


namespace
{
	using namespace BitSerializer;

	void WriteEscapedValue(const std::string_view& value, std::string& outputString, const char separator)
	{
		const char* it = value.data();
		const char* endIt = it + value.size();
		for (; it != endIt; ++it)
		{
			const char sym = *it;
			if (sym == '"' || sym == separator || sym == '\n')
			{
				break;
			}
		}

		if (it == endIt)
		{
			// No any characters that must be escaped
			outputString.append(value);
		}
		else
		{
			// RFC: Fields containing line breaks (CRLF), double quotes, and commas should be enclosed in double-quotes
			outputString.push_back('"');
			outputString.append(value.data(), it);

			for (; it != endIt; ++it)
			{
				if (*it == '"')
				{
					// RFC: Double-quote appearing inside a field must be escaped by preceding it with another double quote
					outputString.push_back('"');
				}
				outputString.push_back(*it);
			}
			outputString.push_back('"');
		}
	}

	void WriteToStreamWithEncoding(const std::string_view& str, std::ostream& outputStream, Convert::UtfType encoding)
	{
		switch (encoding)
		{
		case Convert::UtfType::Utf8:
			outputStream.write(str.data(), static_cast<std::streamsize>(str.size()));
			break;
		case Convert::UtfType::Utf16le:
		{
			std::u16string u16LeStr;
			Convert::Utf16Le::Encode(str.cbegin(), str.cend(), u16LeStr);
			outputStream.write(reinterpret_cast<const char*>(u16LeStr.data()),
				static_cast<std::streamsize>(u16LeStr.size() * sizeof(std::u16string::value_type)));
			break;
		}
		case Convert::UtfType::Utf16be:
		{
			std::u16string u16BeStr;
			Convert::Utf16Be::Encode(str.cbegin(), str.cend(), u16BeStr);
			outputStream.write(reinterpret_cast<const char*>(u16BeStr.data()),
				static_cast<std::streamsize>(u16BeStr.size() * sizeof(std::u16string::value_type)));
			break;
		}
		case Convert::UtfType::Utf32le:
		{
			std::u32string u32LeStr;
			Convert::Utf32Le::Encode(str.cbegin(), str.cend(), u32LeStr);
			outputStream.write(reinterpret_cast<const char*>(u32LeStr.data()),
				static_cast<std::streamsize>(u32LeStr.size() * sizeof(std::u32string::value_type)));
			break;
		}
		case Convert::UtfType::Utf32be:
		{
			std::u32string u32BeStr;
			Convert::Utf32Be::Encode(str.cbegin(), str.cend(), u32BeStr);
			outputStream.write(reinterpret_cast<const char*>(u32BeStr.data()),
				static_cast<std::streamsize>(u32BeStr.size() * sizeof(std::u32string::value_type)));
			break;
		}
		}
	}
}


namespace BitSerializer::Csv::Detail
{
	CCsvStringWriter::CCsvStringWriter(std::string& outputString, bool withHeader, char separator)
		: mOutputString(outputString)
		, mWithHeader(withHeader)
		, mSeparator(separator)
	{
		mCurrentRow.reserve(256);
		mOutputString.reserve(256);
	}

	void CCsvStringWriter::SetEstimatedSize(size_t size)
	{
		mEstimatedSize = size;
	}

	void CCsvStringWriter::WriteValue(const std::string_view& key, const std::string& value)
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
		WriteEscapedValue(value, mCurrentRow, mSeparator);
		++mValueIndex;
	}

	void CCsvStringWriter::NextLine()
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
					+ static_cast<size_t>(static_cast<double>(mCurrentRow.size() + CrLfSize) * static_cast<double>(mEstimatedSize) * 1.1);
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

	CCsvStreamWriter::CCsvStreamWriter(std::ostream& outputStream, bool withHeader, char separator, const StreamOptions& streamOptions)
		: mOutputStream(outputStream)
		, mWithHeader(withHeader)
		, mSeparator(separator)
		, mStreamOptions(streamOptions)
	{
		mCsvHeader.reserve(256);
		mCurrentRow.reserve(256);
		if (mStreamOptions.writeBom)
		{
			WriteBom(mOutputStream, mStreamOptions.encoding);
		}
	}

	void CCsvStreamWriter::WriteValue(const std::string_view& key, const std::string& value)
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
		WriteEscapedValue(value, mCurrentRow, mSeparator);
		++mValueIndex;
	}

	void CCsvStreamWriter::NextLine()
	{
		if (mRowIndex == 0)
		{
			if (mWithHeader)
			{
				mCsvHeader.push_back('\r');
				mCsvHeader.push_back('\n');
				WriteToStreamWithEncoding(mCsvHeader, mOutputStream, mStreamOptions.encoding);
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
		WriteToStreamWithEncoding(mCurrentRow, mOutputStream, mStreamOptions.encoding);

		++mRowIndex;
		mValueIndex = 0;
		mCurrentRow.clear();
	}
}
