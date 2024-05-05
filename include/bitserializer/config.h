/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

#define BITSERIALIZER_VERSION_MAJOR 0
#define BITSERIALIZER_VERSION_MINOR 65
#define BITSERIALIZER_VERSION_PATCH 0
#define BITSERIALIZER_VERSION (BITSERIALIZER_VERSION_MAJOR * 10000 + BITSERIALIZER_VERSION_MINOR * 100 + BITSERIALIZER_VERSION_PATCH)

#ifndef BITSERIALIZER_HAS_FILESYSTEM
#define BITSERIALIZER_HAS_FILESYSTEM 1
#endif

#ifndef BITSERIALIZER_HAS_FLOAT_FROM_CHARS
#if defined(_MSC_VER) || (defined __GNUC__ && __GNUC__ > 11) || (defined __clang__ && __clang_major__ > 13)
#define BITSERIALIZER_HAS_FLOAT_FROM_CHARS 1
#else
#define BITSERIALIZER_HAS_FLOAT_FROM_CHARS 0
#endif
#endif
