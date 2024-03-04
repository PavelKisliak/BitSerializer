add_subdirectory(bitserializer_core_tests)
add_subdirectory(bitserializer_common_tests)
add_subdirectory(bitserializer_conversions_tests)

if(BUILD_CPPRESTJSON_ARCHIVE)
    add_subdirectory(bitserializer_cpprest_json_tests)
endif()

if(BUILD_RAPIDJSON_ARCHIVE)
    add_subdirectory(bitserializer_rapidjson_tests)
endif()

if(BUILD_PUGIXML_ARCHIVE)
    add_subdirectory(bitserializer_pugixml_tests)
endif()

if(BUILD_RAPIDYAML_ARCHIVE)
    add_subdirectory(bitserializer_rapidyaml_tests)
endif()

if(BUILD_CSV_ARCHIVE)
    add_subdirectory(bitserializer_csv_tests)
endif()

if(BUILD_MSGPACK_ARCHIVE)
    add_subdirectory(bitserializer_msgpack_tests)
endif()