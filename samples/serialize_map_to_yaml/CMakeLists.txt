project(serialize_map_to_yaml)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} serialize_map_to_yaml.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::rapidyaml-archive
  BitSerializer::rapidjson-archive
)
