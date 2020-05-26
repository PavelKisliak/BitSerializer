cmake_minimum_required(VERSION 3.10)

project(bitserializer-suite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_CPPRESTJSON_ARCHIVE "Build CppRestJson archive" ON)
option(BUILD_RAPIDJSON_ARCHIVE "Build RapidJson archive" ON)
option(BUILD_PUGIXML_ARCHIVE "Build PugiXml archive" ON)
option(BUILD_RAPIDYAML_ARCHIVE "Build RapidYAML archive" ON)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_SAMPLES "Build samples" OFF)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0")
    message(FATAL_ERROR "BitSerializer requires at least clang-8.0")
endif()
if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.3")
    message(FATAL_ERROR "BitSerializer requires at least gcc-8.3")
endif()

message(STATUS "[Option] BUILD_CPPRESTJSON_ARCHIVE: ${BUILD_CPPRESTJSON_ARCHIVE}")
message(STATUS "[Option] BUILD_RAPIDJSON_ARCHIVE: ${BUILD_RAPIDJSON_ARCHIVE}")
message(STATUS "[Option] BUILD_PUGIXML_ARCHIVE: ${BUILD_PUGIXML_ARCHIVE}")
message(STATUS "[Option] BUILD_RAPIDYAML_ARCHIVE: ${BUILD_RAPIDYAML_ARCHIVE}")
message(STATUS "[Option] BUILD_TESTS: ${BUILD_TESTS}")
message(STATUS "[Option] BUILD_SAMPLES: ${BUILD_SAMPLES}")

if(MSVC)
    add_compile_options(/permissive-)
else()
    add_compile_options(-Wall -Wextra -pedantic
        -Wno-unknown-pragmas -Wno-unused-function -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable)
endif()


add_subdirectory(core)

if(BUILD_CPPRESTJSON_ARCHIVE)
    add_subdirectory(archives/bitserializer_cpprest_json)
endif(BUILD_CPPRESTJSON_ARCHIVE)

if(BUILD_RAPIDJSON_ARCHIVE)
    add_subdirectory(archives/bitserializer_rapidjson)
endif(BUILD_RAPIDJSON_ARCHIVE)

if(BUILD_PUGIXML_ARCHIVE)
    add_subdirectory(archives/bitserializer_pugixml)
endif(BUILD_PUGIXML_ARCHIVE)

if(BUILD_RAPIDYAML_ARCHIVE)
    add_subdirectory(archives/bitserializer_rapidyaml)
endif(BUILD_RAPIDYAML_ARCHIVE)


if(BUILD_TESTS)
    include(GoogleTest)
    enable_testing()
    set_tests_properties(${myTests_targets} PROPERTIES TIMEOUT 10)
    add_subdirectory(tests)
endif(BUILD_TESTS)

if(BUILD_SAMPLES)
    if(BUILD_CPPRESTJSON_ARCHIVE AND BUILD_RAPIDJSON_ARCHIVE AND BUILD_PUGIXML_ARCHIVE)
       add_subdirectory(samples)
    else()
       message(WARNING "Disable build samples in connection with disabled build archives")
    endif()
endif(BUILD_SAMPLES)
