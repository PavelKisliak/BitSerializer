project(serialize_to_stream)

add_executable(${PROJECT_NAME} serialize_to_stream.cpp)

find_package(RapidJSON CONFIG REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
