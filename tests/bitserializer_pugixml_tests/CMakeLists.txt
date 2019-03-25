cmake_minimum_required(VERSION 3.10)

project(bitserializer_pugixml_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package(pugixml CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
  pugixml_archive_tests.cpp
)

target_link_libraries(
  ${PROJECT_NAME}
  ${GTEST_BOTH_LIBRARIES}
  pugixml
)

add_test(PugiXmlTests ${PROJECT_NAME})