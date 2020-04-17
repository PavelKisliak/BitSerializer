cmake_minimum_required(VERSION 3.12)

project(serialize_to_stream)

add_executable(${PROJECT_NAME} serialize_to_stream.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::bitserializer-rapidjson
)
