/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <memory>
#include "json_string_readers.h"
#include "json_string_writers.h"
#include "bitserializer/json_archive.h"


namespace BitSerializer::Json::Detail
{
	JsonWriteRootScope::JsonWriteRootScope(std::string& outputData, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Save>(serializationContext)
	{
		const auto& formatOptions = serializationContext.GetOptions().formatOptions;
		if (formatOptions.enableFormat)
		{
			mJsonWriter = std::make_unique<CJsonStringPrettyWriter>(outputData, formatOptions.paddingChar, formatOptions.paddingCharNum).release();
		}
		else
		{
			mJsonWriter = std::make_unique<CJsonStringWriter>(outputData).release();
		}
	}

	//JsonWriteRootScope::JsonWriteRootScope(std::ostream& outputStream, SerializationContext& serializationContext)
	//	: TArchiveScope<SerializeMode::Save>(serializationContext)
	//	, mJsonWriter(std::make_unique<CJsonStreamWriter>(outputStream).release())
	//{ }

	JsonWriteRootScope::~JsonWriteRootScope()
	{
		delete mJsonWriter;
	}

	JsonReadRootScope::JsonReadRootScope(std::string_view inputData, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Load>(serializationContext)
		, mJsonReader(std::make_unique<CJsonStringReader>(inputData, serializationContext.GetOptions()).release())
	{ }

	//JsonReadRootScope::JsonReadRootScope(std::istream& inputStream, SerializationContext& serializationContext)
	//	: TArchiveScope<SerializeMode::Load>(serializationContext)
	//	, mJsonReader(std::make_unique<CJsonStreamReader>(inputStream, serializationContext.GetOptions()).release())
	//{ }

	JsonReadRootScope::~JsonReadRootScope()
	{
		delete mJsonReader;
	}
}
