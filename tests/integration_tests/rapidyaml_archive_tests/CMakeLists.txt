project(rapidyaml_archive_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Target configuration
add_executable(${PROJECT_NAME}
    rapidyaml_archive_tests.cpp
    rapidyaml_std_containers_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::rapidyaml-archive
    GTest::GTest
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST RapidYamlArchiveTests
    PROPERTIES LABELS "integration_tests"
)
