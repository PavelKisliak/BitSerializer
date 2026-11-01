/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "json_string_writers.h"
#include "json_writer_common.h"

namespace BitSerializer::Json::Detail
{
	JsonStringWriter::JsonStringWriter(std::string& outputString)
		: mBuffer(outputString)
	{
	}

	JsonStringWriter::~JsonStringWriter()
	{
		if (!mFlushed) {
			try {
				mBuffer.Finalize();
			}
			catch (...) {
				// A destructor cannot report output-finalization errors.
				(void)0;
			}
		}
	}

	void JsonStringWriter::WriteValue(std::string_view value)
	{
		WriteString(value, mBuffer);
	}

	//------------------------------------------------------------------------------
	JsonStringPrettyWriter::JsonStringPrettyWriter(std::string& outputString, char paddingChar, uint16_t paddingCharNum)
		: mBuffer(outputString)
		, mPaddingCharNum(paddingCharNum)
		, mPaddingChar(paddingChar)
	{
	}

	JsonStringPrettyWriter::~JsonStringPrettyWriter()
	{
		if (!mFlushed) {
			try {
				mBuffer.Finalize();
			}
			catch (...) {
				// A destructor cannot report output-finalization errors.
				(void)0;
			}
		}
	}

	void JsonStringPrettyWriter::WriteValue(std::string_view value)
	{
		WriteIndent();
		WriteString(value, mBuffer);
	}
}
