project(json_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Target configuration
add_executable(${PROJECT_NAME}
    $<TARGET_OBJECTS:json_impl> $<TARGET_OBJECTS:common_impl>
    json_reader_fixture.h
    json_reader_tests.cpp
    json_writer_fixture.h
    json_writer_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST JsonTests
    PROPERTIES LABELS "unit_tests"
)
