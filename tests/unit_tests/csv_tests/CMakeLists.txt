project(csv_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Private library sources (not available via DLL API)
set(PRIVATE_LIB_SRC
    "${CMAKE_SOURCE_DIR}/src/csv/csv_archive.cpp"
    "${CMAKE_SOURCE_DIR}/src/csv/csv_readers.h"
    "${CMAKE_SOURCE_DIR}/src/csv/csv_readers.cpp"
    "${CMAKE_SOURCE_DIR}/src/csv/csv_writers.h"
    "${CMAKE_SOURCE_DIR}/src/csv/csv_writers.cpp"
)

# Target configuration
add_executable(${PROJECT_NAME}
    ${PRIVATE_LIB_SRC}
    csv_reader_fixture.h
    csv_reader_tests.cpp
    csv_writer_fixture.h
    csv_writer_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::GTest
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST CsvTests
    PROPERTIES LABELS "unit_tests"
)
