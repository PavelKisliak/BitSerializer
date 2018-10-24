set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

add_subdirectory(hello_world)
add_subdirectory(validation)

add_custom_target(samples DEPENDS hello_world validation)