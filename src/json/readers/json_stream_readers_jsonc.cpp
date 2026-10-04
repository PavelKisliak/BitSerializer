/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "json_stream_readers_impl.h"

// NOTE: the JSONC instantiation lives in its own translation unit on purpose:
// compiling it together with the strict-JSON one makes MSVC generate slower code for
// the strict reader (see json_stream_readers.cpp for details). Keep one format per TU.

namespace BitSerializer::Json::Detail
{
	template class BITSERIALIZER_API CJsonStreamReader<ArchiveType::Jsonc>;
}
