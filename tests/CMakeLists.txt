cmake_minimum_required(VERSION 3.9)

project(test_bitserializer)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)

include_directories(
  ${GTEST_INCLUDE_DIRS}
)

add_executable(test_bitserializer json_restcpp_archive_tests.cpp)
target_link_libraries(test_bitserializer PRIVATE GTest::GTest cpprestsdk::cpprest)