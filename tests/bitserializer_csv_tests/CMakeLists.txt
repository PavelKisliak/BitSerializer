project(bitserializer_csv_tests)

find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}
  csv_reader_fixture.h
  csv_reader_tests.cpp
  csv_writer_fixture.h
  csv_writer_tests.cpp
  csv_archive_fixture.h
  csv_archive_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::csv-archive
  GTest::GTest
  GTest::Main
  testing_tools
)

gtest_discover_tests(${PROJECT_NAME} TEST_LIST BitSerializerCsvTests)
