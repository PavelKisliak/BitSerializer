#!/usr/bin/env bash
set -euo pipefail

# AlmaLinux 8 is used as the oldest supported glibc baseline (2.28).
# Its AppStream provides GCC 8.5 and CMake 3.26, but not Ninja.
dnf install -y \
    bzip2 \
    ca-certificates \
    cmake \
    curl \
    gcc \
    gcc-c++ \
    git \
    gzip \
    make \
    patch \
    perl \
    perl-IPC-Cmd \
    pkgconfig \
    python3 \
    tar \
    unzip \
    which \
    xz \
    zip

curl -fsSL https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-linux.zip -o /tmp/ninja.zip
unzip -q -o /tmp/ninja.zip -d /usr/local/bin
chmod +x /usr/local/bin/ninja
rm -f /tmp/ninja.zip

cmake_is_supported() {
    [ "$(printf '%s\n' 3.21 "$1" | sort -V | head -n 1)" = "3.21" ]
}

cmake_version=$(cmake --version | awk 'NR == 1 { print $3 }')
if ! cmake_is_supported "$cmake_version"; then
    echo "CMake $cmake_version is too old, installing CMake 3.31.6"
    curl -fsSL https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-linux-x86_64.tar.gz -o /tmp/cmake.tar.gz
    tar -xzf /tmp/cmake.tar.gz -C /opt
    ln -sf /opt/cmake-3.31.6-linux-x86_64/bin/cmake /usr/local/bin/cmake
    ln -sf /opt/cmake-3.31.6-linux-x86_64/bin/ctest /usr/local/bin/ctest
    rm -f /tmp/cmake.tar.gz
    hash -r
fi

cmake_version=$(cmake --version | awk 'NR == 1 { print $3 }')
if ! cmake_is_supported "$cmake_version"; then
    echo "CMake 3.21 or newer is required, found $cmake_version" >&2
    exit 1
fi

echo "--- Toolchain versions ---"
echo "glibc: $(ldd --version 2>&1 | sed -n '1p')"
echo "GCC:   $(gcc --version | sed -n '1p')"
echo "G++:   $(g++ --version | sed -n '1p')"
echo "CMake: $(cmake --version | sed -n '1p')"
echo "Ninja: $(ninja --version)"
