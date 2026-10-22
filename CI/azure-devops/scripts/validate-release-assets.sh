#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <packages-root> <branch-name>" >&2
    exit 2
fi

packages_root=$1
branch_name=$2

expected_suffixes=(
    "_x86_windows_msvc17_static.zip"
    "_x64_windows_msvc17_static.zip"
    "_x64_windows_msvc17_dynamic.zip"
    "_x64_linux_glibc228_gcc8_static.tar.gz"
    "_x64_macos1015_clang_static.tar.gz"
    "_arm64_linux_gcc13_static.tar.gz"
    "_arm64be_linux_gcc13_static.tar.gz"
)

required_entries=(
    "(^|/)include/bitserializer/bit_serializer\\.h$"
    "(^|/)share/bitserializer/bitserializerConfig\\.cmake$"
    "(^|/)share/doc/bitserializer/license\\.txt$"
)

if [ ! -d "$packages_root" ]; then
    echo "Packages directory does not exist: $packages_root" >&2
    exit 1
fi

mapfile -t archives < <(find "$packages_root" -type f \( -name '*.zip' -o -name '*.tar.gz' \) | sort)
expected_count=${#expected_suffixes[@]}

if [ "${#archives[@]}" -ne "$expected_count" ]; then
    echo "Expected $expected_count release archives, found ${#archives[@]}." >&2
    printf '  %s\n' "${archives[@]}" >&2
    exit 1
fi

for suffix in "${expected_suffixes[@]}"; do
    expected_name="bitserializer_${branch_name}${suffix}"
    mapfile -t matches < <(find "$packages_root" -type f -name "$expected_name" | sort)
    if [ "${#matches[@]}" -ne 1 ]; then
        echo "Expected exactly one '$expected_name', found ${#matches[@]}." >&2
        exit 1
    fi
done

for archive in "${archives[@]}"; do
    case "$archive" in
        *.zip)
            if ! unzip -tq "$archive" >/dev/null; then
                echo "Invalid zip archive: $archive" >&2
                exit 1
            fi
            if ! entries=$(unzip -Z1 "$archive"); then
                echo "Failed to list zip archive: $archive" >&2
                exit 1
            fi
            ;;
        *.tar.gz)
            if ! tar -tzf "$archive" >/dev/null; then
                echo "Invalid tar.gz archive: $archive" >&2
                exit 1
            fi
            if ! entries=$(tar -tzf "$archive"); then
                echo "Failed to list tar.gz archive: $archive" >&2
                exit 1
            fi
            ;;
        *)
            echo "Unsupported archive type: $archive" >&2
            exit 1
            ;;
    esac

    for required_entry in "${required_entries[@]}"; do
        if ! grep -qE "$required_entry" <<< "$entries"; then
            echo "Missing required entry in '$archive': $required_entry" >&2
            exit 1
        fi
    done
done

echo "Validated $expected_count release archives."
