cmake_minimum_required(VERSION 3.10)

project(bitserializer_json_restcpp_tests)

set(CMAKE_CXX_STANDARD 17)

enable_testing()
find_package(GTest REQUIRED)

include_directories(
  ${GTEST_INCLUDE_DIRS}
)

add_executable(JsonRestCppTests 
  json_restcpp_archive_tests.cpp
)

target_link_libraries(
  JsonRestCppTests
  ${GTEST_BOTH_LIBRARIES} 
  cpprestsdk::cpprest
)

add_test(JsonRestCppTests JsonRestCppTests)