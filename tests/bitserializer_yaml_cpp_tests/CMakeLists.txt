cmake_minimum_required(VERSION 3.10)

project(bitserializer_yaml_cpp_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package(yaml-cpp CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
  yaml_cpp_archive_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
  GTest::GTest
  GTest::Main
  yaml-cpp
)

add_test(YamlCppTests ${PROJECT_NAME})