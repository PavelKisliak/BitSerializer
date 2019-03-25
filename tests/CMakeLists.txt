include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(performance_tests)
add_subdirectory(bitserializer_core_tests)
add_subdirectory(bitserializer_cpprest_json_tests)
add_subdirectory(bitserializer_rapidjson_tests)
add_subdirectory(bitserializer_pugixml_tests)

add_custom_target(tests
        DEPENDS
		performance_tests
        bitserializer_core_tests
        bitserializer_cpprest_json_tests
        bitserializer_rapidjson_tests
		bitserializer_pugixml_tests)