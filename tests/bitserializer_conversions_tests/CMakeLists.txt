project(bitserializer_conversions_tests)

find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}
    convert_api_tests.cpp
    utf8_encoding_tests.cpp
    utf16le_encoding_tests.cpp
    utf16be_encoding_tests.cpp
    utf32le_encoding_tests.cpp
    utf32be_encoding_tests.cpp
    utf_transcoding_tests.cpp
    detect_encodings_fixture.h
    detect_encodings_tests.cpp
    encoded_stream_reader_fixture.h
    encoded_stream_reader_tests.cpp
    encoded_stream_writer_fixture.h
    encoded_stream_writer_tests.cpp
    convert_fundamentals_tests.cpp
    convert_enums_tests.cpp
    convert_traits_tests.cpp
    convert_classes_tests.cpp
    convert_filesystem_tests.cpp
    convert_chrono_tests.cpp
    convert_custom_string_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::core
    GTest::Main
    GTest::GTest
    testing_tools
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerAllCoreTests)
