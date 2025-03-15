add_subdirectory(common_tests)
add_subdirectory(convert_tests)
add_subdirectory(core_tests)
add_subdirectory(std_types_tests)

if(BUILD_CSV_ARCHIVE)
    add_subdirectory(csv_tests)
endif()

if(BUILD_MSGPACK_ARCHIVE)
    add_subdirectory(msgpack_tests)
endif()
