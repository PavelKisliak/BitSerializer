cmake_minimum_required(VERSION 3.10)

project(bitserializer_rapidjson_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(RapidJsonTests
  rapidjson_archive_tests.cpp
)

find_package(RapidJSON CONFIG REQUIRED)
target_include_directories(RapidJsonTests PRIVATE ${RAPIDJSON_INCLUDE_DIRS})

target_link_libraries(
  RapidJsonTests
  ${GTEST_BOTH_LIBRARIES} 
)

add_test(RapidJsonTests RapidJsonTests)