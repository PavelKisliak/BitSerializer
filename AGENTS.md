# AGENTS.md

## Overview

BitSerializer is a C++17 header-mostly library for serializing objects to multiple formats (JSON, XML, YAML, CSV, MsgPack) through a unified interface. Licensed under MIT.

Key design principles:
- One common API (`LoadObject`/`SaveObject`) works across all formats
- Modular architecture — each archive is a separate CMake target with its own dependencies; enable only what you need via `BUILD_<ARCHIVE>_ARCHIVE` options
- Compile-time format validation (e.g., JSON allows primitives as roots, CSV only allows arrays)
- RAII-based scopes — object, array, and binary contexts are RAII objects that finalize on destruction
- Traits-based archives — each format declares its capabilities (key types, binary support, size requirements) via a traits struct
- Header-mostly — built-in archives (JSON, CSV, MsgPack) have `src/` implementations; third-party archives (XML, YAML) are header-only

### Supported formats

| Archive | Header | Namespace | Backing library | CMake option |
|---------|--------|-----------|-----------------|--------------|
| JSON/JSONC (built-in) | `json_archive.h` | `BitSerializer::Json` | Built-in | `BUILD_JSON_ARCHIVE` |
| JSON (RapidJSON) | `rapidjson_archive.h` | `BitSerializer::Json::RapidJson` | RapidJSON | `BUILD_RAPIDJSON_ARCHIVE` |
| XML | `pugixml_archive.h` | `BitSerializer::Xml::PugiXml` | PugiXml | `BUILD_PUGIXML_ARCHIVE` |
| YAML | `rapidyaml_archive.h` | `BitSerializer::Yaml::RapidYaml` | RapidYAML | `BUILD_RAPIDYAML_ARCHIVE` |
| CSV | `csv_archive.h` | `BitSerializer::Csv` | Built-in | `BUILD_CSV_ARCHIVE` |
| MsgPack | `msgpack_archive.h` | `BitSerializer::MsgPack` | Built-in | `BUILD_MSGPACK_ARCHIVE` |

Dependencies (via vcpkg): rapidjson, pugixml, ryml, gtest, nlohmann-json (benchmark competitor only), fast-float. fast-float is auto-detected through the `FastFloat` CMake package (opt out with `CMAKE_DISABLE_FIND_PACKAGE_FastFloat=ON`). External consumers/official ports may expose it as an optional feature.

---

## Key Files Reference

| I want to... | Look at |
|---|---|
| Understand the main API | `include/bitserializer/bit_serializer.h` |
| Understand the architecture | `docs/development/architecture.md` |
| Development guide index | `docs/development/README.md` |
| See the full repository map | `CONTRIBUTING.md#project-structure` |
| Work on JSON serialization | `include/bitserializer/json_archive.h`, `src/json/` |
| Work on CSV serialization | `include/bitserializer/csv_archive.h`, `src/csv/` |
| Work on MsgPack serialization | `include/bitserializer/msgpack_archive.h`, `src/msgpack/` |
| Work on XML serialization | `include/bitserializer/pugixml_archive.h` (header-only) |
| Work on YAML serialization | `include/bitserializer/rapidyaml_archive.h` (header-only) |
| Add/modify validators | `include/bitserializer/validate.h` |
| Add/modify refiners | `include/bitserializer/refine.h` |
| Work on type conversion | `include/bitserializer/convert.h`, `conversion_detail/` |
| Add STD type support | `include/bitserializer/types/std/` |
| Understand dispatch mechanism | `include/bitserializer/serialization_detail/serialization_dispatch.h` |
| Work on archive base classes | `include/bitserializer/serialization_detail/archive_base.h` |
| Modify serialization options | `include/bitserializer/serialization_options.h` |
| Work on error handling | `include/bitserializer/serialization_detail/errors_handling.h` |
| Add tests for convert | `tests/unit_tests/convert_tests/` |
| Add tests for core | `tests/unit_tests/core_tests/` |
| Add integration tests | `tests/integration_tests/<archive>_tests/` |
| Find shared test helpers | `src/testing_tools/` |
| See examples | `samples/` (15 sample projects) |
| Understand CI | `CI/azure-devops/azure-pipelines.yml` |
| Run static analysis | `.clang-tidy`, `tools/static_analysis/` |
| Modify build config | `CMakeLists.txt`, `CMakeSettings.json` |
| Manage dependencies | `vcpkg.json`, `cmake/vcpkg/triplets/` |
| Run performance benchmarks | `benchmarks/archives/` — CLI (`--bench`, `--stages`, `--list`) and machine-readable results in `benchmark_results/serialization_speed_report.json` (requires `BUILD_JSON_ARCHIVE=ON`) |
| Benchmark a single function (A/B) | `benchmarks/microbenchmarks/` — intra-binary A/B harness (`benchmarks/harness/bench_harness.h`), see its `README.md` |
| Understand the benchmark workflow | `docs/development/benchmarks.md` |

