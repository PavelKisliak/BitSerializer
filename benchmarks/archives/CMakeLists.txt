project(performance_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    main.cpp
    base_test_models.h
    base_perf_test.h
    rapid_json_performance_test.h
    cpprest_json_performance_test.h
    pugixml_performance_test.h
    rapid_yaml_performance_test.h
    csv_performance_test.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    testing_tools
    BitSerializer::cpprestjson-archive
    BitSerializer::rapidjson-archive
    BitSerializer::pugixml-archive
    BitSerializer::rapidyaml-archive
    BitSerializer::csv-archive
)
