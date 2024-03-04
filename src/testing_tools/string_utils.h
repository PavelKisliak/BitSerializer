/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

#if defined(__cpp_lib_char8_t)
inline auto makeUtf8(const char8_t* s)
{
#pragma warning (disable: 26490)
	return reinterpret_cast<const char*>(s);
#pragma warning (default: 26490)
}
// Macros for definition a cross-platform UTF-8 string
#define UTF8(x) makeUtf8(u8##x)
#else
#define UTF8(x) u8##x
#endif


// Macros for definition a cross-platform path
#ifdef _WIN32
#define UPATH(x) L##x
#else
#define UPATH(x) UTF8(x)
#endif
