"""
********************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
********************************************************************************
"""
import os
import sys

from find_affected_files import object_file_to_source


def find_rebuilt_sources(build_dir, objects_list_file):
    """
    Map object files rebuilt during the current build to their source files.

    :param build_dir: Path to the CMake build directory (all restored/compiled object files live here).
    :param objects_list_file: File with the object file paths, one per line (e.g. from `find -newer`).
    :return: Sorted list of existing source files.
    """
    build_prefix = os.path.normpath(build_dir).replace('\\', '/').rstrip('/') + '/'
    sources = set()

    with open(objects_list_file, 'r') as f:
        for line in f:
            object_path = line.strip().replace('\\', '/')
            if not object_path:
                continue
            if object_path.startswith(build_prefix):
                object_path = object_path[len(build_prefix):]

            source_path = object_file_to_source(object_path)
            if os.path.isfile(source_path):
                sources.add(source_path)

    return sorted(sources)


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: find_rebuilt_sources.py <build_dir> <objects_list_file> <output_file>")
        sys.exit(1)

    build_dir = sys.argv[1]
    objects_list_file = sys.argv[2]
    output_file = sys.argv[3]

    rebuilt_sources = find_rebuilt_sources(build_dir, objects_list_file)
    with open(output_file, 'w') as f:
        f.write('\n'.join(rebuilt_sources))
    print(f"Rebuilt source files ({len(rebuilt_sources)}) saved to: {output_file}")
