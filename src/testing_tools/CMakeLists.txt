project(testing_tools)

find_package(GTest REQUIRED)

add_library(${PROJECT_NAME} OBJECT
    "perf_utils.cpp"
)
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>)

target_link_libraries(${PROJECT_NAME} PUBLIC
    GTest::GTest
)
