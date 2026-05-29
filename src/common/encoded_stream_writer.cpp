/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "encoded_stream_writer.h"

namespace BitSerializer::Convert::Utf
{
	namespace
	{
		void WriteBom(std::ostream& outputStream, UtfType encoding)
		{
			switch (encoding)
			{
			case UtfType::Utf8:
				outputStream.write(Utf8::bom, sizeof Utf8::bom);
				break;
			case UtfType::Utf16le:
				outputStream.write(Utf16Le::bom, sizeof Utf16Le::bom);
				break;
			case UtfType::Utf16be:
				outputStream.write(Utf16Be::bom, sizeof Utf16Be::bom);
				break;
			case UtfType::Utf32le:
				outputStream.write(Utf32Le::bom, sizeof Utf32Le::bom);
				break;
			case UtfType::Utf32be:
				outputStream.write(Utf32Be::bom, sizeof Utf32Be::bom);
				break;
			}
		}
	}

	EncodedStreamWriter::EncodedStreamWriter(std::ostream& outputStream, UtfType targetUtfType, bool addBom, UtfEncodingErrorPolicy encodingErrorPolicy)
		: mOutputStream(outputStream)
		, mEncodingErrorPolicy(encodingErrorPolicy)
	{
		switch (targetUtfType)
		{
		case UtfType::Utf8:
			mUtfToolset.emplace<std::pair<Utf8, std::string>>();
			break;
		case UtfType::Utf16le:
			mUtfToolset.emplace<std::pair<Utf16Le, std::u16string>>();
			break;
		case UtfType::Utf16be:
			mUtfToolset.emplace<std::pair<Utf16Be, std::u16string>>();
			break;
		case UtfType::Utf32le:
			mUtfToolset.emplace<std::pair<Utf32Le, std::u32string>>();
			break;
		case UtfType::Utf32be:
			mUtfToolset.emplace<std::pair<Utf32Be, std::u32string>>();
			break;
		}

		if (addBom)
		{
			WriteBom(mOutputStream, targetUtfType);
		}
	}
}
