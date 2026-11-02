#!/usr/bin/env python3
"""Run archives_benchmarks several times and aggregate the JSON reports.

All unknown arguments are passed to the benchmark, for example:

    python run_repeated.py --repeat 5 --bench bs-json --stages save,load --stage-time 10

The script writes per-run reports to <output>/run-<i>/ and the aggregated report
to <output>/serialization_speed_report.json (same shape as the benchmark output).
"""

from __future__ import annotations

import argparse
import json
import shutil
import statistics
import subprocess
import sys
from pathlib import Path


# Same order as TestStage registration in benchmark_base.h.
STAGE_ORDER = {
    "Save to memory": 0,
    "Load from memory": 1,
    "Save to std::ostream": 2,
    "Load from std::istream": 3,
}


def stage_sort_key(stage: str) -> tuple[int, str]:
    return STAGE_ORDER.get(stage, len(STAGE_ORDER)), stage


# Same order as the benchmark registry in main.cpp (BitSerializer, then its
# competitor). Unknown libraries are sorted after the known ones by name.
LIBRARY_ORDER = {
    "BitSerializer-Json": 0,
    "BitSerializer-Jsonc": 1,
    "BitSerializer-Csv": 2,
    "BitSerializer-MsgPack": 3,
    "BitSerializer-RapidJson": 4,
    "RapidJson": 5,
    "BitSerializer-PugiXml": 6,
    "PugiXml": 7,
    "BitSerializer-RapidYaml": 8,
    "RapidYaml": 9,
    "NlohmannJson-MsgPack": 10,
}


def library_sort_key(library: str) -> tuple[int, str]:
    return LIBRARY_ORDER.get(library, len(LIBRARY_ORDER)), library


def find_executable(explicit: str | None) -> Path:
    if explicit:
        path = Path(explicit)
        if not path.is_file():
            sys.exit(f"Benchmark executable not found: {path}")
        return path

    names = ("archives_benchmarks.exe", "archives_benchmarks")
    for directory in (Path.cwd(), Path(__file__).resolve().parent):
        for name in names:
            path = directory / name
            if path.is_file():
                return path
    sys.exit(
        "Cannot find archives_benchmarks (looked in the current directory and "
        "next to this script). Pass --exe <path>."
    )


def run_benchmark(exe: Path, args: list[str]) -> None:
    command = [str(exe), *args]
    print(f"> {' '.join(command)}", flush=True)
    result = subprocess.run(command, check=False)
    if result.returncode != 0:
        sys.exit(f"Benchmark failed with exit code {result.returncode}")


def load_report(path: Path) -> dict:
    if not path.is_file():
        sys.exit(
            f"Report not found: {path}\n"
            "Make sure the benchmark is built with -DBUILD_JSON_ARCHIVE=ON "
            "(reports use the built-in JSON archive)."
        )
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def aggregate(values: list[float], mode: str) -> int:
    if mode == "best":
        result = max(values)
    elif mode == "median":
        result = statistics.median(values)
    elif mode == "mean":
        result = statistics.mean(values)
    else:
        raise ValueError(mode)
    return int(round(result))


def remove_run_dirs(base: Path) -> None:
    for stale in base.glob("run-*"):
        if stale.is_dir():
            shutil.rmtree(stale)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Run archives_benchmarks several times and aggregate the JSON reports. "
            "All unknown arguments are passed to the benchmark."
        )
    )
    parser.add_argument("--repeat", type=int, default=3, help="number of runs (default: 3)")
    parser.add_argument(
        "--aggregate",
        choices=("best", "median", "mean"),
        default="best",
        help="value written to the aggregated report (default: best)",
    )
    parser.add_argument("--exe", help="path to archives_benchmarks (default: current directory)")
    parser.add_argument(
        "--output",
        default="benchmark_results",
        help="base directory for reports (default: benchmark_results)",
    )
    parser.add_argument("--clean", action="store_true", help="remove per-run directories after aggregation")
    options, forwarded = parser.parse_known_args()

    if options.repeat < 1:
        parser.error("--repeat must be at least 1")
    if "--no-report" in forwarded:
        parser.error("--no-report is not supported: reports are needed for aggregation")

    exe = find_executable(options.exe)

    # `--list` does not run benchmarks, so just forward it and exit.
    if "--list" in forwarded:
        run_benchmark(exe, forwarded)
        return 0

    base = Path(options.output)
    base.mkdir(parents=True, exist_ok=True)
    remove_run_dirs(base)

    speed_runs: list[dict] = []
    size_runs: list[dict] = []
    for index in range(1, options.repeat + 1):
        run_dir = base / f"run-{index}"
        print(f"\n=== Run {index}/{options.repeat} -> {run_dir} ===", flush=True)
        run_benchmark(exe, [*forwarded, "--output", str(run_dir)])
        speed_runs.append(load_report(run_dir / "serialization_speed_report.json"))
        size_runs.append(load_report(run_dir / "serialization_output_size_report.json"))

    # Aggregate speed: same shape as the benchmark report.
    libraries = sorted({library for run in speed_runs for library in run}, key=library_sort_key)
    aggregated: dict[str, dict[str, int]] = {}
    for library in libraries:
        stages = sorted(
            {stage for run in speed_runs for stage in run.get(library, {})},
            key=stage_sort_key,
        )
        aggregated[library] = {}
        for stage in stages:
            values = [run[library][stage] for run in speed_runs if library in run and stage in run[library]]
            aggregated[library][stage] = aggregate(values, options.aggregate)

    speed_path = base / "serialization_speed_report.json"
    with speed_path.open("w", encoding="utf-8") as stream:
        json.dump(aggregated, stream, indent=2)
        stream.write("\n")

    # Output size is identical across runs; verify and keep one copy.
    first_size = size_runs[0]
    if any(run != first_size for run in size_runs[1:]):
        print("Warning: serialization output size differs between runs.", file=sys.stderr)
    ordered_size = {library: first_size[library] for library in sorted(first_size, key=library_sort_key)}
    size_path = base / "serialization_output_size_report.json"
    with size_path.open("w", encoding="utf-8") as stream:
        json.dump(ordered_size, stream, indent=2)
        stream.write("\n")

    # Console summary: best / median / mean / std for every library and stage.
    print("\nAggregated results (fields/ms):")
    header = f"{'library':<28} {'stage':<24} {'best':>12} {'median':>12} {'mean':>12} {'std':>10}"
    print(header)
    print("-" * len(header))
    for library in libraries:
        for stage in aggregated[library]:
            values = [run[library][stage] for run in speed_runs if library in run and stage in run[library]]
            best = max(values)
            median = statistics.median(values)
            mean = statistics.mean(values)
            std = statistics.pstdev(values) if len(values) > 1 else 0.0
            print(f"{library:<28} {stage:<24} {best:>12.0f} {median:>12.0f} {mean:>12.0f} {std:>10.0f}")

    if options.clean:
        remove_run_dirs(base)

    print(f"\nAggregated report: {speed_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
