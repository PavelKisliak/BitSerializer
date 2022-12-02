cmake_minimum_required(VERSION 3.10)

project(bitserializer_rapidjson_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
  rapidjson_archive_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::rapidjson-archive
  GTest::GTest
  GTest::Main
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerRapidJsonTests)
