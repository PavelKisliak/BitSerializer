project(archives_benchmarks)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    main.cpp
    benchmark_base.h
    rapid_json_benchmark.h
    pugixml_benchmark.h
    rapid_yaml_benchmark.h
    csv_benchmark.h
    msgpack_benchmark.h
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<BOOL:${BUILD_RAPIDJSON_ARCHIVE}>:"RAPIDJSON_BENCHMARK">
    $<$<BOOL:${BUILD_PUGIXML_ARCHIVE}>:"PUGIXML_BENCHMARK">
    $<$<BOOL:${BUILD_RAPIDYAML_ARCHIVE}>:"RAPIDYAMLN_BENCHMARK">
    $<$<BOOL:${BUILD_CSV_ARCHIVE}>:"CSV_BENCHMARK">
    $<$<BOOL:${BUILD_MSGPACK_ARCHIVE}>:"MSGPACK_BENCHMARK">
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    testing_tools
    $<$<BOOL:${BUILD_RAPIDJSON_ARCHIVE}>:BitSerializer::rapidjson-archive>
    $<$<BOOL:${BUILD_PUGIXML_ARCHIVE}>:BitSerializer::pugixml-archive>
    $<$<BOOL:${BUILD_RAPIDYAML_ARCHIVE}>:BitSerializer::rapidyaml-archive>
    $<$<BOOL:${BUILD_CSV_ARCHIVE}>:BitSerializer::csv-archive>
    $<$<BOOL:${BUILD_MSGPACK_ARCHIVE}>:BitSerializer::msgpack-archive>
)
