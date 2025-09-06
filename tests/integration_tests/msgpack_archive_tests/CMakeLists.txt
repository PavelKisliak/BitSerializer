project(msgpack_archive_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Target configuration
add_executable(${PROJECT_NAME}
    msgpack_archive_tests.cpp
    msgpack_std_containers_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::msgpack-archive
    GTest::Main
    testing_tools
)

# Copy shared libraries (used by acceptance tests)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:BitSerializer::msgpack-archive>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying BitSerializer::msgpack-archive runtime dependency to build directory"
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST MsgPackArchiveTests
    PROPERTIES LABELS "integration_tests"
)
