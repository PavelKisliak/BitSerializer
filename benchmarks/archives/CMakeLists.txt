project(archives_benchmarks)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    main.cpp
    test_model.h
    benchmark_base.h
    competitors/bitserializer_benchmark.h
    competitors/rapidjson_benchmark.h
    competitors/pugixml_benchmark.h
    competitors/rapidyaml_benchmark.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<BOOL:${BUILD_RAPIDJSON_ARCHIVE}>:"RAPIDJSON_BENCHMARK">
    $<$<BOOL:${BUILD_PUGIXML_ARCHIVE}>:"PUGIXML_BENCHMARK">
    $<$<BOOL:${BUILD_RAPIDYAML_ARCHIVE}>:"RAPIDYAML_BENCHMARK">
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
