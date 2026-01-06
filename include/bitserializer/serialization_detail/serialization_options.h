/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

#if defined(__GNUC__) || defined(__clang__)
    #warning "Header "bitserializer/serialization_detail/serialization_options.h" is deprecated. Include "bitserializer/serialization_options.h" instead."
#elif defined(_MSC_VER)
    #pragma message("Header \"bitserializer/serialization_detail/serialization_options.h\" is deprecated. Include \"bitserializer/serialization_options.h\" instead.")
#endif

#include "bitserializer/serialization_options.h"
