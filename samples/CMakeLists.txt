cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

add_executable(hello_world hello_world.cc)
target_link_libraries(hello_world PRIVATE cpprestsdk::cpprest)