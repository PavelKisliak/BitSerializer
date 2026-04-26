project(json_archive_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Target configuration
add_executable(${PROJECT_NAME}
    json_archive_tests.cpp
    json_std_containers_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::json-archive
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST JsonArchiveTests
    PROPERTIES LABELS "integration_tests"
)
