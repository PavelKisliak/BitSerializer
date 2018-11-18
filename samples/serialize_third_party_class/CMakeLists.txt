project(serialize_third_party_class)

add_executable(${PROJECT_NAME} serialize_third_party_class.cpp)

find_package(RapidJSON CONFIG REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
