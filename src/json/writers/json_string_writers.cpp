/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "json_string_writers.h"
#include "json_writer_common.h"

namespace BitSerializer::Json::Detail
{
	JsonStringWriter::JsonStringWriter(std::string& outputString)
		: mOutputString(outputString)
	{
	}

	void JsonStringWriter::WriteValue(std::string_view value)
	{
		WriteString(value, mOutputString);
	}

	//------------------------------------------------------------------------------
	JsonStringPrettyWriter::JsonStringPrettyWriter(std::string& outputString, char paddingChar, uint16_t paddingCharNum)
		: mOutputString(outputString)
		, mPaddingCharNum(paddingCharNum)
		, mPaddingChar(paddingChar)
	{
	}

	void JsonStringPrettyWriter::WriteValue(std::string_view value)
	{
		WriteIndent();
		WriteString(value, mOutputString);
	}
}
