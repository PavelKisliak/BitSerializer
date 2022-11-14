/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "csv_writers.h"
#include <algorithm>


namespace
{
	using namespace BitSerializer;

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

	void WriteToStreamWithEncoding(std::string_view str, std::ostream& outputStream, Convert::UtfType encoding)
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
		if (mStreamOptions.writeBom)
		{
			WriteBom(mOutputStream, mStreamOptions.encoding);
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
				WriteToStreamWithEncoding(mCsvHeader, mOutputStream, mStreamOptions.encoding);
			}
		}

		mCurrentRow.append({ '\r', '\n' });
		WriteToStreamWithEncoding(mCurrentRow, mOutputStream, mStreamOptions.encoding);

		++mRowIndex;
		mPrevValuesCount = mValueIndex;
		mValueIndex = 0;
		mCurrentRow.clear();
	}
}
