project(performance_tests)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

find_package(cpprestsdk CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
  main.cpp
  base_test_models.h
  rapid_json_performance_test.h
  cpprest_json_performance_test.h
  pugixml_performance_test.h
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
    cpprestsdk::cpprest
    pugixml
)
