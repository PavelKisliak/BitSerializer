project(bitserializer_core_tests)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
  serialization_archive_traits_tests.cpp
  serialization_base_types_tests.cpp
  serialization_containers_tests.cpp
  serialization_object_traits_tests.cpp
  serialization_stl_types_tests.cpp
  string_conversion_tests.cpp
  validators_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::Main
    GTest::GTest)

add_test(BitSerializerAllCoreTests ${PROJECT_NAME})