# x64 Linux triplet that builds every dependency against libc++.
#
# Used by the Ubuntu_Clang_libcxx CI job: the library and its dependencies (GoogleTest,
# PugiXml, RapidYAML, ...) must share one standard library, otherwise linking a test binary
# that uses libc++ with libraries built against libstdc++ fails or mixes two standard libraries
# in one process. The toolchain file selects Clang and switches it to libc++.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_BUILD_TYPE release)

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../../toolchains/clang-libcxx-linux-toolchain.cmake")
