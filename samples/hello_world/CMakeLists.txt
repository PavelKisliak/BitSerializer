project(hello_world)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} hello_world.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::cpprestjson-archive
)
