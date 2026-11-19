# Toolchain for the `x64-linux-libcxx` vcpkg triplet. Every dependency is built with Clang
# against libc++. The project itself must use the same compiler and standard library (see
# the Ubuntu_Clang_libcxx_static CI job). Otherwise test binaries mix libc++ and libstdc++
# objects, and linking fails.
#
# `-stdlib=libc++` goes into the *_FLAGS_INIT variables, which apply to the first configure
# of every port (each vcpkg port has its own build tree) and leave explicit port flags
# untouched.
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_FLAGS_INIT "-stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-stdlib=libc++")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-stdlib=libc++")
