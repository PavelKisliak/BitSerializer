cmake_minimum_required(VERSION 3.10)

project(test_bitserializer)

set(CMAKE_CXX_STANDARD 17)

enable_testing()
find_package(GTest REQUIRED)

include_directories(
  ${GTEST_INCLUDE_DIRS}
)

add_executable(BitSerializerTests 
  json_restcpp_archive_tests.cpp
  serialization_archive_traits_tests.cpp
  serialization_containers_tests.cpp
  serialization_object_traits_tests.cpp
  serialization_stl_types_tests.cpp
  string_conversion_tests.cpp)

target_link_libraries(
  BitSerializerTests
  ${GTEST_BOTH_LIBRARIES} 
  cpprestsdk::cpprest
)

add_test(Lib_Tests BitSerializerTests)