add_executable(validation validation.cpp)

find_package(RapidJSON CONFIG REQUIRED)
target_include_directories(validation PRIVATE ${RAPIDJSON_INCLUDE_DIRS})

target_link_libraries(validation
   PRIVATE
        cpprestsdk::cpprest
        cpprestsdk::cpprestsdk_zlib_internal
        cpprestsdk::cpprestsdk_boost_internal
        cpprestsdk::cpprestsdk_openssl_internal
        ${Boost_SYSTEM_LIBRARY}
)