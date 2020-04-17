cmake_minimum_required(VERSION 3.12)

project(bitserializer_pugixml_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>_${VCPKG_TARGET_TRIPLET})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
  pugixml_archive_tests.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  BitSerializer::bitserializer-pugixml
  GTest::GTest
  GTest::Main
)

enable_testing()
add_test(PugiXmlTests ${PROJECT_NAME})
