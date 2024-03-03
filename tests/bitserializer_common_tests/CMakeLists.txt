project(bitserializer_common_tests)

find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}
    binary_stream_reader_tests.cpp
    binary_stream_reader_fixture.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    common
    GTest::Main
    GTest::GTest
    testing_tools
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerAllCoreTests)
