#################################################################################
# Copyright (C) 2020-2025 by Pavel Kisliak
# This file is part of BitSerializer library, licensed under the MIT license.
#################################################################################
cmake_minimum_required(VERSION 3.10)

# Set VCPKG toolchain if it does not provided (must be set before the project())
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()
set(VCPKG_FEATURE_FLAGS "versions")

project(bitserializer
    VERSION 0.75.0
    DESCRIPTION "C++ 17 library for serialization to multiple output formats (JSON, XML, YAML, CSV, MsgPack)"
    LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

#################################################################################
# Options
#################################################################################
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
message(STATUS "[Option] BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")

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

option(BUILD_TESTS "Build tests" OFF)
message(STATUS "[Option] BUILD_TESTS: ${BUILD_TESTS}")

option(BUILD_BENCHMARKS "Build benchmarks" OFF)
message(STATUS "[Option] BUILD_BENCHMARKS: ${BUILD_BENCHMARKS}")

option(BUILD_SAMPLES "Build samples" OFF)
message(STATUS "[Option] BUILD_SAMPLES: ${BUILD_SAMPLES}")

option(STATIC_ANALYSIS_CLANG_TIDY "Static analysis (Clang-tidy)" OFF)
message(STATUS "[Option] STATIC_ANALYSIS_CLANG_TIDY: ${STATIC_ANALYSIS_CLANG_TIDY}")

#################################################################################
# Global compiler configuration
#################################################################################
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0")
    message(WARNING "BitSerializer requires at least clang-8.0")
endif()
if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.3")
    message(WARNING "BitSerializer requires at least gcc-8.3")
endif()

if(MSVC)
    add_compile_options(/permissive- /W3)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wextra -pedantic -Wno-psabi
        -Wno-unknown-pragmas -Wno-unused-but-set-parameter -Wno-unused-but-set-variable -Wno-maybe-uninitialized)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -Wno-unknown-pragmas -Wno-gnu-zero-variadic-macro-arguments)
endif()

# For debug libs, add "d" postfix
if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX "d")
endif()

