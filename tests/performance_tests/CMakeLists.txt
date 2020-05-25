cmake_minimum_required(VERSION 3.10)

project(performance_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/${CMAKE_SYSTEM_NAME}/${CONFIG})

add_executable(${PROJECT_NAME}
    main.cpp
    base_test_models.h
    rapid_json_performance_test.h
    cpprest_json_performance_test.h
    pugixml_performance_test.h
    rapid_yaml_perfomance_test.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::bitserializer-cpprestjson
    BitSerializer::bitserializer-rapidjson
    BitSerializer::bitserializer-pugixml
    BitSerializer::bitserializer-rapidyaml
)
