cmake_minimum_required(VERSION 3.8)

project(bitserializer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(cpprestsdk REQUIRED)

find_path(CPP_REST_SDK_DIR cpprest/json.h)
include_directories(
  ${CPP_REST_SDK_DIR}
  include
)

add_library(bitserializer INTERFACE)

target_include_directories(bitserializer 
  INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

 add_subdirectory(tests)
 add_subdirectory(samples)
