cmake_minimum_required(VERSION 3.12)

project(serialize_xml_attributes)

add_executable(${PROJECT_NAME} serialize_xml_attributes.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::bitserializer-pugixml
)
