cmake_minimum_required(VERSION 3.12)

project(serialize_third_party_class)

add_executable(${PROJECT_NAME} serialize_third_party_class.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::bitserializer-rapidjson
)
