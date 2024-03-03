project(testing_tools)

find_package(GTest REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>)

target_link_libraries(${PROJECT_NAME} INTERFACE
    GTest::GTest
)
