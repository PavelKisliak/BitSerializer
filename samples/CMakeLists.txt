if(BUILD_RAPIDJSON_ARCHIVE)
    add_subdirectory(hello_world)
    add_subdirectory(validation)
    add_subdirectory(serialize_third_party_class)
    add_subdirectory(serialize_custom_string)
    add_subdirectory(serialize_custom_array)
    add_subdirectory(serialize_to_stream)
endif()

if(BUILD_PUGIXML_ARCHIVE)
    add_subdirectory(serialize_xml_attributes)
endif()

if(BUILD_RAPIDJSON_ARCHIVE AND BUILD_PUGIXML_ARCHIVE)
    add_subdirectory(multiformat_customization)
endif()

if(BUILD_RAPIDYAML_ARCHIVE)
    add_subdirectory(serialize_map_to_yaml)
endif()

if(BUILD_CSV_ARCHIVE)
    add_subdirectory(serialize_to_csv)
endif()

if(BUILD_RAPIDJSON_ARCHIVE AND BUILD_MSGPACK_ARCHIVE)
    add_subdirectory(msgpack_vs_json)
endif()

if(BUILD_MSGPACK_ARCHIVE)
    add_subdirectory(versioning)
endif()

add_subdirectory(string_conversions)
