project(common)

add_library(${PROJECT_NAME} STATIC
	binary_stream_reader.cpp
)

#target_link_libraries(${PROJECT_NAME} INTERFACE
#    ${BITSERIALIZER_CORE_NAME}
#)

target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>)
