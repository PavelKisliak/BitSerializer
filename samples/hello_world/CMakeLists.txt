project(hello_world)

add_executable(${PROJECT_NAME} hello_world.cpp)

find_package(cpprestsdk CONFIG REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_path(CPP_REST_SDK_DIR cpprest/json.h)
include_directories(${CPP_REST_SDK_DIR})

target_link_libraries(${PROJECT_NAME}
   PRIVATE
        cpprestsdk::cpprest
        cpprestsdk::cpprestsdk_zlib_internal
        cpprestsdk::cpprestsdk_boost_internal
        cpprestsdk::cpprestsdk_openssl_internal
        ${Boost_SYSTEM_LIBRARY}
)