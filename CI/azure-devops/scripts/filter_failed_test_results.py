#!/usr/bin/env python3
"""Filter a CTest dashboard report down to failed tests.

Reads ``Test.xml`` files produced by ``ctest -T test`` and writes a smaller
report containing only tests whose status is not ``passed``. This keeps
per-platform failure diagnostics without uploading the full test set for every
configuration.
"""

import argparse
import glob
import os
import sys
import xml.etree.ElementTree as ET


def _remove(path):
    try:
        os.remove(path)
    except OSError:
        pass


def _full_name(test):
    name = test.get("FullName")
    if name:
        return name
    return test.findtext("FullName") or test.findtext("Name")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", default="build")
    parser.add_argument("--output", default=None)
    args = parser.parse_args()

    output = args.output or os.path.join(args.build_dir, "Testing", "failed", "Test.xml")
    output_abs = os.path.abspath(output)

    pattern = os.path.join(args.build_dir, "Testing", "**", "Test.xml")
    files = [f for f in glob.glob(pattern, recursive=True) if os.path.abspath(f) != output_abs]

    if not files:
        print("No Test.xml found, nothing to filter")
        _remove(output)
        return

    failed_tests = []
    base_root = None

    for path in files:
        try:
            root = ET.parse(path).getroot()
        except ET.ParseError as error:
            print("Warning: cannot parse {}: {}".format(path, error), file=sys.stderr)
            continue

        testing = root.find("Testing")
        if testing is None:
            continue

        if base_root is None:
            base_root = root

        for test in testing.findall("Test"):
            if test.get("Status") != "passed":
                failed_tests.append(test)

    if base_root is None:
        print("No valid CTest report found, nothing to filter")
        _remove(output)
        return

    if not failed_tests:
        print("No failed tests, nothing to publish")
        _remove(output)
        return

    out_root = ET.Element(base_root.tag, dict(base_root.attrib))
    out_testing = ET.SubElement(out_root, "Testing")

    base_testing = base_root.find("Testing")
    for child in base_testing:
        if child.tag not in ("Test", "TestList"):
            out_testing.append(child)

    failed_names = {_full_name(test) for test in failed_tests}
    base_list = base_testing.find("TestList")
    if base_list is not None:
        out_list = ET.SubElement(out_testing, "TestList")
        for name_node in base_list.findall("Test"):
            if name_node.text in failed_names:
                out_list.append(name_node)

    for test in failed_tests:
        out_testing.append(test)

    os.makedirs(os.path.dirname(output), exist_ok=True)
    ET.ElementTree(out_root).write(output, encoding="UTF-8", xml_declaration=True)
    print("Wrote {} failed test(s) to {}".format(len(failed_tests), output))


if __name__ == "__main__":
    main()