include_directories(
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

# Configuration the linkage type of the library (shared or static)
if(BUILD_SHARED_LIBS)
    set(BITSERIALIZER_LINK_TYPE "BITSERIALIZER_SHARED_LINK")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
else()
    set(BITSERIALIZER_LINK_TYPE "BITSERIALIZER_STATIC_LINK")
endif()

# Configuration the output directory for all binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Automatic detection of std::filesystem availability
if(NOT DEFINED BITSERIALIZER_HAS_FILESYSTEM)
    include(CheckCXXSymbolExists)
    check_cxx_symbol_exists(std::filesystem::path::preferred_separator filesystem BITSERIALIZER_HAS_FILESYSTEM)
endif()

#################################################################################
# Static analysis
#################################################################################
if(STATIC_ANALYSIS_CLANG_TIDY)
    if(MSVC)
        find_program(CLANG_TIDY_PROGRAM NAMES "clang-tidy" PATHS "$ENV{ProgramFiles}/Microsoft Visual Studio/*/*/VC/Tools/Llvm/x64/bin/")
    else()
        find_program(CLANG_TIDY_PROGRAM NAMES "clang-tidy")
    endif()

    if (CLANG_TIDY_PROGRAM)
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
        set(CLANG_TIDY_COMMAND "${CLANG_TIDY_PROGRAM}" "--config-file=${PROJECT_SOURCE_DIR}/.clang-tidy")

        # Note: analysis of RapidYaml archive doesn't work due to incorrecly detected C4_MSVC (c4\compiler.hpp)
        if(MSVC)
            set(CLANG_TIDY_COMMAND "${CLANG_TIDY_PROGRAM};--extra-arg=/EHsc")
        endif()

        # Apply clang-tidy globally to all C++ targets
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
    else()
        message(WARNING "Could not find clang-tidy program, static analysis disabled")
    endif()
endif()

#################################################################################
# Targets configuration
#################################################################################
set(BITSERIALIZER_NAMESPACE "BitSerializer")

configure_file("${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.h.in" "${CMAKE_CURRENT_LIST_DIR}/include/${PROJECT_NAME}/config.h")

# Private implementation of common code (object files)
add_library(common_impl OBJECT
    "src/common/binary_stream_reader.h"
    "src/common/binary_stream_reader.cpp"
)

# BitSerializer core
set(BITSERIALIZER_CORE_NAME "core")
set(BITSERIALIZER_TARGETS ${BITSERIALIZER_CORE_NAME})
add_library(${BITSERIALIZER_CORE_NAME} INTERFACE)
add_library(${BITSERIALIZER_NAMESPACE}::${BITSERIALIZER_CORE_NAME} ALIAS ${BITSERIALIZER_CORE_NAME})

target_include_directories(${BITSERIALIZER_CORE_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(${BITSERIALIZER_CORE_NAME} INTERFACE cxx_std_17)

if(BITSERIALIZER_HAS_FILESYSTEM)
    if(CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
            target_link_libraries(${BITSERIALIZER_CORE_NAME} INTERFACE stdc++fs)
        endif()
    endif()
endif()

# BitSerializer rapidjson archive
if(BUILD_RAPIDJSON_ARCHIVE)
    set(RAPIDJSON_ARCHIVE_NAME "rapidjson-archive")
    add_library(${RAPIDJSON_ARCHIVE_NAME} INTERFACE)
    add_library(${BITSERIALIZER_NAMESPACE}::${RAPIDJSON_ARCHIVE_NAME} ALIAS ${RAPIDJSON_ARCHIVE_NAME})
    list(APPEND BITSERIALIZER_TARGETS ${RAPIDJSON_ARCHIVE_NAME})

    set_target_properties(${RAPIDJSON_ARCHIVE_NAME}
        PROPERTIES
            OUTPUT_NAME "bitserializer-rapidjson"
    )

    find_package(RapidJSON CONFIG REQUIRED)
    target_link_libraries(${RAPIDJSON_ARCHIVE_NAME} INTERFACE
        ${BITSERIALIZER_NAMESPACE}::${BITSERIALIZER_CORE_NAME}
        rapidjson
    )
endif()

# BitSerializer pugixml archive
if(BUILD_PUGIXML_ARCHIVE)
    set(PUGIXML_ARCHIVE_NAME "pugixml-archive")
    add_library(${PUGIXML_ARCHIVE_NAME} INTERFACE)
    add_library(${BITSERIALIZER_NAMESPACE}::${PUGIXML_ARCHIVE_NAME} ALIAS ${PUGIXML_ARCHIVE_NAME})
    list(APPEND BITSERIALIZER_TARGETS ${PUGIXML_ARCHIVE_NAME})

    set_target_properties(${PUGIXML_ARCHIVE_NAME}
        PROPERTIES
            OUTPUT_NAME "bitserializer-pugixml"
    )

    find_package(pugixml CONFIG REQUIRED)
    target_link_libraries(${PUGIXML_ARCHIVE_NAME} INTERFACE
        ${BITSERIALIZER_NAMESPACE}::${BITSERIALIZER_CORE_NAME}
        pugixml::pugixml
    )
endif()

# BitSerializer rapidyaml archive
if(BUILD_RAPIDYAML_ARCHIVE)
    set(RAPIDYAML_ARCHIVE_NAME "rapidyaml-archive")
    add_library(${RAPIDYAML_ARCHIVE_NAME} INTERFACE)
    add_library(${BITSERIALIZER_NAMESPACE}::${RAPIDYAML_ARCHIVE_NAME} ALIAS ${RAPIDYAML_ARCHIVE_NAME})
    list(APPEND BITSERIALIZER_TARGETS ${RAPIDYAML_ARCHIVE_NAME})

    set_target_properties(${RAPIDYAML_ARCHIVE_NAME}
        PROPERTIES
            OUTPUT_NAME "bitserializer-ryaml"
    )

    find_package(ryml CONFIG REQUIRED)
    target_link_libraries(${RAPIDYAML_ARCHIVE_NAME} INTERFACE
        ${BITSERIALIZER_NAMESPACE}::${BITSERIALIZER_CORE_NAME}
        ryml::ryml
    )
endif()

# BitSerializer CSV archive
if(BUILD_CSV_ARCHIVE)
    # CSV archive object files
    add_library(csv_impl OBJECT
        "src/csv/csv_archive.cpp"
        "src/csv/csv_readers.h" "src/csv/csv_readers.cpp"
        "src/csv/csv_writers.h" "src/csv/csv_writers.cpp"
    )
    target_include_directories(csv_impl PRIVATE "src/")
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(csv_impl PRIVATE BITSERIALIZER_BUILDING_DLL)
    endif()

    # CSV archive public library
    set(CSV_ARCHIVE_NAME "csv-archive")
    if(BUILD_SHARED_LIBS)
        add_library(${CSV_ARCHIVE_NAME} SHARED $<TARGET_OBJECTS:csv_impl>)
    else()
        add_library(${CSV_ARCHIVE_NAME} STATIC $<TARGET_OBJECTS:csv_impl>)
    endif()
    add_library(${BITSERIALIZER_NAMESPACE}::${CSV_ARCHIVE_NAME} ALIAS ${CSV_ARCHIVE_NAME})
    list(APPEND BITSERIALIZER_TARGETS ${CSV_ARCHIVE_NAME})

    set_target_properties(${CSV_ARCHIVE_NAME}
        PROPERTIES
            OUTPUT_NAME "bitserializer-csv"
    )

    target_link_libraries(${CSV_ARCHIVE_NAME} INTERFACE
        ${BITSERIALIZER_NAMESPACE}::${BITSERIALIZER_CORE_NAME}
    )
endif()

# BitSerializer MsgPack archive
if(BUILD_MSGPACK_ARCHIVE)
    # MsgPack archive object files
    add_library(msgpack_impl OBJECT
        "src/msgpack/msgpack_archive.cpp"
        "src/msgpack/msgpack_readers.h" "src/msgpack/msgpack_readers.cpp"
        "src/msgpack/msgpack_writers.h" "src/msgpack/msgpack_writers.cpp"
    )
    target_include_directories(msgpack_impl PRIVATE "src/")
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(msgpack_impl PRIVATE BITSERIALIZER_BUILDING_DLL)
    endif()

    # MsgPack archive public library
    set(MSGPACK_ARCHIVE_NAME "msgpack-archive")
    if(BUILD_SHARED_LIBS)
        add_library(${MSGPACK_ARCHIVE_NAME} SHARED $<TARGET_OBJECTS:msgpack_impl> $<TARGET_OBJECTS:common_impl>)
    else()
        add_library(${MSGPACK_ARCHIVE_NAME} STATIC $<TARGET_OBJECTS:msgpack_impl> $<TARGET_OBJECTS:common_impl>)
    endif()
    add_library(${BITSERIALIZER_NAMESPACE}::${MSGPACK_ARCHIVE_NAME} ALIAS ${MSGPACK_ARCHIVE_NAME})
    list(APPEND BITSERIALIZER_TARGETS ${MSGPACK_ARCHIVE_NAME})

    set_target_properties(${MSGPACK_ARCHIVE_NAME}
        PROPERTIES
            OUTPUT_NAME "bitserializer-msgpack"
    )
    
    target_link_libraries(${MSGPACK_ARCHIVE_NAME} INTERFACE
        ${BITSERIALIZER_NAMESPACE}::${BITSERIALIZER_CORE_NAME}
    )
endif()

#################################################################################
# Tests (optional)
#################################################################################
if(BUILD_TESTS OR BUILD_BENCHMARKS)
    add_subdirectory(src/testing_tools)
endif()

if(BUILD_TESTS)
    include(GoogleTest)
    include(CTest)

    # Configure memory check
    if(CMAKE_MEMORYCHECK_COMMAND)
        set(MEMORYCHECK_COMMAND "${CMAKE_MEMORYCHECK_COMMAND}")
        set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --error-exitcode=1")
    endif()

    # Configure tests
    enable_testing()
    set_tests_properties(${myTests_targets} PROPERTIES TIMEOUT 10)
    add_subdirectory(tests/unit_tests)
    add_subdirectory(tests/integration_tests)
endif()

#################################################################################
# Benchmarks (optional)
#################################################################################
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks/archives)
endif()

#################################################################################
# Samples (optional)
#################################################################################
if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

#################################################################################
# Installation
#################################################################################
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})

install(TARGETS ${BITSERIALIZER_TARGETS}
    EXPORT ${PROJECT_NAME}_Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY ExactVersion)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

install(EXPORT ${PROJECT_NAME}_Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${BITSERIALIZER_NAMESPACE}::
    DESTINATION ${INSTALL_CONFIGDIR})

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${INSTALL_CONFIGDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/bitserializer
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h" PATTERN "*_archive.h" EXCLUDE)

if(BUILD_RAPIDJSON_ARCHIVE)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/bitserializer/rapidjson_archive.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bitserializer)
endif()

if(BUILD_PUGIXML_ARCHIVE)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/bitserializer/pugixml_archive.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bitserializer)
endif()

if(BUILD_RAPIDYAML_ARCHIVE)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/bitserializer/rapidyaml_archive.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bitserializer)
endif()

if(BUILD_CSV_ARCHIVE)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/bitserializer/csv_archive.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bitserializer)
endif()

if(BUILD_MSGPACK_ARCHIVE)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/bitserializer/msgpack_archive.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bitserializer)
endif()
