project(serialize_to_stream)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} serialize_to_stream.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::rapidjson-archive
)
