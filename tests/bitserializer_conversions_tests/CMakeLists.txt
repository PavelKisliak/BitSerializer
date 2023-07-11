cmake_minimum_required(VERSION 3.10)

project(bitserializer_conversions_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
    convert_api_tests.cpp
    utf8_encoding_tests.cpp
    utf16_encoding_tests.cpp
    utf32_encoding_tests.cpp
    detect_encodings_fixture.h
    detect_encodings_tests.cpp
    encoded_stream_reader_fixture.h
    encoded_stream_reader_tests.cpp
    convert_fundamentals_tests.cpp
    convert_traits_tests.cpp
    convert_classes_tests.cpp
    convert_transcoding_tests.cpp
    convert_std_tests.cpp
    convert_chrono_tests.cpp
    convert_custom_string_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::core
    GTest::Main
    GTest::GTest
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerAllCoreTests)
