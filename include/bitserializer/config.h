/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

// Include a standard header first so that the standard library identification macros
// (`__GLIBCXX__`, `_LIBCPP_VERSION`) are defined before the feature detection below,
// regardless of the header inclusion order.
#include <cstddef>

#define BITSERIALIZER_VERSION_MAJOR 0
#define BITSERIALIZER_VERSION_MINOR 91
#define BITSERIALIZER_VERSION_PATCH 0
#define BITSERIALIZER_VERSION (BITSERIALIZER_VERSION_MAJOR * 10000 + BITSERIALIZER_VERSION_MINOR * 100 + BITSERIALIZER_VERSION_PATCH)

#define BITSERIALIZER_STATIC_LINK

#ifndef BITSERIALIZER_HAS_FILESYSTEM
#define BITSERIALIZER_HAS_FILESYSTEM 1
#endif

// `std::from_chars`/`std::to_chars` floating-point overloads come from the standard
// library, not from the compiler. MSVC and libstdc++ 11+ provide the `long double`
// overload; libc++ (Android NDK, Emscripten, FreeBSD, Apple) does not, so these platforms
// use the portable `strto*` conversion. `__GLIBCXX__` moves with patch releases
// (Ubuntu's GCC 9.4 reports a 2021 date, for example), so the check uses
// `_GLIBCXX_RELEASE` instead.
#ifndef BITSERIALIZER_HAS_FLOAT_FROM_CHARS
#if defined(_MSC_VER) || (defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE >= 11)
#define BITSERIALIZER_HAS_FLOAT_FROM_CHARS 1
#else
#define BITSERIALIZER_HAS_FLOAT_FROM_CHARS 0
#endif
#endif

// The installed copy records the configuration detected by CMake, so that consumers
// without the exported targets compile the same code as the prebuilt archives. The copy committed to the source tree keeps the neutral defaults.
#ifndef BITSERIALIZER_HAS_FAST_FLOAT
#define BITSERIALIZER_HAS_FAST_FLOAT 0
#endif

#ifndef BITSERIALIZER_USE_BUNDLED_FAST_FLOAT
#define BITSERIALIZER_USE_BUNDLED_FAST_FLOAT 0
#endif

// SIMD availability of the target (compiler/target detected, not a build-system setting).
// Used by the SIMD helpers shared by the built-in archives (see src/common/simd/).
#ifndef BITSERIALIZER_HAS_SSE2
#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#define BITSERIALIZER_HAS_SSE2 1
#else
#define BITSERIALIZER_HAS_SSE2 0
#endif
#endif

#ifndef BITSERIALIZER_HAS_NEON
#if defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define BITSERIALIZER_HAS_NEON 1
#else
#define BITSERIALIZER_HAS_NEON 0
#endif
#endif

// Inlining control helpers. `BITSERIALIZER_FORCEINLINE` is used for small hot functions
// that must be inlined, while `BITSERIALIZER_NOINLINE` keeps cold functions
// (e.g. error paths, rarely-taken branches) out of the hot loop so it stays small and inlineable.
#if defined(_MSC_VER)
#define BITSERIALIZER_NOINLINE __declspec(noinline)
#define BITSERIALIZER_FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define BITSERIALIZER_NOINLINE __attribute__((noinline))
#define BITSERIALIZER_FORCEINLINE inline __attribute__((always_inline))
#else
#define BITSERIALIZER_NOINLINE
#define BITSERIALIZER_FORCEINLINE inline
#endif
