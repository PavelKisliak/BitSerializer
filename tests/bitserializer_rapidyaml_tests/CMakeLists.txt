cmake_minimum_required(VERSION 3.10)

project(bitserializer_rapidyaml_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/${CMAKE_SYSTEM_NAME}/${CONFIG})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
  rapidyaml_archive_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::rapidyaml-archive
  GTest::GTest
  GTest::Main
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerRapidYamlTests)
