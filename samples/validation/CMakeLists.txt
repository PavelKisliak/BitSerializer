project(validation)

add_executable(${PROJECT_NAME} validation.cpp)

find_package(RapidJSON CONFIG REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
