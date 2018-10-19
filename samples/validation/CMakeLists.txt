cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

add_executable(validation validation.cpp)
target_link_libraries(validation PRIVATE cpprestsdk::cpprest)