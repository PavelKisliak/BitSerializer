add_subdirectory(bitserializer_core_tests)
add_subdirectory(bitserializer_json_restcpp_tests)
add_subdirectory(bitserializer_rapidjson_tests)

add_custom_target(tests
        DEPENDS
        bitserializer_core_tests
        bitserializer_json_restcpp_tests
        bitserializer_rapidjson_tests)