/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include "bitserializer/json_archive.h"
#include "readers/json_string_readers.h"
#include "readers/json_stream_readers.h"

// Private header holding the `JsonReadRootScope<TFormat>` template definitions.
// They are kept here (rather than in json_write_root_scope.cpp) so that the Json and JSONC
// instantiations can be compiled in separate translation units: compiling both in
// one TU makes MSVC generate slower code for the strict-JSON one.

namespace BitSerializer::Json::Detail
{
	template <ArchiveType TFormat>
	JsonReadRootScope<TFormat>::JsonReadRootScope(std::string_view inputData, SerializationContext& serializationContext)
		: ArchiveScope<SerializeMode::Load>(serializationContext)
		, mJsonReader(std::make_unique<CJsonStringReader<TFormat>>(inputData, serializationContext.GetOptions()).release())
	{ }

	template <ArchiveType TFormat>
	JsonReadRootScope<TFormat>::JsonReadRootScope(std::istream& inputStream, SerializationContext& serializationContext)
		: ArchiveScope<SerializeMode::Load>(serializationContext)
		, mJsonReader(std::make_unique<CJsonStreamReader<TFormat>>(inputStream, serializationContext.GetOptions()).release())
	{ }

	template <ArchiveType TFormat>
	JsonReadRootScope<TFormat>::~JsonReadRootScope()
	{
		delete mJsonReader;
	}
}
