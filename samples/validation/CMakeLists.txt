add_executable(validation validation.cpp)

target_link_libraries(validation
   PRIVATE
        cpprestsdk::cpprest
        cpprestsdk::cpprestsdk_zlib_internal
        cpprestsdk::cpprestsdk_boost_internal
        cpprestsdk::cpprestsdk_openssl_internal
        ${Boost_SYSTEM_LIBRARY}
)