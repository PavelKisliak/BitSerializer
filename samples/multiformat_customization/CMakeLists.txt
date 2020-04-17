cmake_minimum_required(VERSION 3.12)

project(multiformat_customization)

add_executable(${PROJECT_NAME} multiformat_customization.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::bitserializer-rapidjson
  BitSerializer::bitserializer-pugixml
)
