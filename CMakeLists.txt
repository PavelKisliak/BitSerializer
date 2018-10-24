cmake_minimum_required(VERSION 3.10)

project(bitserializer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(cpprestsdk REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

find_path(CPP_REST_SDK_DIR cpprest/json.h)
include_directories(
  ${CPP_REST_SDK_DIR}
  core
  archives
)

add_library(bitserializer INTERFACE)

target_include_directories(bitserializer 
  INTERFACE
    $<INSTALL_INTERFACE:core>
    $<INSTALL_INTERFACE:archives>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/core>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/archives>
)

if(MSVC)
    target_compile_options(bitserializer INTERFACE /W4)
else()
    target_compile_options(bitserializer
            INTERFACE
            -Wall
            -Wextra # reasonable and standard
            -Wnon-virtual-dtor # warn the user if a class with virtual functions has a
                               # non-virtual destructor.

            # track down memory errors
            -Wold-style-cast # warn for c-style casts
            -Wcast-align # warn for potential performance problem casts
            -Wunused # warn on anything being unused
            -Woverloaded-virtual # warn if you overload but not override a virtual

            # function
            -Wpedantic # warn if non-standard C++ is used
            -Wconversion # warn on type conversions that may lose data
            -Wsign-conversion # warn on sign conversions

            # do not exist
            -Wduplicated-cond # warn if if / else chain has duplicated conditions
            -Wduplicated-branches # warn if if / else branches have duplicated code
            -Wlogical-op # warn about logical operations being where bitwise is used
    )
endif()

#add_subdirectory(tests)
add_subdirectory(samples)
