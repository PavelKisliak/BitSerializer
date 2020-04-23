#################################################################################
# Copyright (C) 2020 by Pavel Kisliak                                           #
# This file is part of BitSerializer library, licensed under the MIT license.   #
#################################################################################
cmake_minimum_required(VERSION 3.10)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

project(bitserializer-cpprestjson
  VERSION 0.9
  DESCRIPTION "C++ 17 library for serialization, module for support JSON (implementation based on the CppRestSDK library)"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#################################################################################
# Target configuration
#################################################################################
set (BITSERIALIZER_NAMESPACE "BitSerializer")

add_library(${PROJECT_NAME} INTERFACE)
add_library(${BITSERIALIZER_NAMESPACE}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

# Link dependencies
find_package(cpprestsdk CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} INTERFACE
  BitSerializer::bitserializer
  cpprestsdk::cpprest
)

#################################################################################
# Installation
#################################################################################
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}_Targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY ExactVersion)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

install(EXPORT ${PROJECT_NAME}_Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${BITSERIALIZER_NAMESPACE}::
  DESTINATION ${INSTALL_CONFIGDIR})

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${INSTALL_CONFIGDIR})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cpprest_json_archive.h DESTINATION include/bitserializer_cpprest_json)
