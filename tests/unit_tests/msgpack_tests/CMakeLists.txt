project(msgpack_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Target configuration
add_executable(${PROJECT_NAME}
    $<TARGET_OBJECTS:msgpack_impl> $<TARGET_OBJECTS:common_impl>
    msgpack_reader_fixture.h
    msgpack_reader_tests.cpp
    msgpack_writer_fixture.h
    msgpack_writer_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::GTest
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST MsgPackTests
    PROPERTIES LABELS "unit_tests"
)
