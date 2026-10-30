#################################################################################
# Copyright (C) 2020-2026 by Pavel Kisliak
# This file is part of BitSerializer library, licensed under the MIT license.
#################################################################################
#
# Rewrites the source paths in a gcovr Cobertura report to absolute paths.
#
# gcovr >= 6 stores the <source> directory relative to the current working
# directory (e.g. ".."), which the Azure DevOps HTML report generator resolves
# against the wrong base and therefore renders "No assemblies have been covered".
# Making both <source> and the relative <class> filenames absolute lets the
# report generator locate the sources at publish time.
#
# Usage:
#   cmake -DCOVERAGE_FILE=<file.xml> -DSOURCE_ROOT=<dir> -P fix_cobertura_paths.cmake
#
#################################################################################
if(NOT DEFINED COVERAGE_FILE OR NOT DEFINED SOURCE_ROOT)
    message(FATAL_ERROR "COVERAGE_FILE and SOURCE_ROOT must be defined")
endif()

file(READ "${COVERAGE_FILE}" _coverage_content)

# Normalize the directory separators and prefix for the XML (Unix-style paths).
file(TO_CMAKE_PATH "${SOURCE_ROOT}" _source_root)
string(REGEX REPLACE "/$" "" _source_root "${_source_root}")

string(REGEX REPLACE
    "<source>[^<]*</source>"
    "<source>${_source_root}</source>"
    _coverage_content "${_coverage_content}")

# Only relative filenames are prefixed (absolute ones start with '/').
string(REGEX REPLACE
    "filename=\"([^/\"][^\"]*)\""
    "filename=\"${_source_root}/\\1\""
    _coverage_content "${_coverage_content}")

file(WRITE "${COVERAGE_FILE}" "${_coverage_content}")
