/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "json_stream_writers.h"
#include "json_writer_common.h"

namespace BitSerializer::Json::Detail
{
	namespace
	{
		void WriteEscapedString(std::string_view source, Convert::Utf::EncodedStreamWriter& target)
		{
			static constexpr char hexChars[] = "0123456789ABCDEF";
			target.Write("\"");

			size_t pos = 0;
			size_t escapePos = FindFirstEscape(source, pos);
			while (escapePos != std::string_view::npos)
			{
				if (escapePos > pos) {
					target.Write(source.substr(pos, escapePos - pos));
				}

				const unsigned char uc = static_cast<unsigned char>(source[escapePos]);
				switch (uc)
				{
				case '"':  target.Write("\\\""); break;
				case '\\': target.Write("\\\\"); break;
				case '\b': target.Write("\\b"); break;
				case '\f': target.Write("\\f"); break;
				case '\n': target.Write("\\n"); break;
				case '\r': target.Write("\\r"); break;
				case '\t': target.Write("\\t"); break;
				default:
					{
						const char unicodeEscape[6] = { '\\', 'u', '0', '0', hexChars[uc >> 4], hexChars[uc & 0x0F] };
						target.Write(std::string_view(unicodeEscape, sizeof(unicodeEscape)));
					}
					break;
				}

				pos = escapePos + 1;
				escapePos = FindFirstEscape(source, pos);
			}
			if (pos < source.size()) {
				target.Write(source.substr(pos));
			}
			target.Write("\"");
		}
	}

	JsonStreamWriter::JsonStreamWriter(std::ostream& outputStream, const StreamOptions& streamOptions, Convert::Utf::UtfEncodingErrorPolicy encodingErrorPolicy)
		: mEncodedStream(outputStream, streamOptions.encoding, streamOptions.writeBom, encodingErrorPolicy)
	{ }

	void JsonStreamWriter::WriteValue(std::string_view value)
	{
		WriteEscapedString(value, mEncodedStream);
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
		WriteEscapedString(value, mEncodedStream);
	}
}
