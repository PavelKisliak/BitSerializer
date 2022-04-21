cmake_minimum_required(VERSION 3.10)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
)

add_subdirectory(bitserializer_core_tests)
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


if(BUILD_CPPRESTJSON_ARCHIVE AND BUILD_RAPIDJSON_ARCHIVE AND BUILD_PUGIXML_ARCHIVE AND BUILD_RAPIDYAML_ARCHIVE AND BUILD_CSV_ARCHIVE)
   add_subdirectory(performance_tests)
else()
   message(WARNING "Disable build performance tests in connection with disabled build archives")
endif()
