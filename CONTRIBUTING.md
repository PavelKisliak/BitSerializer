# Contributing to BitSerializer

Thank you for your interest in contributing to BitSerializer! This guide will help you set up the development environment and understand the project workflow.

## How to Contribute

The most valuable contribution is often a clear description of a problem or an idea. You do not need to write code to contribute: a well-scoped feature request or bug report is enough for the maintainer to evaluate and, if accepted, implement the change directly.

- **Ideas and feature requests** — describe the problem, who runs into it, the expected behavior, and the constraints or alternatives you considered. A precise request is the fastest path to a change; use the [feature request template](https://github.com/PavelKisliak/BitSerializer/issues/new?template=feature_request.yml).
- **Bug reports** — include a minimal reproduction, expected vs actual behavior, your BitSerializer version (see `include/bitserializer/config.h`), compiler/OS/version, and architecture. Use the [bug report template](https://github.com/PavelKisliak/BitSerializer/issues/new?template=bug_report.yml).
- **Documentation and tests** — typo fixes, clearer examples, missing explanations, and additional coverage are always valuable. Read [Testing](docs/development/testing.md) before adding or changing tests.
- **Performance improvements** — welcome; if you already have measurements, include before/after results (see [Performance Benchmarks](docs/development/benchmarks.md)). If you only have a workload or use case, describe it in an issue.
- **Code contributions** — welcome as well. Small fixes, documentation, and tests can go straight to a PR. For non-trivial changes (new features, public API, architecture), open an issue or discussion first and mention whether you plan to implement it; this avoids duplicate work and lets us agree on the approach.

> [!NOTE]
> For **significant changes** — new features, architectural refactoring, changes to the public API — please start by creating an **ADR (Architecture Decision Record)** as a GitHub issue or discussion before submitting a PR. This helps us align on the approach before you invest time in implementation.

### Architecture Decision Records (ADR)

An ADR documents a significant architectural decision and its context. When proposing a major change, include the following:

- **Title** — short descriptive name for the decision (e.g., "Switch from DOM to SAX parser for JSON archive")
- **Context** — what problem or requirement motivates this change? What constraints exist?
- **Decision** — what approach are you proposing? Describe it clearly.
- **Alternatives considered** — what other approaches were evaluated, and why were they rejected?
- **Consequences** — what are the trade-offs? What becomes easier or harder? Are there breaking changes?

ADRs don't need to be formal or lengthy — a clear, concise explanation is enough. The goal is to have a shared understanding before diving into implementation.

### On adding new archive formats

The library aims to support the most widely used serialization formats, not every format that exists. Each new archive adds maintenance burden, and the author's capacity is limited. If you'd like to propose a new format, please open a discussion first — explain the use case, the format's popularity and adoption, and why existing archives don't cover the need.

## Development guide

Detailed development documentation lives in [`docs/development/`](docs/development/README.md):

- [Architecture](docs/development/architecture.md) — archive pattern, scope contract, loading semantics.
- [Testing](docs/development/testing.md) — test targets, helpers, unit vs integration tests.
- [Code style](docs/development/code-style.md) — naming, formatting, documentation and license headers.
- [Benchmarks](docs/development/benchmarks.md) — round-trip and microbenchmark workflow.
- [Workflow](docs/development/workflow.md) — commit convention, PR workflow, CI matrix.

Build instructions are split by audience: this file covers the human setup (Visual Studio, Developer Command Prompt, VCPKG), while [`AGENTS.md`](AGENTS.md) covers non-interactive/agent builds (`vcvarsall` capture and exact commands). Keep both in sync where they describe the same option.

## Prerequisites

### All platforms

- **C++17 compiler** (or newer)
- **CMake** >= 3.10
- **Ninja** build system
- **VCPKG** package manager

### Windows

- **Visual Studio 2022 or 2026** (Community, Professional, or Enterprise)
  - Install the "Desktop development with C++" workload
  - This includes MSVC, CMake, and Ninja
  - Optionally, install the **C++ Clang Compiler for Windows** component for Clang-cl support
  - Install the **Test Adapter for Google Test** extension (by Microsoft) — enables running GTest tests directly from the VS Test Explorer

### Linux / macOS

- **Linux:** GCC (8+) or Clang (8+)
- **macOS:** Xcode Command Line Tools (provides Clang)
- Install Ninja:
  ```bash
  # Linux (Debian/Ubuntu)
  sudo apt-get install build-essential ninja-build cmake

  # macOS
  brew install ninja cmake
  ```

---

## Setting up VCPKG

VCPKG manages third-party dependencies (RapidJSON, PugiXml, RapidYAML, Google Test).

### Install VCPKG

```bash
# Clone vcpkg (anywhere on your system)
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Bootstrap
# Windows:
.\bootstrap-vcpkg.bat
# Linux/macOS:
./bootstrap-vcpkg.sh
```

### Set environment variable

Set `VCPKG_ROOT` to point to your vcpkg installation:

**Windows (PowerShell):**
```powershell
[System.Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\path\to\vcpkg", "User")
```
> [!TIP]
> This sets the variable permanently (stored in the registry). Restart your terminal or IDE for the change to take effect in already running processes.

**Linux/macOS:**
```bash
# Add to ~/.bashrc or ~/.zshrc:
export VCPKG_ROOT=/path/to/vcpkg
```

---

## Building the project

> [!NOTE]
> This section describes the human setup path: Visual Studio or a pre-configured VS Developer Command Prompt, where the MSVC environment is already available. For non-interactive shells and AI agents (where the environment must be captured from `vcvarsall.bat`), see [AGENTS.md](AGENTS.md#windows-build-from-powershell). The two paths intentionally differ; keep the shared CMake options in sync.

### Option 1: Visual Studio (recommended for Windows)

The easiest way on Windows is to use Visual Studio's built-in CMake support:

1. Open Visual Studio (2022 or 2026)
2. Select **File > Open > Folder...** and choose the BitSerializer repository root
3. Visual Studio will automatically detect `CMakeSettings.json` and configure the project
4. Select a configuration from the dropdown (e.g., `Win-MSVC-x64-Debug`)
5. Build with **F7** (or **Ctrl+Shift+B**, or **Build > Build All**)
6. Run tests with **Test > Run All Tests**

> [!TIP]
> VS stores build output in `%LOCALAPPDATA%\CMakeBuilds\<GUID>\build\<Config>\` (not in the repository). When running tests or binaries from the command line, use the path shown in the VS Output window, or build from the command line instead (see Options 2/3 below) to get the standard `build/` directory.

Available configurations in `CMakeSettings.json`:

| Configuration | Compiler | Platform | C++ Std | Linkage |
|---|---|---|---|---|
| `Win-MSVC-x86-Debug/Release` | MSVC | x86 | 17 | static |
| `Win-MSVC-x64-Debug/Release` | MSVC | x64 | 17 | static |
| `Win-MSVC-x64-Debug+Analysis` | MSVC | x64 | 17 | static + clang-tidy |
| `Win-MSVC-x64-Debug-Dynamic` | MSVC | x64 | 17 | shared |
| `Win-MSVC-x64-CXX20-Debug/Release` | MSVC | x64 | 20 | static |
| `Win-MSVC-x64-CXX23-Debug/Release` | MSVC | x64 | 23 | static |
| `Win-Clang-x64-Debug/Release` | Clang-cl | x64 | 17 | static |
| `Linux-GCC8-x64-Debug/Release` | GCC 8 | x64 | 17 | static |
| `Linux-GCC12-x64-CXX20-Debug/Release` | GCC 12 | x64 | 20 | static |
| `Linux-Clang8-x64-Debug/Release` | Clang 8 | x64 | 17 | static |
| `Linux-Clang14-x64-CXX20-Debug/Release` | Clang 14 | x64 | 20 | static |
| `Linux-GCC-ARM32-Debug/Release` | GCC (cross) | ARM32 | 17 | static |
| `Linux-GCC-ARM64-CXX20-Debug/Release` | GCC (cross) | ARM64 | 20 | static |
| `Linux-GCC-ARM64BE-Debug/Release` | GCC (cross) | ARM64BE | 17 | static |

> [!NOTE]
> Linux and ARM configurations require a remote machine connected via SSH. This can be a VM in VirtualBox, WSL, or any Linux machine accessible from your Windows host. Configure the remote connection in **Tools > Options > Cross Platform > Connection Manager** in Visual Studio.
>
> See also: [Linux development with C++ in Visual Studio](https://learn.microsoft.com/en-us/cpp/linux/?view=msvc-170) — setup guide, remote connection, and CMake Linux projects.

> [!NOTE]
> The project uses `CMakeSettings.json` rather than `CMakePresets.json`. This is intentional — CMake Presets require CMake 3.19+, but the project supports GCC 8 and Clang 8, which ship with older CMake versions (3.10–3.16).

> [!TIP]
> If VS doesn't pick up `CMakeSettings.json` when opening the project, check that **Tools > Options > CMake > General > Always use CMake Presets** is **disabled**. When enabled, VS ignores `CMakeSettings.json` and may auto-create a default `CMakePresets.json`.

### Option 2: Command line (Windows)

Use the **pre-configured VS developer environment** — open one of these from the Start Menu:

```
Visual Studio (2022/2026) > Visual Studio Tools > VC >
    x64 Native Tools Command Prompt for VS    (build for x64)
    x64_x86 Cross Tools Command Prompt for VS (build for x86 from x64 host)
```

These prompts have MSVC, CMake, and Ninja already in PATH. Then:

```cmd
cd <path-to-bitserializer>

:: CMake configure
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_TESTS=ON -DBUILD_SAMPLES=ON -DBUILD_BENCHMARKS=ON ^
    -DBUILD_JSON_ARCHIVE=ON -DBUILD_CSV_ARCHIVE=ON -DBUILD_MSGPACK_ARCHIVE=ON ^
    -DBUILD_RAPIDJSON_ARCHIVE=ON -DBUILD_PUGIXML_ARCHIVE=ON -DBUILD_RAPIDYAML_ARCHIVE=ON

:: Build
ninja -C build

:: Run tests (-j = concurrent tests; lower it if memory is tight)
ctest --test-dir build -T test --output-on-failure -j %NUMBER_OF_PROCESSORS%
```

**Using Clang-cl instead of MSVC:**

From the same VS developer command prompt, add `-DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl` to the CMake command:

```cmd
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl ^
    -DBUILD_TESTS=ON -DBUILD_SAMPLES=ON -DBUILD_BENCHMARKS=ON ^
    -DBUILD_JSON_ARCHIVE=ON -DBUILD_CSV_ARCHIVE=ON -DBUILD_MSGPACK_ARCHIVE=ON ^
    -DBUILD_RAPIDJSON_ARCHIVE=ON -DBUILD_PUGIXML_ARCHIVE=ON -DBUILD_RAPIDYAML_ARCHIVE=ON
```

### Option 3: Command line (Linux / macOS)

```bash
cd <path-to-bitserializer>

# Configure
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON -DBUILD_SAMPLES=ON -DBUILD_BENCHMARKS=ON \
    -DBUILD_JSON_ARCHIVE=ON -DBUILD_CSV_ARCHIVE=ON -DBUILD_MSGPACK_ARCHIVE=ON \
    -DBUILD_RAPIDJSON_ARCHIVE=ON -DBUILD_PUGIXML_ARCHIVE=ON -DBUILD_RAPIDYAML_ARCHIVE=ON

# Build
ninja -C build

# Run tests (-j = concurrent tests; lower it if memory is tight)
ctest --test-dir build -T test --output-on-failure -j "$(getconf _NPROCESSORS_ONLN)"
```

**Using a specific compiler:**

```bash
# GCC 14
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=gcc-14 -DCMAKE_CXX_COMPILER=g++-14 \
    ...

# Clang 15
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang-15 -DCMAKE_CXX_COMPILER=clang++-15 \
    ...
```

### Option 4: Bazel (optional)

Bazel builds the library and the integration tests from source and fetches all dependencies from the Bazel Central Registry, so VCPKG is not required. Use [Bazelisk](https://github.com/bazelbuild/bazelisk) so the version pinned in `.bazelversion` is used.

```bash
bazel build //...
bazel test //...
```

Public targets are `//:core`, `//:json_archive`, `//:csv_archive`, `//:msgpack_archive`, `//:pugixml_archive` and `//:rapidyaml_archive` (the deprecated RapidJSON archive is not exposed). On Windows, set `BAZEL_SH` to the Git Bash executable before running `bazel test`.

`MODULE.bazel` is generated from `cmake/MODULE.bazel.in` during CMake configure; do not edit it manually. The version is taken from `CMakeLists.txt`, so after bumping it for a release, rerun CMake and commit the regenerated `MODULE.bazel` (CI checks the consistency with `tools/bazel/check_module_version.py`).

### Static analysis (clang-tidy)

Add `-DSTATIC_ANALYSIS_CLANG_TIDY=ON` to any CMake configure command (Windows, Linux, or macOS) to run clang-tidy during the build:

```bash
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Debug \
    -DSTATIC_ANALYSIS_CLANG_TIDY=ON \
    -DBUILD_TESTS=ON -DBUILD_JSON_ARCHIVE=ON \
    -DBUILD_CSV_ARCHIVE=ON -DBUILD_MSGPACK_ARCHIVE=ON
ninja -C build
```

### Code coverage (Linux)

Add `-DENABLE_COVERAGE=ON` to a Debug build (GCC/Clang only; rejected on MSVC/clang-cl) to
instrument with `--coverage`, then run the tests and generate the report with the `coverage`
target (requires `gcovr`):

```bash
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON \
    -DBUILD_TESTS=ON -DBUILD_JSON_ARCHIVE=ON \
    -DBUILD_CSV_ARCHIVE=ON -DBUILD_MSGPACK_ARCHIVE=ON
ninja -C build
ctest --test-dir build --output-on-failure -j "$(getconf _NPROCESSORS_ONLN)"   # -j = concurrent tests
ninja -C build coverage   # writes build/coverage/coverage.xml and build/coverage/html/
```

The report is limited to library code (`include/bitserializer/` and `src/`); tests, samples,
benchmarks and VCPKG dependencies are excluded. CI publishes it from the `Ubuntu_GCC_Coverage`
Preflight job.

---

## CMake Options Reference

> [!IMPORTANT]
> All archive options are **OFF by default** — you must explicitly enable what you need.

> Keep this table in sync with [AGENTS.md — CMake Options](AGENTS.md#cmake-options).

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTS` | OFF | Build test executables |
| `BUILD_SAMPLES` | OFF | Build sample projects |
| `BUILD_BENCHMARKS` | OFF | Build performance benchmarks |
| `BUILD_MICROBENCHMARKS` | OFF | Build intra-binary A/B microbenchmarks |
| `BUILD_JSON_ARCHIVE` | OFF | Built-in JSON archive (no external dependency) |
| `BUILD_CSV_ARCHIVE` | OFF | Built-in CSV archive (no external dependency) |
| `BUILD_MSGPACK_ARCHIVE` | OFF | Built-in MsgPack archive (no external dependency) |
| `BUILD_RAPIDJSON_ARCHIVE` | OFF | JSON archive via RapidJSON |
| `BUILD_PUGIXML_ARCHIVE` | OFF | XML archive via PugiXml |
| `BUILD_RAPIDYAML_ARCHIVE` | OFF | YAML archive via RapidYAML |
| `BUILD_SHARED_LIBS` | OFF | Build as shared/DLL library |
| `BITSERIALIZER_BUNDLE_FAST_FLOAT` | OFF | Install the matched fast_float headers into the package (requires FastFloat) |
| `STATIC_ANALYSIS_CLANG_TIDY` | OFF | Enable clang-tidy during build |
| `ENABLE_COVERAGE` | OFF | Instrument with `--coverage` and add the `coverage` target (GCC/Clang only, not MSVC) |
| `CMAKE_CXX_STANDARD` | 17 | C++ standard (17, 20, 23, or 26) |

> [!NOTE]
> Selecting `CMAKE_CXX_STANDARD=26` requires CMake 3.30 or newer. MSVC has no C++26 mode: with MSVC, `23` maps to `/std:c++latest`, which is the closest available mode.

---

## Project Structure

Paths are relative to the project root:

```
BitSerializer/
├── include/bitserializer/          # Public headers (the library)
│   ├── bit_serializer.h            # Main entry: LoadObject<>() / SaveObject<>()
│   ├── *_archive.h                 # One header per format (json, csv, msgpack, pugixml, rapidjson, rapidyaml)
│   ├── key_value.h                 # KeyValue<> and AttrValue<> wrappers
│   ├── validate.h                  # Validators: Required, Email, PhoneNumber, MaxSize, Range...
│   ├── refine.h                    # Refiners: TrimWhitespace, ToLowerCase, ToUpperCase, Fallback
│   ├── convert.h                   # String conversion submodule (To<>/From<>)
│   ├── serialization_options.h     # SerializationOptions struct
│   ├── config.h                    # Version macros and feature flags
│   ├── export.h                    # DLL export/import macros
│   ├── serialization_detail/       # Core engine (see docs/development/architecture.md)
│   ├── conversion_detail/          # Conversion internals (fundamentals, enums, chrono, UTF, filesystem)
│   ├── types/std/                  # STD type serializers (vector, map, optional, chrono, tuple, etc.)
│   └── common/                     # Utilities (memory.h, text.h)
│
├── src/                            # Implementations for built-in formats
│   ├── csv/                        # CSV reader/writer + csv_archive.cpp
│   ├── json/                       # JSON root scopes + readers/ + writers/
│   ├── msgpack/                    # MsgPack reader/writer + msgpack_archive.cpp
│   ├── common/                     # Binary stream reader
│   └── testing_tools/              # Shared test utilities (fixtures, assertions; GTest-based)
│
├── tests/
│   ├── unit_tests/                 # Per-module unit tests
│   │   ├── common_tests/           # Binary stream reader, shared utilities
│   │   ├── core_tests/             # Archive base, dispatch, context, validators, refine
│   │   ├── convert_tests/          # Type conversion, UTF encoding, chrono, filesystem
│   │   ├── csv_tests/              # CSV reader/writer
│   │   ├── json_tests/             # JSON reader/writer
│   │   ├── msgpack_tests/          # MsgPack reader/writer
│   │   └── std_types_tests/        # STD container serialization
│   ├── integration_tests/          # Per-archive end-to-end tests
│   │   ├── csv_archive_tests/
│   │   ├── json_archive_tests/
│   │   ├── msgpack_archive_tests/
│   │   ├── pugixml_archive_tests/
│   │   ├── rapidjson_archive_tests/
│   │   ├── rapidyaml_archive_tests/
│   │   └── convert_api_tests/
│   └── acceptance_tests/           # Installed-library tests (reuses integration tests, CI Valgrind checks)
│
├── samples/                        # 15 example projects (hello_world, validation, versioning, etc.)
├── benchmarks/                     # archives/ (round-trip) + harness/ (perf utils, A/B harness) + microbenchmarks/
├── docs/                           # Per-format documentation + development/
├── cmake/                          # vcpkg triplets + ARM cross-compilation toolchains
├── CI/azure-devops/                # Azure DevOps pipeline definitions
├── tools/static_analysis/          # Clang-tidy scripts
└── CMakeLists.txt                  # Root build file
```

## Architecture Overview

The library uses an **archive pattern**: each format provides `input_archive_type` (deserialization) and `output_archive_type` (serialization). `BitSerializer::LoadObject<TArchive>()` / `SaveObject<TArchive>()` dispatch values through `Detail::Dispatch()`, which routes fundamentals, STD containers, KeyValue pairs, and custom classes to the correct `Serialize()` overload.

See [Architecture](docs/development/architecture.md) for the full sequence diagram, archive traits, scope contract, loading semantics for arrays and objects, and the `ArchiveBase` alias.

---

## Coding Style

When in doubt, follow the patterns you see in the surrounding code. The full conventions (naming, braces, Doxygen comments, license headers and include order) are in [Code style](docs/development/code-style.md).

Key points: C++17 minimum, tabs for indentation, `#pragma once`, clang-tidy with warnings as errors, MIT license header on every source file.

---

## Testing

See [Testing](docs/development/testing.md) for test targets, filtering, the shared helpers and the unit/integration layering rules. Run the full suite before committing with the `ctest` command from the build section.

---

## Performance Benchmarks

If your changes may affect performance (serialization speed or output size), please verify that there is no significant regression by running the benchmarks. See [Performance Benchmarks](docs/development/benchmarks.md) for the full workflow: round-trip benchmark, microbenchmarks (intra-binary A/B), code-layout sensitivity and the compiler rule.

---

## Commit Messages and Pull Requests

Use the commit format `[Component] Short description (#issue)` in imperative mood. Rebase on the latest `master` before opening a PR, keep a linear history, and fold follow-up fixes into the original commit while the branch is under review.

See [Workflow](docs/development/workflow.md) for the component list, commit examples, the full PR workflow, the CI matrix and the checks it runs.
