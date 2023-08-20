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

target_link_libraries(${PROJECT_NAME} PRIVATE
    testing_tools
    BitSerializer::cpprestjson-archive
    BitSerializer::rapidjson-archive
    BitSerializer::pugixml-archive
    BitSerializer::rapidyaml-archive
    BitSerializer::csv-archive
)
