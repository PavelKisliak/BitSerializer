project(core_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Project sources
add_executable(${PROJECT_NAME}
    archive_base_tests.cpp
    attribute_value_tests.cpp
    bin_timestamp_tests.cpp
    key_value_tests.cpp
    serialization_archive_traits_tests.cpp
    serialization_base_types_tests.cpp
    serialization_context_tests.cpp
    serialization_object_traits_tests.cpp
    validators_tests.cpp
    refine_tests.cpp
)

# Target configuration
target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::core
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST CoreTests
    PROPERTIES LABELS "unit_tests"
)
