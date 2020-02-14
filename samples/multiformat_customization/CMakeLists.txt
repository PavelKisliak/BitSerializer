project(multiformat_customization)

add_executable(${PROJECT_NAME} multiformat_customization.cpp)

find_package(RapidJSON CONFIG REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
find_package(pugixml CONFIG REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${PUGIXML_INCLUDE_DIRS})