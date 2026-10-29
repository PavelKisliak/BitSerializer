project(archives_benchmarks)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    test_model.h
    benchmark_base.h
    benchmark_base.cpp
    competitors/bitserializer_benchmark.h
    competitors/rapidjson_benchmark.h
    competitors/pugixml_benchmark.h
    competitors/rapidyaml_benchmark.h
    competitors/nlohmannjson_benchmark.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<BOOL:${BUILD_RAPIDJSON_ARCHIVE}>:"RAPIDJSON_BENCHMARK">
    $<$<BOOL:${BUILD_PUGIXML_ARCHIVE}>:"PUGIXML_BENCHMARK">
    $<$<BOOL:${BUILD_RAPIDYAML_ARCHIVE}>:"RAPIDYAML_BENCHMARK">
    $<$<BOOL:${BUILD_CSV_ARCHIVE}>:"CSV_BENCHMARK">
    $<$<BOOL:${BUILD_MSGPACK_ARCHIVE}>:"MSGPACK_BENCHMARK">
    $<$<BOOL:${BUILD_JSON_ARCHIVE}>:"JSON_BENCHMARK">
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    bench_harness
    testing_tools
    $<$<BOOL:${BUILD_RAPIDJSON_ARCHIVE}>:BitSerializer::rapidjson-archive>
    $<$<BOOL:${BUILD_PUGIXML_ARCHIVE}>:BitSerializer::pugixml-archive>
    $<$<BOOL:${BUILD_RAPIDYAML_ARCHIVE}>:BitSerializer::rapidyaml-archive>
    $<$<BOOL:${BUILD_CSV_ARCHIVE}>:BitSerializer::csv-archive>
    $<$<BOOL:${BUILD_MSGPACK_ARCHIVE}>:BitSerializer::msgpack-archive>
    $<$<BOOL:${BUILD_JSON_ARCHIVE}>:BitSerializer::json-archive>
)

# Stabilize results against build-to-build code-layout shifts.
#
# The benchmark reports `min` time over millions of single-call samples, an
# extreme-value statistic that is very sensitive to the alignment of the hot
# functions: a benign code change anywhere in the linked image can shift them by
# a cache line and move the reported speed by ~5-10% (a "layout lottery") even
# when the algorithm is unchanged. Forcing function alignment to 64 bytes (one
# cache line) freezes that alignment and collapses the spread to ~1-2%, so small
# optimizations can be measured reliably.
#
# The hot serialization code lives in the archive libraries, so both the benchmark
# target and the archive libraries must be aligned: aligning only the benchmark's
# own translation units still leaves a ~7-13% artifact on stages whose library
# code merely moved. This applies to benchmark builds only; the shipped library is
# built without the flag. Function alignment has no ABI impact, so this stays
# compatible with any DLL/static build.
#
# MSVC (cl.exe) has no equivalent knob (no per-function alignment attribute or
# flag; /ALIGN aligns sections, not functions), so its runs remain indicative
# only - use Clang or GCC for authoritative A/B comparisons.
option(BENCHMARK_ALIGN_FUNCTIONS "Freeze function alignment to stabilize benchmark results (Clang/GCC)" ON)

set(BENCHMARK_ALIGN_OPTIONS "")
if(BENCHMARK_ALIGN_FUNCTIONS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(BENCHMARK_ALIGN_OPTIONS -mllvm -align-all-functions=6)
elseif(BENCHMARK_ALIGN_FUNCTIONS AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(BENCHMARK_ALIGN_OPTIONS -falign-functions=64 -falign-loops=32)
endif()

if(BENCHMARK_ALIGN_OPTIONS)
    target_compile_options(${PROJECT_NAME} PRIVATE ${BENCHMARK_ALIGN_OPTIONS})
    foreach(_benchmarkArchiveLib common_impl csv_impl msgpack_impl json_impl)
        if(TARGET ${_benchmarkArchiveLib})
            target_compile_options(${_benchmarkArchiveLib} PRIVATE ${BENCHMARK_ALIGN_OPTIONS})
        endif()
    endforeach()
endif()
