cmake_minimum_required(VERSION 3.12)

project(hello_world)

add_executable(${PROJECT_NAME} hello_world.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::bitserializer-cpprestjson
)
