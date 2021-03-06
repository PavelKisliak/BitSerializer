project(archives_benchmarks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    main.cpp
    benchmark_base.h
    rapid_json_benchmark.h
    cpprest_json_benchmark.h
    pugixml_benchmark.h
    rapid_yaml_benchmark.h
    csv_benchmark.h
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<BOOL:${BUILD_CPPRESTJSON_ARCHIVE}>:"CPPRESTJSON_BENCHMARK">
    $<$<BOOL:${BUILD_RAPIDJSON_ARCHIVE}>:"RAPIDJSON_BENCHMARK">
    $<$<BOOL:${BUILD_PUGIXML_ARCHIVE}>:"PUGIXML_BENCHMARK">
    $<$<BOOL:${BUILD_RAPIDYAML_ARCHIVE}>:"RAPIDYAMLN_BENCHMARK">
    $<$<BOOL:${BUILD_CSV_ARCHIVE}>:"CSV_BENCHMARK">
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    testing_tools
    $<$<BOOL:${BUILD_CPPRESTJSON_ARCHIVE}>:BitSerializer::cpprestjson-archive>
    $<$<BOOL:${BUILD_RAPIDJSON_ARCHIVE}>:BitSerializer::rapidjson-archive>
    $<$<BOOL:${BUILD_PUGIXML_ARCHIVE}>:BitSerializer::pugixml-archive>
    $<$<BOOL:${BUILD_RAPIDYAML_ARCHIVE}>:BitSerializer::rapidyaml-archive>
    $<$<BOOL:${BUILD_CSV_ARCHIVE}>:BitSerializer::csv-archive>
)
