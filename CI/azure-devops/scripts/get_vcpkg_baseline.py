#!/usr/bin/env python3
"""Print the pinned VCPKG commit from the project's vcpkg.json manifest."""

import json
import sys
from pathlib import Path


def main():
    manifest = Path(__file__).resolve().parents[3] / "vcpkg.json"
    try:
        with manifest.open(encoding="utf-8") as stream:
            config = json.load(stream)
    except (OSError, ValueError) as error:
        print("Cannot read {}: {}".format(manifest, error), file=sys.stderr)
        return 1

    baseline = config.get("builtin-baseline")
    if not baseline:
        print("'builtin-baseline' is missing in {}".format(manifest), file=sys.stderr)
        return 1

    print(baseline)
    return 0


if __name__ == "__main__":
    sys.exit(main())
