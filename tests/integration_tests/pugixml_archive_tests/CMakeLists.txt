project(pugixml_archive_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Target configuration
add_executable(${PROJECT_NAME}
    pugixml_archive_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::pugixml-archive
    GTest::GTest
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST PugiXmlArchiveTests
    PROPERTIES LABELS "integration_tests"
)
