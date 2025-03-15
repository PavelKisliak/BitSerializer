project(common_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Project sources
add_executable(${PROJECT_NAME}
    binary_stream_reader_tests.cpp
    binary_stream_reader_fixture.h
)

# Target configuration
target_link_libraries(${PROJECT_NAME} PRIVATE
    common
    GTest::Main
    GTest::GTest
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST CommonTests
    PROPERTIES LABELS "unit_tests"
)
