project(bench_harness)

# Shared infrastructure for benchmarks: CPU helpers (priority, affinity, warm-up)
# and the intra-binary A/B microbenchmark harness. Header-mostly; only
# perf_utils.cpp is compiled. Kept separate from `testing_tools` so benchmarks do
# not depend on the test framework (GTest). Static (not OBJECT) so linking works
# with the project's minimum CMake 3.10 — object files of OBJECT libraries are
# propagated to consumers only since CMake 3.12.
add_library(${PROJECT_NAME} STATIC
    perf_utils.cpp
    perf_utils.h
    bench_harness.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)
