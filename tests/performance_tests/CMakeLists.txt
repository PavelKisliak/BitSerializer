cmake_minimum_required(VERSION 3.10)

project(performance_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/${CMAKE_SYSTEM_NAME}/${CONFIG})

add_executable(${PROJECT_NAME}
    main.cpp
    base_test_models.h
    archive_base_perf_test.h
    rapid_json_performance_test.h
    cpprest_json_performance_test.h
    pugixml_performance_test.h
    rapid_yaml_performance_test.h
    csv_performance_test.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::cpprestjson-archive
    BitSerializer::rapidjson-archive
    BitSerializer::pugixml-archive
    BitSerializer::rapidyaml-archive
    BitSerializer::csv-archive
)
