/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "json_string_writers.h"
#include "json_writer_common.h"

namespace BitSerializer::Json::Detail
{
	CJsonStringWriter::CJsonStringWriter(std::string& outputString)
		: mOutputString(outputString)
	{
	}

	void CJsonStringWriter::WriteValue(std::string_view value)
	{
		WriteString(value, mOutputString);
	}

	//------------------------------------------------------------------------------
	CJsonStringPrettyWriter::CJsonStringPrettyWriter(std::string& outputString, char paddingChar, uint16_t paddingCharNum)
		: mOutputString(outputString)
		, mPaddingCharNum(paddingCharNum)
		, mPaddingChar(paddingChar)
	{
	}

	void CJsonStringPrettyWriter::WriteValue(std::string_view value)
	{
		WriteIndent();
		WriteString(value, mOutputString);
	}
}
