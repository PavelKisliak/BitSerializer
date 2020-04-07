cmake_minimum_required(VERSION 3.10)

project(bitserializer_json_restcpp_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

find_package(GTest REQUIRED)
find_package(cpprestsdk CONFIG REQUIRED)

enable_testing()

add_executable(${PROJECT_NAME}
  cpprest_json_archive_tests.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
    GTest::GTest GTest::Main
    cpprestsdk::cpprest
)

add_test(RestCppJsonTests ${PROJECT_NAME})