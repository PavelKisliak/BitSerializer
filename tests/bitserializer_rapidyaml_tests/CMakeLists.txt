project(bitserializer_rapidyaml_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}
  rapidyaml_archive_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::rapidyaml-archive
  GTest::GTest
  GTest::Main
  testing_tools
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerRapidYamlTests)
