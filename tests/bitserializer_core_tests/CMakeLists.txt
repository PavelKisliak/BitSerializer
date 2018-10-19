cmake_minimum_required(VERSION 3.10)

project(test_bitserializer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

enable_testing()
find_package(GTest REQUIRED)

include_directories(
  ${GTEST_INCLUDE_DIRS}
)

add_executable(BitSerializerCoreTests 
  serialization_archive_traits_tests.cpp
  serialization_containers_tests.cpp
  serialization_object_traits_tests.cpp
  serialization_stl_types_tests.cpp
  string_conversion_tests.cpp
  validators_tests.cpp
 )

target_link_libraries(
  BitSerializerCoreTests
  ${GTEST_BOTH_LIBRARIES} 
  cpprestsdk::cpprest
)

add_test(BitSerializerCoreTests BitSerializerCoreTests)