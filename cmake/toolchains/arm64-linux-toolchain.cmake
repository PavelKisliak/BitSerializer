set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm64)

if(NOT DEFINED ARM64_GNU_TOOLCHAIN_ROOT)
  set(ARM64_GNU_TOOLCHAIN_ROOT "/usr/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu")
endif()

set(tools "${ARM64_GNU_TOOLCHAIN_ROOT}/bin")
set(CMAKE_C_COMPILER "${ARM64_GNU_TOOLCHAIN_ROOT}/bin/aarch64-none-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "${ARM64_GNU_TOOLCHAIN_ROOT}/bin/aarch64-none-linux-gnu-g++")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_CROSSCOMPILING_EMULATOR "qemu-aarch64;-L;${ARM64_GNU_TOOLCHAIN_ROOT}/aarch64-none-linux-gnu/libc/")
