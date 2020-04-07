project(hello_world)

add_executable(${PROJECT_NAME} hello_world.cpp)

find_package(cpprestsdk CONFIG REQUIRED)
include_directories(${CPP_REST_SDK_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE cpprestsdk::cpprest)