---

## Architecture

The library uses an **archive pattern**: each format provides `input_archive_type` (deserialization) and `output_archive_type` (serialization). `LoadObject<TArchive>()` / `SaveObject<TArchive>()` dispatch values through `Detail::Dispatch()`, which routes fundamentals, STD containers, KeyValue pairs, and custom classes to the correct `Serialize()` overload.

See [Architecture](docs/development/architecture.md) for the sequence diagram, archive traits, `ArchiveBase`, scope contract and loading semantics. The dispatch entry point is `serialization_detail/serialization_dispatch.h`.

---

## Key APIs

### Serialization wrappers

- `archive << KeyValue("name", value)` — named field (all formats)
- `archive << AttrValue("name", value)` — XML attribute only
- `archive << PropertyValue("name", value)` — auto: XML attr if value is string-like, else key-value
- `archive << KeyValue("name", value, Required())` — field with validator
- `archive << KeyValue("name", value, Validate::Email(), Refine::TrimWhitespace())` — multiple modifiers

### Validators (`Validate::`)

`Required`, `Required("custom message")`, `Email`, `PhoneNumber`, `Range(min, max)`, `MaxSize(n)`, `MinSize(n)`

### Refiners (`Refine::`)

`TrimWhitespace`, `ToLowerCase`, `ToUpperCase`, `Fallback(defaultValue)`

### Supported types

| Category | Header |
|----------|--------|
| Fundamentals (int, float, bool, string) | built-in |
| Enums (register with `BITSERIALIZER_REGISTER_ENUM`) | `conversion_detail/convert_enum.h` |
| Chrono, Filesystem | `types/std/chrono.h`, `types/std/filesystem.h` |
| STD containers (vector, map, set, optional, tuple, pair, variant) | `types/std/*.h` |
| Custom classes | user `Serialize()` method or global `SerializeObject()`/`SerializeArray()` |

---

## Build System

