add_executable(hello_world hello_world.cpp)

target_link_libraries(hello_world
        PRIVATE
        cpprestsdk::cpprest
        cpprestsdk::cpprestsdk_zlib_internal
        cpprestsdk::cpprestsdk_boost_internal
        cpprestsdk::cpprestsdk_openssl_internal
        ${Boost_SYSTEM_LIBRARY}
)