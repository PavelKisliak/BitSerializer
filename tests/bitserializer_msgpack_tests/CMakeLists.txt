project(bitserializer_msgpack_tests)

find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}
    msgpack_archive_tests.cpp
    msgpack_reader_fixture.h
    msgpack_reader_tests.cpp
    msgpack_writer_fixture.h
    msgpack_writer_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::msgpack-archive
    GTest::GTest
    GTest::Main
    testing_tools
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerMsgPackTests)
