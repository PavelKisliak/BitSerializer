project(bitserializer_rapidjson_tests)

find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}
  rapidjson_archive_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::rapidjson-archive
  GTest::GTest
  GTest::Main
  testing_tools
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerRapidJsonTests)
