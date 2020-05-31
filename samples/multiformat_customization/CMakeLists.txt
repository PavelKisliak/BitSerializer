cmake_minimum_required(VERSION 3.10)

project(multiformat_customization)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} multiformat_customization.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::rapidjson-archive
  BitSerializer::pugixml-archive
)
