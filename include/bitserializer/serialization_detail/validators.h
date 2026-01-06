/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

#if defined(__GNUC__) || defined(__clang__)
    #warning "Header "bitserializer/serialization_detail/validators.h" is deprecated. Include "bitserializer/validate.h" instead."
#elif defined(_MSC_VER)
    #pragma message("Header \"bitserializer/serialization_detail/validators.h\" is deprecated. Include \"bitserializer/validate.h\" instead.")
#endif

#include "bitserializer/validate.h"
