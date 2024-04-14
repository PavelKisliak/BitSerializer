project(bitserializer_core_tests)

find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}
    archive_base_tests.cpp
    serialization_archive_traits_tests.cpp
    serialization_base_types_tests.cpp
    serialization_context_tests.cpp
    serialization_object_traits_tests.cpp
    serialization_std_containers_tests.cpp
    serialization_std_types_tests.cpp
    serialization_std_chrono_tests.cpp
    serialization_std_filesystem_tests.cpp
    serialization_ctime_tests.cpp
    validators_tests.cpp
    key_value_tests.cpp
    attribute_value_tests.cpp
    bin_timestamp_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::core
    GTest::Main
    GTest::GTest
    testing_tools
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerAllCoreTests)
