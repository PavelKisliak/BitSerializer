project(rapidjson_archive_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Target configuration
add_executable(${PROJECT_NAME}
    rapidjson_archive_tests.cpp
    rapidjson_std_containers_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::rapidjson-archive
    GTest::Main
    testing_tools
)

# This test target intentionally exercises the deprecated archive; disable only
# the archive's deprecation marker so compiler and clang-tidy diagnostics stay quiet.
target_compile_definitions(${PROJECT_NAME} PRIVATE BITSERIALIZER_DISABLE_RAPIDJSON_DEPRECATION)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST RapidJsonArchiveTests
    PROPERTIES LABELS "integration_tests"
)
