#!/usr/bin/env python3
"""Verify that MODULE.bazel matches the CMake project version.

MODULE.bazel is generated from cmake/MODULE.bazel.in during CMake configure.
This check catches a stale committed copy (for example, after bumping the
version in CMakeLists.txt for a release).
"""
from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]


def main() -> int:
    cmake_text = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    module_text = (ROOT / "MODULE.bazel").read_text(encoding="utf-8")

    cmake_match = re.search(
        r"project\(\s*bitserializer.*?\bVERSION\s+([0-9]+\.[0-9]+\.[0-9]+)",
        cmake_text,
        re.DOTALL,
    )
    module_match = re.search(
        r'^[ \t]*version[ \t]*=[ \t]*"([^"]+)"',
        module_text,
        re.MULTILINE,
    )

    if not cmake_match or not module_match:
        print("Could not determine the version from CMakeLists.txt or MODULE.bazel", file=sys.stderr)
        return 2

    cmake_version = cmake_match.group(1)
    module_version = module_match.group(1)
    if cmake_version != module_version:
        print(
            f"MODULE.bazel version ({module_version}) does not match CMakeLists.txt ({cmake_version}); "
            "run CMake to regenerate MODULE.bazel and commit it",
            file=sys.stderr,
        )
        return 1

    print(f"MODULE.bazel version matches CMakeLists.txt ({cmake_version})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
