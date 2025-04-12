#################################################################################
# Copyright (C) 2020-2025 by Pavel Kisliak
# This file is part of BitSerializer library, licensed under the MIT license.
#################################################################################
#
# Solution for testing the complete release of library (installed via CMake).
#
#################################################################################
cmake_minimum_required(VERSION 3.21)
project(acceptance_tests)

#################################################################################
# Options
#################################################################################
option(BUILD_RAPIDJSON_ARCHIVE "Build RapidJson archive" OFF)
message(STATUS "[Option] BUILD_RAPIDJSON_ARCHIVE: ${BUILD_RAPIDJSON_ARCHIVE}")

option(BUILD_PUGIXML_ARCHIVE "Build PugiXml archive" OFF)
message(STATUS "[Option] BUILD_PUGIXML_ARCHIVE: ${BUILD_PUGIXML_ARCHIVE}")

option(BUILD_RAPIDYAML_ARCHIVE "Build RapidYAML archive" OFF)
message(STATUS "[Option] BUILD_RAPIDYAML_ARCHIVE: ${BUILD_RAPIDYAML_ARCHIVE}")

option(BUILD_CSV_ARCHIVE "Build CSV archive" OFF)
message(STATUS "[Option] BUILD_CSV_ARCHIVE: ${BUILD_CSV_ARCHIVE}")

option(BUILD_MSGPACK_ARCHIVE "Build MsgPack archive" OFF)
message(STATUS "[Option] BUILD_MSGPACK_ARCHIVE: ${BUILD_MSGPACK_ARCHIVE}")

# Unused parameters (keep them only for avoid warnings when use the same set of parameters as for the main project)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BUILD_SAMPLES "Build samples" OFF)
option(STATIC_ANALYSIS_CLANG_TIDY "Static analysis (Clang-tidy)" OFF)

#################################################################################
# Global compiler configuration
#################################################################################
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/permissive- /W3)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wextra -pedantic -Wno-psabi
        -Wno-unknown-pragmas -Wno-unused-but-set-parameter -Wno-unused-but-set-variable -Wno-maybe-uninitialized)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -Wno-unknown-pragmas -Wno-gnu-zero-variadic-macro-arguments)
endif()

#################################################################################
# Find testing library
#################################################################################
find_package(bitserializer CONFIG REQUIRED)

#################################################################################
# Testing tools
#################################################################################
add_library("testing_tools" INTERFACE)
target_include_directories("testing_tools" INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/../../src/>)

#################################################################################
# Configure tests
#################################################################################
include(GoogleTest)
include(CTest)

enable_testing()
set_tests_properties(${myTests_targets} PROPERTIES TIMEOUT 10)

# Reuse integration tests as acceptance tests
add_subdirectory(../integration_tests/convert_api_tests ${CMAKE_CURRENT_BINARY_DIR}/convert_api_tests)

if(BUILD_RAPIDJSON_ARCHIVE)
    add_subdirectory(../integration_tests/rapidjson_archive_tests ${CMAKE_CURRENT_BINARY_DIR}/rapidjson_archive_tests)
endif()

if(BUILD_PUGIXML_ARCHIVE)
    add_subdirectory(../integration_tests/pugixml_archive_tests ${CMAKE_CURRENT_BINARY_DIR}/pugixml_archive_tests)
endif()

if(BUILD_RAPIDYAML_ARCHIVE)
    add_subdirectory(../integration_tests/rapidyaml_archive_tests ${CMAKE_CURRENT_BINARY_DIR}/rapidyaml_archive_tests)
endif()

if(BUILD_CSV_ARCHIVE)
    add_subdirectory(../integration_tests/csv_archive_tests ${CMAKE_CURRENT_BINARY_DIR}/csv_archive_tests)
endif()

if(BUILD_MSGPACK_ARCHIVE)
    add_subdirectory(../integration_tests/msgpack_archive_tests ${CMAKE_CURRENT_BINARY_DIR}/msgpack_archive_tests)
endif()
