project(convert_api_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Target configuration
add_executable(${PROJECT_NAME}
    convert_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::core
    GTest::Main
    GTest::GTest
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST ConvertApiTests
    PROPERTIES LABELS "integration_tests"
)
