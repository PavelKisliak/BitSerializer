cmake_minimum_required(VERSION 3.10)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
)

add_subdirectory(performance_tests)
add_subdirectory(bitserializer_core_tests)
add_subdirectory(bitserializer_cpprest_json_tests)
add_subdirectory(bitserializer_rapidjson_tests)
add_subdirectory(bitserializer_pugixml_tests)
if(BUILD_RAPIDYAML_ARCHIVE)
    add_subdirectory(bitserializer_rapidyaml_tests)
endif(BUILD_RAPIDYAML_ARCHIVE)
