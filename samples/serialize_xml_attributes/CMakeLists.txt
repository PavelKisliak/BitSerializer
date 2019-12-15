project(serialize_xml_attributes)

add_executable(${PROJECT_NAME} serialize_xml_attributes.cpp)

find_package(pugixml CONFIG REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${PUGIXML_INCLUDE_DIRS})
