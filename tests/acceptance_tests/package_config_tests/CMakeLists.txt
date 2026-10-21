cmake_minimum_required(VERSION 3.21)
project(package_config_tests NONE)

# The prebuilt package must be consumable without installing any third-party
# dependencies when only the built-in archives are used.
find_package(bitserializer CONFIG REQUIRED)

foreach(_target IN ITEMS
        BitSerializer::core
        BitSerializer::json-archive
        BitSerializer::csv-archive
        BitSerializer::msgpack-archive)
    if(NOT TARGET ${_target})
        message(FATAL_ERROR "The prebuilt package is missing target: ${_target}")
    endif()
endforeach()
