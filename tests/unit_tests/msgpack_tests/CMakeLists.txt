project(msgpack_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Project sources
add_executable(${PROJECT_NAME}
    msgpack_reader_fixture.h
    msgpack_reader_tests.cpp
    msgpack_writer_fixture.h
    msgpack_writer_tests.cpp
)

# Target configuration
target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::msgpack-archive
    GTest::GTest
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST MsgPackTests
    PROPERTIES LABELS "unit_tests"
)