> [!NOTE]
> This is the non-interactive/agent build path. The human setup (Visual Studio, Developer Command Prompt, VCPKG) is in [CONTRIBUTING.md](CONTRIBUTING.md#building-the-project). The shared CMake options are intentionally duplicated in both files — keep them in sync.

### Requirements
- CMake >= 3.10
- VCPKG (set `VCPKG_ROOT` environment variable)
- Ninja build system
- C++17 compiler (MSVC 2019+, GCC 8+, Clang 8+)

### CMake Options

All archive options are **OFF by default** — you must explicitly enable what you need:

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

> Keep this table in sync with [CONTRIBUTING.md — CMake Options Reference](CONTRIBUTING.md#cmake-options-reference). Selecting `CMAKE_CXX_STANDARD=26` requires CMake 3.30 or newer; MSVC has no C++26 mode (`23` maps to `/std:c++latest`).

`FastFloat` is auto-detected for source builds: if the package is available, `Convert` and the built-in archives use `fast_float`; otherwise they use `std::from_chars`. The repository manifest includes `fast-float` as a normal dependency; pass `-DCMAKE_DISABLE_FIND_PACKAGE_FastFloat=ON` to use the fallback. External consumers/official ports can expose it as an optional feature. CI keeps the regular matrix on the fallback path; selected existing jobs (MSVC, clang-cl, GCC) opt in via `fastFloat: true`. `BITSERIALIZER_BUNDLE_FAST_FLOAT=ON` installs the fast_float headers under `include/bitserializer/third_party/fast_float`; release packages enable it, so consumers do not need a separate FastFloat package. The installed package fixes the fast_float state in `bitserializerConfig.cmake` instead of re-detecting it (when bundling is off, it requires FastFloat explicitly). Release packages also install `share/doc/bitserializer/fast_float.txt`.

### Build directory

Build output goes to `build/` in the project root (already in `.gitignore`).

> [!TIP]
> `ctest -j` is the number of tests run concurrently. The snippets below use all logical CPUs (`$env:NUMBER_OF_PROCESSORS` on Windows, `getconf _NPROCESSORS_ONLN` on Linux/macOS); lower the value if the tests exhaust memory or timing-sensitive tests become flaky. Avoid bare `ctest -j`: the optional level is only supported since CMake 3.29, while this project supports CMake 3.10+.

### Windows Build (from PowerShell)

The MSVC compiler requires calling `vcvarsall.bat` to set up environment variables. This snippet auto-detects the newest installed VS 2022/2026, captures the environment, configures, builds, and runs tests:

```powershell
# 1. Auto-detect the newest VS installation (2026 is checked first)
$vsPaths = @(
    "C:\Program Files\Microsoft Visual Studio\18\Enterprise",
    "C:\Program Files\Microsoft Visual Studio\18\Professional",
    "C:\Program Files\Microsoft Visual Studio\18\Community",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional",
    "C:\Program Files\Microsoft Visual Studio\2022\Community"
)
$vsPath = $vsPaths | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $vsPath) { throw "VS 2022/2026 not found; set `$vsPath manually" }
$ninja = "$vsPath\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"

# 2. Capture vcvarsall environment to temp dir (avoids polluting repo)
$tempDir = "$env:TEMP\bitserializer"
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$scriptBlock = @"
@echo off
call "$vsPath\VC\Auxiliary\Build\vcvarsall.bat" x64 > nul
set
"@
$vcvarsScript = "$tempDir\_vcvars.cmd"
Set-Content -Path $vcvarsScript -Value $scriptBlock -Encoding Ascii
$output = & cmd /c "$vcvarsScript"
$envVars = @{}
foreach ($line in $output) {
    if ($line -match '^([^=]+)=(.*)$') { $envVars[$matches[1]] = $matches[2] }
}
foreach ($key in $envVars.Keys) {
    Set-Item -Path "Env:$key" -Value $envVars[$key] -ErrorAction SilentlyContinue
}

# 3. CMake configure (all archives + tests)
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_MAKE_PROGRAM="$ninja" `
    -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
    -DVCPKG_TARGET_TRIPLET=x64-windows-release `
    -DBUILD_TESTS=ON -DBUILD_SAMPLES=ON -DBUILD_BENCHMARKS=ON `
    -DBUILD_JSON_ARCHIVE=ON -DBUILD_CSV_ARCHIVE=ON -DBUILD_MSGPACK_ARCHIVE=ON `
    -DBUILD_RAPIDJSON_ARCHIVE=ON -DBUILD_PUGIXML_ARCHIVE=ON -DBUILD_RAPIDYAML_ARCHIVE=ON

# 4. Build
& $ninja -C build

# 5. Test
ctest --test-dir build -T test --output-on-failure -j $env:NUMBER_OF_PROCESSORS
```

**Clang-cl variant** (ships with both VS 2022 and VS 2026): add to the CMake command:
```powershell
-DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl
```

> **Note:** Environment variables set by vcvarsall are lost when the shell exits. Every `cmake --build` / `ninja` invocation must run in a shell that has executed the environment-capture block (steps 1–2) above — it is **not** enough to configure once. If compilation fails with `fatal error C1083: Cannot open include file: 'iosfwd'` (or `'memory'`, `'cstddef'`), the MSVC environment is missing from the current shell; re-run the capture block before building.

### Linux/macOS Build

```bash
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON -DBUILD_SAMPLES=ON -DBUILD_BENCHMARKS=ON \
    -DBUILD_JSON_ARCHIVE=ON -DBUILD_CSV_ARCHIVE=ON -DBUILD_MSGPACK_ARCHIVE=ON \
    -DBUILD_RAPIDJSON_ARCHIVE=ON -DBUILD_PUGIXML_ARCHIVE=ON -DBUILD_RAPIDYAML_ARCHIVE=ON
ninja -C build
ctest --test-dir build -T test --output-on-failure -j "$(getconf _NPROCESSORS_ONLN)"
```

### Code coverage (Linux only)

Instrumentation is opt-in via `ENABLE_COVERAGE` (adds `--coverage` for GCC/Clang); it is rejected
on MSVC/clang-cl. The `coverage` target runs `gcovr` (must be installed) and writes
`build/coverage/coverage.xml` (Cobertura, used by Azure DevOps) and `build/coverage/html/index.html`.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DBUILD_TESTS=ON \
    -DBUILD_JSON_ARCHIVE=ON -DBUILD_CSV_ARCHIVE=ON -DBUILD_MSGPACK_ARCHIVE=ON \
    -DBUILD_RAPIDJSON_ARCHIVE=ON -DBUILD_PUGIXML_ARCHIVE=ON -DBUILD_RAPIDYAML_ARCHIVE=ON
ninja -C build
ctest --test-dir build --output-on-failure -j "$(getconf _NPROCESSORS_ONLN)"
ninja -C build coverage
```

The report covers only library code (`include/bitserializer/` and `src/`, excluding
`src/testing_tools/`); tests, samples, benchmarks and VCPKG dependencies are filtered out.
CI collects and publishes it in the `Ubuntu_GCC_Coverage` Preflight job.

---

## Testing

> [!IMPORTANT]
> Before adding or modifying tests, read [Testing](docs/development/testing.md). It is the source of truth for the helper catalogue, layering rules and archive-parity requirements; the summary below is not a substitute.

- Use the shared helpers in `src/testing_tools/` (`TestSerialize*`, `TestClassWith*`, `AutoFixture`, `ArchiveStub`) instead of hand-written arrange/save/load/assert code.
- Unit tests (`tests/unit_tests/`) cover behavior and edge cases; integration tests (`tests/integration_tests/`) only verify `LoadObject`/`SaveObject` wiring and the public archive surface. Do not duplicate cases across layers.
- When changing an integration test for one archive, check the same scenario for the other archives and add the analogous test where the format allows it; reuse or add a shared `Test*` helper instead of duplicating the body.
- Run the full suite before committing: `ctest --test-dir build -T test --output-on-failure -j <N>` (`N` = logical CPUs; see the build snippets above).
- Targeted run: `build/bin/<test>.exe --gtest_filter="*YourTest*"`; rebuild one target with `ninja -C build <target>`.

---

## Working with This Codebase

### File encoding & line endings

- **Never corrupt encoding.** Many files are UTF-8 **with BOM** and contain non-ASCII text (e.g. `—`, `→`, emoji, Russian strings in docs/tests). Some files are UTF-8 without BOM. Preserve the exact bytes (BOM included) of anything you edit.
- **Use the dedicated `edit`/`write` tools** — they read and write bytes without re-encoding. **Never** edit a file with PowerShell text cmdlets (`Get-Content`/`Set-Content`/`Out-File`) or other shell text redirection: PowerShell 5.1 decodes/re-encodes and corrupts non-ASCII and BOM. Avoid `cmd /c "type ... > file"` style redirects for the same reason.
- **Respect `.gitattributes`** — all source and markdown files are `text eol=lf` (normalized to LF in the repo), so line endings are **not** a correctness concern; git normalizes them on commit. Still, for the user's local convenience, **preserve the file's current line endings** (CRLF or LF) instead of rewriting the whole file — the platform may not be Windows, and a bulk EOL rewrite creates noisy diffs. If you must script a bulk change, operate on bytes via .NET (`[IO.File]::ReadAllBytes`/`WriteAllBytes`) and re-run a `git diff` afterwards.
- **Verify after editing** any file that may contain non-ASCII: run `git diff -- <file>` and confirm the diff contains **no multibyte character changes** (grep the diff for `[^\x00-\x7F]`). If the diff shows `?`, `Â`, or replacement characters, you corrupted the encoding — restore from HEAD (`git checkout -- <file>`) and redo the edit with the `edit` tool.
- **Verify line endings** after editing: the diff should show only your intended change, not a whole-file whitespace rewrite.
- **Markdown style**: keep each paragraph and bullet on a single line (no hard wrapping); separate paragraphs and list items with blank lines. This applies to `AGENTS.md`, `CONTRIBUTING.md`, `README.md`, `docs/` and other Markdown files.

### Making changes

1. **Identify the component** you're modifying (archive, core, convert, etc.)
2. **Find the relevant files** using the table above. If you will add or change tests, read [Testing](docs/development/testing.md) first — this is mandatory, not optional background reading.
3. **Build only the affected target**: `ninja -C build <target_name>`
4. **Run targeted tests**: `build/bin/<test>.exe --gtest_filter="*YourTest*"` (from command-line build)
5. **Run full test suite** before committing: `ctest --test-dir build -T test --output-on-failure -j <N>` (`N` = logical CPUs)
6. **For performance-affecting changes, run the round-trip benchmark**: see [Benchmarks](docs/development/benchmarks.md). Compare several back-to-back runs in a single session (results drift across sessions). **Keep the default stage time (30 s) — do not shorten it for A/B comparisons**: short runs (a few seconds) have large run-to-run variance on a busy machine and produce misleading results. For **function-level A/B** use the intra-binary harness in [benchmarks/microbenchmarks](benchmarks/microbenchmarks/README.md) (layout-robust, reliable even on MSVC).
7. **Offer static analysis before committing**: ask whether to run clang-tidy on the changed files (see [Static analysis](#static-analysis-clang-tidy) below). Limit it to affected translation units (a changed header can pull in many), and run it in parallel via `run-clang-tidy.py`. Honor a stated user preference (never / always without asking).
8. **Commit with convention**: `[Component] Short description (#issue)` in imperative mood. See [Workflow](docs/development/workflow.md) for the component list, examples and working-branch rules (fold follow-up fixes into the original commit).

### Code style

Follow [Code style](docs/development/code-style.md). Key points: tabs, `#pragma once`, C++17, Allman braces (single-statement blocks may keep the opening brace on the same line), Doxygen comments in English, MIT license header, clang-tidy warnings as errors.

The `T` prefix marks a template **parameter**, not a template **class**; concrete types keep plain CamelCase even when they are class templates (details in the style guide).

### Static analysis (clang-tidy)

CI runs clang-tidy on **changed files** only (`.clang-tidy`, warnings as errors). Reproduce locally before pushing:

```powershell
cmake -GNinja -B build-tidy -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows-release `
  -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl `
  -DSTATIC_ANALYSIS_CLANG_TIDY=ON -DBUILD_TESTS=ON -DBUILD_JSON_ARCHIVE=ON
ninja -C build-tidy json_tests
```

Requires the MSVC environment (see Windows build above).

- **Preferred**: `python tools/static_analysis/clang-tidy/run-clang-tidy.py -config-file=".clang-tidy" -p build-tidy -j <N> <path-regex>` — runs the analysis in parallel (CI uses the same script). A full run is slow (the CI job allows 90 minutes), so limit it to the affected files.
- **Single file**: `clang-tidy -p=build-tidy --config-file=.clang-tidy --extra-arg=/EHsc <file.cpp>`.
- Headers have no compile command of their own — analyze them through a `.cpp` that includes them. To mirror CI's "changed files only" check, use `tools/static_analysis/find_affected_files.py`, which maps changed headers to the affected translation units via `ninja -t deps`.
- Configure `build-tidy` with every archive whose sources/tests you intend to analyze (`-DBUILD_*_ARCHIVE=ON`); otherwise those translation units are absent from `compile_commands.json`. Re-run the configure step after changing CMake options or adding/removing test targets — a stale `compile_commands.json` silently analyzes the wrong file set.

**Deprecated members:** the *implicit* copy/move constructor reads members one by one, so a class with `[[deprecated]]` members fails clang-tidy as soon as it's copied (e.g. `return e;`) even if your code never reads them. Fix by declaring an explicit copy constructor that initializes deprecated members from their non-deprecated counterparts — *not* with `#pragma`/`NOLINT` (and a `#pragma` inside a member initializer list is ill-formed). See `ParsingException` in `serialization_detail/errors_handling.h`.

### Pull requests

See [Workflow](docs/development/workflow.md) for the PR workflow (rebase on `master`, linear history), commit convention and CI matrix.
