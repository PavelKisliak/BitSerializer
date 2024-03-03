project(serialize_xml_attributes)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} serialize_xml_attributes.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::pugixml-archive
)
