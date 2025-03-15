add_subdirectory(convert_api_tests)

if(BUILD_RAPIDJSON_ARCHIVE)
    add_subdirectory(rapidjson_archive_tests)
endif()

if(BUILD_PUGIXML_ARCHIVE)
    add_subdirectory(pugixml_archive_tests)
endif()

if(BUILD_RAPIDYAML_ARCHIVE)
    add_subdirectory(rapidyaml_archive_tests)
endif()

if(BUILD_CSV_ARCHIVE)
    add_subdirectory(csv_archive_tests)
endif()

if(BUILD_MSGPACK_ARCHIVE)
    add_subdirectory(msgpack_archive_tests)
endif()
