project(std_types_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Project sources
add_executable(${PROJECT_NAME}
    chrono_tests.cpp
    ctime_tests.cpp
    filesystem_tests.cpp
    std_containers_tests.cpp
    std_pmr_containers_tests.cpp
    std_types_tests.cpp
)

# Target configuration
target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::core
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST StdTypesTests
    PROPERTIES LABELS "unit_tests"
)
