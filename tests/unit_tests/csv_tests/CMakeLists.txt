project(csv_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Project sources
add_executable(${PROJECT_NAME}
  csv_reader_fixture.h
  csv_reader_tests.cpp
  csv_writer_fixture.h
  csv_writer_tests.cpp
)

# Target configuration
target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::csv-archive
  GTest::GTest
  GTest::Main
  testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST CsvTests
    PROPERTIES LABELS "unit_tests"
)
