project(msgpack_tests)

# Third party libraries
find_package(GTest REQUIRED)

# Private library sources (not available via DLL API)
set(PRIVATE_LIB_SRC
    "${CMAKE_SOURCE_DIR}/src/common/binary_stream_reader.cpp"
    "${CMAKE_SOURCE_DIR}/src/msgpack/msgpack_archive.cpp"
    "${CMAKE_SOURCE_DIR}/src/msgpack/msgpack_readers.h"
    "${CMAKE_SOURCE_DIR}/src/msgpack/msgpack_readers.cpp"
    "${CMAKE_SOURCE_DIR}/src/msgpack/msgpack_writers.h"
    "${CMAKE_SOURCE_DIR}/src/msgpack/msgpack_writers.cpp"
)

# Target configuration
add_executable(${PROJECT_NAME}
    ${PRIVATE_LIB_SRC}
    msgpack_reader_fixture.h
    msgpack_reader_tests.cpp
    msgpack_writer_fixture.h
    msgpack_writer_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::GTest
    GTest::Main
    testing_tools
)

# Tests configuration
gtest_discover_tests(${PROJECT_NAME} TEST_LIST MsgPackTests
    PROPERTIES LABELS "unit_tests"
)
