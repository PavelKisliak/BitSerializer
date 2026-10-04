/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "json_read_root_scope.h"

// NOTE: explicit instantiation is split per format into separate translation units
// for the same reason as the readers: MSVC generates slower code for the strict-JSON
// instantiation when the JSONC one is compiled in the same TU. Keep one format per TU.

namespace BitSerializer::Json::Detail
{
	template class BITSERIALIZER_API JsonReadRootScope<ArchiveType::Jsonc>;
}
