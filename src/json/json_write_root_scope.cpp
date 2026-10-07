/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <memory>
#include "writers/json_string_writers.h"
#include "writers/json_stream_writers.h"
#include "bitserializer/json_archive.h"


namespace BitSerializer::Json::Detail
{
	JsonWriteRootScopeBase::JsonWriteRootScopeBase(std::string& outputData, SerializationContext& serializationContext)
		: ArchiveScope<SerializeMode::Save>(serializationContext)
	{
		// Created via make_unique + release and kept as a raw pointer on purpose: using std::unique_ptr
		// caused a measurable MSVC codegen regression in this hot path (bug reproduced on the reader side).
		const auto& formatOptions = serializationContext.GetOptions().formatOptions;
		if (formatOptions.enableFormat)
		{
			mJsonWriter = std::make_unique<JsonStringPrettyWriter>(outputData, formatOptions.paddingChar, formatOptions.paddingCharNum).release();
		}
		else
		{
			mJsonWriter = std::make_unique<JsonStringWriter>(outputData).release();
		}
	}

	JsonWriteRootScopeBase::JsonWriteRootScopeBase(std::ostream& outputStream, SerializationContext& serializationContext)
		: ArchiveScope<SerializeMode::Save>(serializationContext)
	{
		// raw pointer on purpose (see the note in the constructor above)
		const auto& options = serializationContext.GetOptions();
		switch (options.streamOptions.encoding)
		{
		case Convert::Utf::UtfType::Utf8:
		case Convert::Utf::UtfType::Utf16le:
		case Convert::Utf::UtfType::Utf16be:
		case Convert::Utf::UtfType::Utf32le:
		case Convert::Utf::UtfType::Utf32be:
			break;
		default:
			throw SerializationException(SerializationErrorCode::UnsupportedEncoding);
		}

		if (options.formatOptions.enableFormat)
		{
			mJsonWriter = std::make_unique<JsonStreamPrettyWriter>(outputStream, options.streamOptions,
				options.formatOptions.paddingChar, options.formatOptions.paddingCharNum,
				options.utfEncodingErrorPolicy).release();
		}
		else
		{
			mJsonWriter = std::make_unique<JsonStreamWriter>(outputStream, options.streamOptions,
				options.utfEncodingErrorPolicy).release();
		}
	}

	JsonWriteRootScopeBase::~JsonWriteRootScopeBase()
	{
		delete mJsonWriter;
	}

}
