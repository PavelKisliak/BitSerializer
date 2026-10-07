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
// They are kept here (rather than in json_read_root_scope.cpp) so that the Json and JSONC
// instantiations can be compiled in separate translation units: compiling both in
// one TU makes MSVC generate slower code for the strict-JSON one.

namespace BitSerializer::Json::Detail
{
	template <ArchiveType TFormat>
	JsonReadRootScope<TFormat>::JsonReadRootScope(std::string_view inputData, SerializationContext& serializationContext)
		: ArchiveScope<SerializeMode::Load>(serializationContext)
		// Kept as a raw pointer (make_unique + release) on purpose: the std::unique_ptr version
		// caused a measurable MSVC codegen regression in this hot load path.
		, mJsonReader(std::make_unique<JsonStringReader<TFormat>>(inputData, serializationContext.GetOptions()).release())
	{ }

	template <ArchiveType TFormat>
	JsonReadRootScope<TFormat>::JsonReadRootScope(std::istream& inputStream, SerializationContext& serializationContext)
		: ArchiveScope<SerializeMode::Load>(serializationContext)
		// Raw pointer on purpose (see the note in the constructor above).
		, mJsonReader(std::make_unique<JsonStreamReader<TFormat>>(inputStream, serializationContext.GetOptions()).release())
	{ }

	template <ArchiveType TFormat>
	JsonReadRootScope<TFormat>::~JsonReadRootScope()
	{
		delete mJsonReader;
	}
}
