project(performance_tests)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

find_package(cpprestsdk CONFIG REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
  main.cpp
  base_test_models.h
  rapid_json_performance_test.h
  cpprest_json_performance_test.h
  pugixml_performance_test.h
)

find_path(CPP_REST_SDK_DIR cpprest/json.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${CPP_REST_SDK_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${RAPIDJSON_INCLUDE_DIRS})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # since clang, link with -lc++experimental.
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            c++experimental
            cpprestsdk::cpprest
            cpprestsdk::cpprestsdk_zlib_internal
            cpprestsdk::cpprestsdk_boost_internal
            cpprestsdk::cpprestsdk_openssl_internal
            ${Boost_SYSTEM_LIBRARY}
            pugixml
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # since gcc, link with -lstdc++fs.
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            stdc++fs
            cpprestsdk::cpprest
            cpprestsdk::cpprestsdk_zlib_internal
            cpprestsdk::cpprestsdk_boost_internal
            cpprestsdk::cpprestsdk_openssl_internal
            ${Boost_SYSTEM_LIBRARY}
            pugixml
    )
else()
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            cpprestsdk::cpprest
            cpprestsdk::cpprestsdk_zlib_internal
            cpprestsdk::cpprestsdk_boost_internal
            cpprestsdk::cpprestsdk_openssl_internal
            ${Boost_SYSTEM_LIBRARY}
            pugixml
    )
endif()
