cmake_minimum_required(VERSION 3.10)

project(bitserializer_core_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/${CMAKE_SYSTEM_NAME}/${CONFIG})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
    string_conversion_tests.cpp
    utf8_encoding_tests.cpp
    serialization_archive_traits_tests.cpp
    serialization_base_types_tests.cpp
    serialization_object_traits_tests.cpp
    serialization_std_containers_tests.cpp
    serialization_std_types_tests.cpp
    validators_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::core
    GTest::Main
    GTest::GTest
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerAllCoreTests)
