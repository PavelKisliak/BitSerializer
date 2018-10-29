project(samples)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

find_package(cpprestsdk REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

find_path(CPP_REST_SDK_DIR cpprest/json.h)

include_directories(${CPP_REST_SDK_DIR})

add_subdirectory(hello_world)
add_subdirectory(validation)

add_custom_target(samples DEPENDS hello_world validation)