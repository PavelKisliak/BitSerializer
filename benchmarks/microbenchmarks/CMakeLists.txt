project(microbenchmarks)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Hot-function A/B example: alternative implementations of the JSON string
# writer's escaping routine. Uses the archive's internal header, so it is built
# only with the JSON archive and adds the library source/include roots.
if(BUILD_JSON_ARCHIVE)
    add_executable(microbenchmark
        microbenchmark.cpp
    )
    target_include_directories(microbenchmark PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
    )
    target_link_libraries(microbenchmark PRIVATE bench_harness)
endif()

# Archive Save/Load A/B example: compares two instantiations of the JSON archive
# (strict JSON vs JSONC) on a small model. It is a template for swapping one
# archive component without forking the whole archive; prints a hint when the JSON
# archive is not enabled, so the target always builds.
add_executable(archive_ab_benchmarks
    archive_ab.cpp
    benchmark_models.h
)

target_compile_definitions(archive_ab_benchmarks PRIVATE
    $<$<BOOL:${BUILD_JSON_ARCHIVE}>:"JSON_BENCHMARK">
)

target_link_libraries(archive_ab_benchmarks PRIVATE
    bench_harness
    $<$<BOOL:${BUILD_JSON_ARCHIVE}>:BitSerializer::json-archive>
)
