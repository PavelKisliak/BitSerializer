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

option(BUILD_JSON_ARCHIVE "Build JSON archive" OFF)
message(STATUS "[Option] BUILD_JSON_ARCHIVE: ${BUILD_JSON_ARCHIVE}")

# Register one aggregated CTest entry per test executable (whole gtest binary as a single test).
# Used by the Valgrind CI job to avoid paying the Valgrind process startup cost for every gtest case.
option(BUILD_MEMCHECK_AGGREGATE_TESTS "Register aggregated tests for memory checking" OFF)
message(STATUS "[Option] BUILD_MEMCHECK_AGGREGATE_TESTS: ${BUILD_MEMCHECK_AGGREGATE_TESTS}")

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
    add_compile_options(/permissive- /W3 /bigobj)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wextra -pedantic -Wno-psabi
        -Wno-unknown-pragmas -Wno-unused-but-set-parameter -Wno-unused-but-set-variable -Wno-maybe-uninitialized)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -Wno-unknown-pragmas -Wno-gnu-zero-variadic-macro-arguments)
endif()

#################################################################################
# Find testing library
#################################################################################
# Request only enabled archives so their optional dependencies are discovered.
set(_bitserializer_components)
if(BUILD_JSON_ARCHIVE)
    list(APPEND _bitserializer_components json)
endif()
if(BUILD_CSV_ARCHIVE)
    list(APPEND _bitserializer_components csv)
endif()
if(BUILD_MSGPACK_ARCHIVE)
    list(APPEND _bitserializer_components msgpack)
endif()
if(BUILD_RAPIDJSON_ARCHIVE)
    list(APPEND _bitserializer_components rapidjson)
endif()
if(BUILD_PUGIXML_ARCHIVE)
    list(APPEND _bitserializer_components pugixml)
endif()
if(BUILD_RAPIDYAML_ARCHIVE)
    list(APPEND _bitserializer_components rapidyaml)
endif()

if(_bitserializer_components)
    find_package(bitserializer CONFIG REQUIRED COMPONENTS ${_bitserializer_components})
else()
    find_package(bitserializer CONFIG REQUIRED)
endif()

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

if(BUILD_JSON_ARCHIVE)
    add_subdirectory(../integration_tests/json_archive_tests ${CMAKE_CURRENT_BINARY_DIR}/json_archive_tests)
endif()

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

#################################################################################
# Aggregated tests for memory checking
#################################################################################
# Running a whole test executable as a single CTest test lets Valgrind analyze it in one
# process instead of starting a new Valgrind instance for every gtest case (per-case startup
# dominates the memory check time with a Debug build).
if(BUILD_MEMCHECK_AGGREGATE_TESTS)
    set(_memcheck_targets convert_api_tests)
    if(BUILD_JSON_ARCHIVE)
        list(APPEND _memcheck_targets json_archive_tests)
    endif()
    if(BUILD_RAPIDJSON_ARCHIVE)
        list(APPEND _memcheck_targets rapidjson_archive_tests)
    endif()
    if(BUILD_PUGIXML_ARCHIVE)
        list(APPEND _memcheck_targets pugixml_archive_tests)
    endif()
    if(BUILD_RAPIDYAML_ARCHIVE)
        list(APPEND _memcheck_targets rapidyaml_archive_tests)
    endif()
    if(BUILD_CSV_ARCHIVE)
        list(APPEND _memcheck_targets csv_archive_tests)
    endif()
    if(BUILD_MSGPACK_ARCHIVE)
        list(APPEND _memcheck_targets msgpack_archive_tests)
    endif()

    foreach(_target IN LISTS _memcheck_targets)
        add_test(NAME "${_target}_aggregate" COMMAND ${_target} --gtest_color=no)
        set_tests_properties("${_target}_aggregate" PROPERTIES
            LABELS "memcheck_aggregate"
            TIMEOUT 1800
        )
    endforeach()
endif()
