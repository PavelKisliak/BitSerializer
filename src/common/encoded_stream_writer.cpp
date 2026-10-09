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
		, mTargetUtfType(targetUtfType)
		, mEncodingErrorPolicy(encodingErrorPolicy)
	{
		mByteBuffer.reserve(OutputBufferSize);
		if (addBom)
		{
			WriteBom(mOutputStream, targetUtfType);
		}
	}

	void EncodedStreamWriter::Flush()
	{
		if (!mByteBuffer.empty())
		{
			mOutputStream.write(mByteBuffer.data(), static_cast<std::streamsize>(mByteBuffer.size()));
			mByteBuffer.clear();
		}
	}
}
