project(bitserializer_core_tests)

find_package(GTest REQUIRED)

set(TEST_CORE_SOURCE_FILES
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

if(NOT APPLE)
    list(APPEND TEST_CORE_SOURCE_FILES serialization_std_pmr_containers_tests.cpp)
endif()

add_executable(${PROJECT_NAME} ${TEST_CORE_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::core
    GTest::Main
    GTest::GTest
    testing_tools
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerAllCoreTests)
