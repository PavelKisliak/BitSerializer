cmake_minimum_required(VERSION 3.12)

project(validation)

add_executable(${PROJECT_NAME} validation.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::bitserializer-rapidjson
)
