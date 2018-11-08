project(bitserializer_json_restcpp_tests)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

find_package(GTest REQUIRED)

find_package(cpprestsdk REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

find_path(CPP_REST_SDK_DIR cpprest/json.h)

include_directories(${GTEST_INCLUDE_DIRS} ${CPP_REST_SDK_DIR})

enable_testing()

add_executable(${PROJECT_NAME}
  cpprest_json_archive_tests.cpp
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # since clang, link with -lc++experimental.
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            c++experimental
            GTest::GTest GTest::Main
            cpprestsdk::cpprest
            cpprestsdk::cpprestsdk_zlib_internal
            cpprestsdk::cpprestsdk_boost_internal
            cpprestsdk::cpprestsdk_openssl_internal
            ${Boost_SYSTEM_LIBRARY}
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # since gcc, link with -lstdc++fs.
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            stdc++fs
            GTest::GTest GTest::Main
            cpprestsdk::cpprest
            cpprestsdk::cpprestsdk_zlib_internal
            cpprestsdk::cpprestsdk_boost_internal
            cpprestsdk::cpprestsdk_openssl_internal
            ${Boost_SYSTEM_LIBRARY}
    )
else()
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            GTest::GTest GTest::Main
            cpprestsdk::cpprest
            cpprestsdk::cpprestsdk_zlib_internal
            cpprestsdk::cpprestsdk_boost_internal
            cpprestsdk::cpprestsdk_openssl_internal
            ${Boost_SYSTEM_LIBRARY}
    )
endif()

add_test(RestCppJsonTests ${PROJECT_NAME})