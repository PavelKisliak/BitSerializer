project(msgpack_vs_json)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} msgpack_vs_json.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::rapidjson-archive
  BitSerializer::msgpack-archive
)
