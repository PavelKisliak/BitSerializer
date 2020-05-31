cmake_minimum_required(VERSION 3.10)

project(serialize_xml_attributes)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} serialize_xml_attributes.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::pugixml-archive
)
