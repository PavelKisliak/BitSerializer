/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "json_stream_readers_impl.h"

// NOTE: this explicit instantiation intentionally lives in its own translation unit.
// MSVC generates noticeably slower code for the strict-JSON reader when the larger
// JSONC instantiation is compiled in the same TU, even though `if constexpr` removes
// all JSONC handling from the strict path. Keep exactly one format per TU.

namespace BitSerializer::Json::Detail
{
	template class JsonStreamReader<ArchiveType::Json>;
}
