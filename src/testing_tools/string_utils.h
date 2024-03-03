/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

#if defined(__cpp_lib_char8_t)
inline auto u8Cpp20(const char8_t* s)
{
#pragma warning (disable: 26490)
	return reinterpret_cast<const char*>(s);
#pragma warning (default: 26490)
}
#define UTF8(x) u8Cpp20(u8##x)
#else
#define UTF8(x) u8##x
#endif
