/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/config.h"

// DLL import/export macro
#if defined(BITSERIALIZER_SHARED_LINK) && defined(BITSERIALIZER_BUILDING_DLL)
    // Building the DLL - export symbols
    #if defined(_WIN32) || defined(_WIN64)
        // Windows platform (MSVC, GCC, Clang)
        #ifdef _MSC_VER
            // MSVC compiler
            #define BITSERIALIZER_API __declspec(dllexport)
        #else
            // GCC or Clang on Windows
            #define BITSERIALIZER_API __attribute__((dllexport))
        #endif
    #else
        // Non-Windows platforms (Linux, macOS, etc.)
        #define BITSERIALIZER_API __attribute__((visibility("default")))
    #endif
#elif defined(BITSERIALIZER_STATIC_LINK)
    // Static library - no export/import semantics
    #define BITSERIALIZER_API
#else
    // Using the DLL - import symbols
    #if defined(_WIN32) || defined(_WIN64)
        // Windows platform (MSVC, GCC, Clang)
        #ifdef _MSC_VER
            // MSVC compiler
            #define BITSERIALIZER_API __declspec(dllimport)
        #else
            // GCC or Clang on Windows
            #define BITSERIALIZER_API __attribute__((dllimport))
        #endif
    #else
        // Non-Windows platforms (Linux, macOS, etc.)
        #define BITSERIALIZER_API __attribute__((visibility("default")))
    #endif
#endif
