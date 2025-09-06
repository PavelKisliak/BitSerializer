project(csv_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Target configuration
add_executable(${PROJECT_NAME}
    $<TARGET_OBJECTS:csv_impl>
    csv_reader_fixture.h
    csv_reader_tests.cpp
    csv_writer_fixture.h
    csv_writer_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST CsvTests
    PROPERTIES LABELS "unit_tests"
)
