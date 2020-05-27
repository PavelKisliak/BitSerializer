set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/${CMAKE_SYSTEM_NAME}/${CONFIG})

add_subdirectory(hello_world)
add_subdirectory(validation)
add_subdirectory(serialize_third_party_class)
add_subdirectory(multiformat_customization)
add_subdirectory(serialize_xml_attributes)
add_subdirectory(serialize_to_stream)
