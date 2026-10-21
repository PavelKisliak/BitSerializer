project(json_archive_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Target configuration
add_executable(${PROJECT_NAME}
    json_archive_tests.cpp
    jsonc_archive_tests.cpp
    json_std_containers_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::json-archive
    GTest::Main
    testing_tools
)

# Copy shared libraries (used by acceptance tests)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:BitSerializer::json-archive>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying BitSerializer::json-archive runtime dependency to build directory"
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST JsonArchiveTests
    PROPERTIES LABELS "integration_tests"
)
