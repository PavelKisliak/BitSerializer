cmake_minimum_required(VERSION 3.12)

project(bitserializer-suite LANGUAGES CXX)

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_SAMPLES "Build samples" OFF)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

message(STATUS "[Option] BUILD_TESTS: ${BUILD_TESTS}")
message(STATUS "[Option] BUILD_SAMPLES: ${BUILD_SAMPLES}")

add_subdirectory(core)
add_subdirectory(archives/bitserializer_cpprest_json)
add_subdirectory(archives/bitserializer_rapidjson)
add_subdirectory(archives/bitserializer_pugixml)

if(BUILD_TESTS)
    include(GoogleTest)
    enable_testing()
    set_tests_properties(${myTests_targets} PROPERTIES TIMEOUT 10)
    add_subdirectory(tests)
endif(BUILD_TESTS)

if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif(BUILD_SAMPLES)
