project(rapidjson_archive_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Project sources
add_executable(${PROJECT_NAME}
    rapidjson_archive_tests.cpp
)

# Target configuration
target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::rapidjson-archive
    GTest::GTest
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST RapidJsonArchiveTests
    PROPERTIES LABELS "integration_tests"
)
