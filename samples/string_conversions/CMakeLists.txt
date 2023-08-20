project(string_conversions)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} string_conversions.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::core
)
