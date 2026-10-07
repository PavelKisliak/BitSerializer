/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "json_stream_writers.h"
#include "json_writer_common.h"

namespace BitSerializer::Json::Detail
{
	JsonStreamWriter::JsonStreamWriter(std::ostream& outputStream, const StreamOptions& streamOptions, Convert::Utf::UtfEncodingErrorPolicy encodingErrorPolicy)
		: mEncodedStream(outputStream, streamOptions.encoding, streamOptions.writeBom, encodingErrorPolicy)
	{ }

	void JsonStreamWriter::WriteValue(std::string_view value)
	{
		mStringBuffer.clear();
		WriteString(value, mStringBuffer);
		mEncodedStream.Write(std::string_view(mStringBuffer));
	}

	//------------------------------------------------------------------------------

	JsonStreamPrettyWriter::JsonStreamPrettyWriter(std::ostream& outputStream, const StreamOptions& streamOptions,
		char paddingChar, uint16_t paddingCharNum, Convert::Utf::UtfEncodingErrorPolicy encodingErrorPolicy)
		: mEncodedStream(outputStream, streamOptions.encoding, streamOptions.writeBom, encodingErrorPolicy)
		, mPaddingCharNum(paddingCharNum)
		, mPaddingChar(paddingChar)
	{ }

	void JsonStreamPrettyWriter::WriteValue(std::string_view value)
	{
		WriteIndent();
		mStringBuffer.clear();
		WriteString(value, mStringBuffer);
		mEncodedStream.Write(std::string_view(mStringBuffer));
	}
}
