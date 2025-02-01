project(hello_world)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} hello_world.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::rapidjson-archive
)
