cmake_minimum_required(VERSION 3.12)

project(bitserializer_json_restcpp_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
find_package(cpprestsdk CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
  cpprest_json_archive_tests.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    BitSerializer::bitserializer-cpprestjson
    GTest::GTest
    GTest::Main
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerCppRestJsonTests)
