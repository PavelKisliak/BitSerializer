project(bitserializer_json_restcpp_tests)

find_package(GTest REQUIRED)
find_package(cpprestsdk CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
  cpprest_json_archive_tests.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    BitSerializer::cpprestjson-archive
    GTest::GTest
    GTest::Main
    testing_tools
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerCppRestJsonTests)
