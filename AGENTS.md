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
| JSON (built-in) | `json_archive.h` | `BitSerializer::Json` | Built-in | `BUILD_JSON_ARCHIVE` |
| JSON (RapidJSON) | `rapidjson_archive.h` | `BitSerializer::Json::RapidJson` | RapidJSON | `BUILD_RAPIDJSON_ARCHIVE` |
| XML | `pugixml_archive.h` | `BitSerializer::Xml::PugiXml` | PugiXml | `BUILD_PUGIXML_ARCHIVE` |
| YAML | `rapidyaml_archive.h` | `BitSerializer::Yaml::RapidYaml` | RapidYAML | `BUILD_RAPIDYAML_ARCHIVE` |
| CSV | `csv_archive.h` | `BitSerializer::Csv` | Built-in | `BUILD_CSV_ARCHIVE` |
| MsgPack | `msgpack_archive.h` | `BitSerializer::MsgPack` | Built-in | `BUILD_MSGPACK_ARCHIVE` |

Dependencies (via vcpkg): rapidjson, pugixml, ryml, gtest, nlohmann-json (benchmark competitor only).

---

## Project Structure

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
│   ├── config.h                    # Version macros, feature flags (BITSERIALIZER_HAS_FLOAT_FROM_CHARS, BITSERIALIZER_HAS_FILESYSTEM)
│   ├── export.h                    # DLL export/import macros
│   ├── serialization_detail/       # Core engine (see Architecture)
│   ├── conversion_detail/          # Conversion internals (fundamentals, enums, chrono, UTF, filesystem)
│   ├── types/std/                  # STD type serializers (vector, map, optional, chrono, tuple, etc.)
│   └── common/                     # Utilities (memory.h, text.h)
│
├── src/                            # Implementations for built-in formats
│   ├── csv/                        # CSV reader/writer + csv_archive.cpp
│   ├── json/                       # JSON reader/writer + json_archive.cpp
│   ├── msgpack/                    # MsgPack reader/writer + msgpack_archive.cpp
│   ├── common/                     # Binary stream reader
│   └── testing_tools/              # Shared test utilities (fixtures, assertions, perf)
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
├── benchmarks/                     # Performance benchmarks vs nlohmann-json, RapidJSON, PugiXml, RapidYAML
├── docs/                           # Per-format documentation (markdown)
├── cmake/
│   ├── vcpkg/triplets/             # Custom triplets for ARM Linux
│   └── toolchains/                 # ARM cross-compilation toolchains
├── CI/azure-devops/                # Azure DevOps pipeline definitions
├── tools/static_analysis/          # Clang-tidy scripts
└── CMakeLists.txt                  # Root build file
```

---

## Key Files Reference

| I want to... | Look at |
|---|---|
| Understand the main API | `include/bitserializer/bit_serializer.h` |
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
| See examples | `samples/` (15 sample projects) |
| Understand CI | `CI/azure-devops/azure-pipelines.yml` |
| Run static analysis | `.clang-tidy`, `tools/static_analysis/` |
| Modify build config | `CMakeLists.txt`, `CMakeSettings.json` |
| Manage dependencies | `vcpkg.json`, `cmake/vcpkg/triplets/` |
| Run performance benchmarks | `benchmarks/archives/` — machine-readable results in `benchmark_results/serialization_speed_report.json` (written when the RapidJSON archive is enabled) |

---

## Architecture

The library uses an **archive pattern**: each format provides `input_archive_type` (deserialization) and `output_archive_type` (serialization). `BitSerializer::LoadObject<TArchive>()` / `SaveObject<TArchive>()` dispatch values through `Detail::Dispatch()`, which routes fundamentals, STD containers, KeyValue pairs, and custom classes to the correct `Serialize()` overload. See `serialization_detail/serialization_dispatch.h` and the respective archive headers for details. For a sequence diagram of the serialization flow with a matching code example, see [Architecture Overview](CONTRIBUTING.md#architecture-overview).

---

## Archive Traits

Each archive declares capabilities via a traits struct (e.g. `JsonArchiveTraits`). Archives are assembled via `ArchiveBase<Traits, ReadRootScope, WriteRootScope>` which sets `input_archive_type` / `output_archive_type`.

| Trait | Role |
|-------|------|
| `preferred_output_type` | Default buffer type for serialized output (`std::string`) |
| `string_view_type` | Canonical string view for I/O (`std::string_view`) |
| `is_binary` | Whether format is binary (`true` for MsgPack) |
| `require_array_size` / `require_map_size` | Whether size must be known before writing |

Stream support is determined at compile-time via `is_archive_support_{input,output}_data_type_v` (checks `is_constructible_v` of root scope with `(stream, SerializationContext&)`). If an archive lacks stream constructors, `bit_serializer.h` applies a fallback: **Load** → read stream into `preferred_output_type`, verify UTF-8, skip BOM, delegate to string `LoadObject`; **Save** → serialize to `preferred_output_type`, write optional BOM + data to stream. Only `char` streams and UTF-8 are supported in the fallback.

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
| `BUILD_BENCHMARKS` | OFF | Build benchmarks |
| `BUILD_JSON_ARCHIVE` | OFF | Built-in JSON archive |
| `BUILD_CSV_ARCHIVE` | OFF | Built-in CSV archive |
| `BUILD_MSGPACK_ARCHIVE` | OFF | Built-in MsgPack archive |
| `BUILD_RAPIDJSON_ARCHIVE` | OFF | RapidJSON-based JSON archive |
| `BUILD_PUGIXML_ARCHIVE` | OFF | PugiXml-based XML archive |
| `BUILD_RAPIDYAML_ARCHIVE` | OFF | RapidYAML-based YAML archive |
| `BUILD_SHARED_LIBS` | OFF | Build as DLL/shared library |

### Build directory

Build output goes to `build/` in the project root (already in `.gitignore`).

### Windows Build (from PowerShell)

The MSVC compiler requires calling `vcvarsall.bat` to set up environment variables. This snippet auto-detects VS 2022, captures the environment, configures, builds, and runs tests:

```powershell
# 1. Auto-detect VS 2022 installation
$vsPaths = @(
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional",
    "C:\Program Files\Microsoft Visual Studio\2022\Community"
)
$vsPath = $vsPaths | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $vsPath) { throw "VS 2022 not found; set `$vsPath manually" }
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
ctest --test-dir build -T test --output-on-failure -j2
```

**Clang-cl variant** (also ships with VS 2022): add to the CMake command:
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
ctest --test-dir build -T test --output-on-failure -j2
```

---

## Testing

### Test executables

Built to `build/bin/` (command-line builds). Names match CTest test names and CMake targets (e.g. `json_tests`, `csv_archive_tests`, `core_tests`). In Visual Studio, binaries are in `%LOCALAPPDATA%\CMakeBuilds\<GUID>\build\<Config>\bin\`.

### Filtering tests (GTest syntax)

```bash
# Single test
./build/bin/convert_tests --gtest_filter="ConvertFundamentals.ToTheSameType"

# Entire suite
./build/bin/convert_tests --gtest_filter="ConvertFundamentals.*"

# Wildcard
./build/bin/convert_tests --gtest_filter="*Bool*"

# Multiple
./build/bin/convert_tests --gtest_filter="Suite1.Test1:Suite2.Test2"

# Exclude
./build/bin/convert_tests --gtest_filter="-Suite.SkipThis"
```

### Via CTest

```bash
ctest -R "TestName" --output-on-failure    # Filter by regex
ctest -T test --output-on-failure -j2      # Run all
```

### Per-target rebuild

```bash
ninja -C build convert_tests    # Rebuild only one test target
```

### Test helpers

Shared test utilities live in `src/testing_tools/` — reuse them instead of writing ad-hoc code.

**`common_test_methods.h`** — serialization test methods. These encapsulate the whole arrange → save → load → assert cycle, so **prefer them over hand-writing a full test**. Every helper uses `AutoFixture` to fill random data and `GTestExpectEq` to compare.

| Helper | Purpose |
|--------|---------|
| `TestSerializeType<TArchive, TValue>()` | Roundtrip a freshly generated value at the root scope: build fixture → save → load → compare |
| `TestSerializeType<TArchive>(value)` | Roundtrip a specific value instance (root scope) |
| `TestSerializeType<TArchive>(KeyValue(k, v))` | Roundtrip a key-value pair at root scope (XML) |
| `TestSerializeArray<TArchive, TValue>(srcSize=7, tgtSize=src)` | Roundtrip a C-array of `TValue` |
| `TestSerializeArrayWithKey<TArchive, TValue>(...)` | Roundtrip a named C-array |
| `TestSerializeTwoDimensionalArray<TArchive, TValue>(...)` | Roundtrip a 2D C-array |
| `TestSerializePmrType<TArchive, TValue>()` | Roundtrip a `std::pmr` container (asserts allocator is `polymorphic_allocator`) |
| `TestSerializeClassToStream<TArchive>(value)` | Roundtrip a class through `std::stringstream` |
| `TestSerializeArrayToStream<TArchive>(array)` | Roundtrip a C-array through a stream |
| `TestSerializeArrayToFile<TArchive>(overwrite=false)` | Roundtrip to/from a temp file (array of `TestPointClass`) |
| `TestLoadingToDifferentType<TArchive>(value, expected)` | Load source into a *different* target type, expect conversion |
| `TestLoadToNotEmptyContainer<TArchive, TContainer>(size)` | Load into a pre-filled container (must be cleared) |
| `TestLoadingEmptyContainer<TArchive, TContainer>()` | Loading an empty array must clear a non-empty container |
| `TestMismatchedTypesPolicy<TArchive, Source, Target>(policy)` | Serialize `Source`, load into incompatible `Target`, verify `MismatchedTypesPolicy` |
| `TestOverflowNumberPolicy<TArchive, Source, Target>(policy)` | Serialize a value near type limits, load into narrower type, verify `OverflowNumberPolicy` |
| `TestValidationForNamedValues<TArchive, T>()` | Validators (`Required()` etc.) on named fields |
| `TestEncodingPolicy<TArchive>(policy)` | UTF encoding error policy on invalid sequences |
| `TestSkippingObjectValueWhenMismatchKey<TArchive>()` | Mismatched keys must leave target fields untouched |
| `TestVisitKeysInObjectScope<TArchive>(skipValues=false)` | Visit keys in an object scope |
| `TestThrowExceptionWhenFileAlreadyExists<TArchive>()` | `SaveObjectToFile` must throw when the file exists |

**`common_test_entities.h`** — reusable serializable test classes. Use these instead of writing your own one-field/multi-field classes for roundtrip tests.

- `TestClassWithSubType<T, KeyValueArgs...>` — wraps a single value under the key `"TestValue"` and serializes it as a named field. Default-ctor fills the value via `BuildFixture`; the `explicit` ctor accepts an initial value (`TestClassWithSubType(initValue)`) plus optional `KeyValue` modifiers (e.g. validators). Use when you need **a single named field** with a **specific value** (unlike `TestClassWithSubTypes`, which only supports default-construction). Also usable when the archive's root scope can't hold a bare value (e.g. objects in `ArchiveStub`). Members: `GetValue()`, `KeyName`.
- `TestClassWithSubTypes<Args...>` — a `std::tuple<Args...>` whose elements are serialized as named fields `"Member_0"`, `"Member_1"`, … Only default-constructible (plus `WithRequired()` to mark all fields `Required()`). Use for **multiple fields** when you don't need to set initial values.
- `TestClassWithAttributes<Args...>` — tuple serialized as XML `AttributeValue`s (`"Attribute_0"`, …) for attribute-only archives.
- `TestClassWithReverseLoad<Args...>` — like `TestClassWithSubTypes` but writes in forward order and loads in reverse order (tests order-agnostic deserialization).
- `TestClassWithCustomKey<T>` — two `pair` members whose keys come from the value's type (min/max of `T`).
- `TestClassWithSubArray<T, Size=7>` / `TestClassWithSubTwoDimArray<T, ...>` — wrap a C-array / 2D C-array under one key.
- `TestClassWithExternalSerialization` — uses a global `SerializeObject()` instead of a member method.
- `TestClassWithInheritance<TBase>` — tests `BaseObject<TBase>`.
- `TestClassWithMismatchedField` — saves as `"id"`, loads as `"guid"`.
- `TestClassWithVersioning` — adds extra fields when saving (versioning).
- `TestClassForCheckValidation<T>` / `TestClassForCheckCompatibleTypes<T>` — a `Required()` field plus a deliberately-absent field.
- `TestPointClass` (2 ints), `TestUnion` (int+float), `TestEnum` — simple fixtures.

**`auto_fixture.h`** — random test-data generator. Core type is `AutoFixture::Fixture` (seeded `std::mt19937`, `GetContainerSize()`), used via:
- Free functions `::BuildFixture(value)` / `::BuildFixture<T>()` — the common case; use the shared default `Fixture` (size 7).
- Explicit `AutoFixture::Fixture().Build(value)` / `.Build<T>()` — configure via fluent `WithContainerSize(n)` / `WithSeed(s)`.

Dispatch is via ADL on `Fixture&`: per-type overloads are free `AutoFixture::BuildFixture(Fixture&, T&)` functions in `namespace AutoFixture`. Custom types opt in with `static void BuildFixture(T&)` (member) or a free `AutoFixture::BuildFixture(Fixture&, T&)`.

**`auto_fixture/std/*.h`** — one header per std type (vector, map, variant, chrono, memory, etc.). Include only the ones your test uses; `auto_fixture/std/all.h` pulls in every std fixture (use in the `*_std_containers_tests.cpp` files). Prefer per-type includes elsewhere.

**`archive_stub.h`** — format-independent archive for unit tests. Key specifics:
- `key_type` is `std::wstring`; root scope supports only values/arrays, **not keyed objects** — wrap objects via `TestClassWithSubType`
- `preferred_output_type` is `TestIoDataRoot` (internal IOData tree), not a string — roundtrip works without any text parsing

### Testing patterns

- **STL types unit tests** (`std_types_tests`) run against `ArchiveStub` only — base serialization logic is shared, so per-archive coverage is done by smoke tests (`SerializeStdTypes`) in each archive's integration tests.
- **Error policy tests**: serialize a *source* type, deserialize into an *incompatible target* type rather than crafting broken input data manually. E.g. variant out-of-range index = save `variant<int, std::string, double>` (index 2 active) → load into `variant<int, std::string>`.
- **Custom options** are passed explicitly: `SerializationOptions options; options.mismatchedTypesPolicy = ...; LoadObject<TArchive>(obj, output, options);`
- **Prefer the `TestSerialize*` helpers** over hand-rolling arrange/save/load/assert. For a single named field with a known value use `TestClassWithSubType`; for several generated fields use `TestClassWithSubTypes`.

---

## Working with This Codebase

### File encoding & line endings

- **Never corrupt encoding.** Many files are UTF-8 **with BOM** and contain non-ASCII text (e.g. `—`, `→`, emoji, Russian strings in docs/tests). Some files are UTF-8 without BOM. Preserve the exact bytes (BOM included) of anything you edit.
- **Use the dedicated `edit`/`write` tools** — they read and write bytes without re-encoding. **Never** edit a file with PowerShell text cmdlets (`Get-Content`/`Set-Content`/`Out-File`) or other shell text redirection: PowerShell 5.1 decodes/re-encodes and corrupts non-ASCII and BOM. Avoid `cmd /c "type ... > file"` style redirects for the same reason.
- **Respect `.gitattributes`** — all source and markdown files are `text eol=lf` (normalized to LF in the repo), so line endings are **not** a correctness concern; git normalizes them on commit. Still, for the user's local convenience, **preserve the file's current line endings** (CRLF or LF) instead of rewriting the whole file — the platform may not be Windows, and a bulk EOL rewrite creates noisy diffs. If you must script a bulk change, operate on bytes via .NET (`[IO.File]::ReadAllBytes`/`WriteAllBytes`) and re-run a `git diff` afterwards.
- **Verify after editing** any file that may contain non-ASCII: run `git diff -- <file>` and confirm the diff contains **no multibyte character changes** (grep the diff for `[^\x00-\x7F]`). If the diff shows `?`, `Â`, or replacement characters, you corrupted the encoding — restore from HEAD (`git checkout -- <file>`) and redo the edit with the `edit` tool.
- **Verify line endings** after editing: the diff should show only your intended change, not a whole-file whitespace rewrite.

### Making changes

1. **Identify the component** you're modifying (archive, core, convert, etc.)
2. **Find the relevant files** using the table above
3. **Build only the affected target**: `ninja -C build <target_name>`
4. **Run targeted tests**: `build/bin/<test>.exe --gtest_filter="*YourTest*"` (from command-line build)
5. **Run full test suite** before committing: `ctest --test-dir build -T test --output-on-failure`
6. **Commit with convention**: `[Component] Short description (#issue)`

Components: `[Core]`, `[Json]`, `[CSV]`, `[MsgPack]`, `[RapidJson]`, `[Convert]`, `[Tests]`, `[CI]`, `[Docs]`.
Examples:
```
[Json] Add support for reading surrogate pairs
[CSV] Optimize deserialization performance (+22%) (#42)
[Convert] Improve float-to-string precision
[RapidJson, PugiXml, RapidYaml] Fix loading optional object from Null (#11)
```

**Working-branch commits:** on a feature branch, commit only meaningful changes. When a commit still needs fixes or refinements (review feedback, CI failures, static-analysis errors, etc.), fold them into that commit with `git commit --amend` (or `git rebase -i` to squash a follow-up) — do **not** create separate "fix" commits. This keeps history clean; the PR ends up with one commit per logical change. Only make a new commit for a genuinely new piece of work. If the branch is already pushed, force-push after amending (`git push --force-with-lease`) — but only push when the user explicitly asks.

### Code style

- **Formatter**: `.clang-tidy`, warnings as errors
- **Indentation**: Tabs, `#pragma once`, C++17
- **License header**: MIT (see [CONTRIBUTING.md](CONTRIBUTING.md))

| Element | Convention | Example |
|---------|-----------|---------|
| Classes / Structs (incl. class templates) | CamelCase, no `T` prefix | `ArchiveScope`, `SerializationOptions` |
| Interfaces | `I` + CamelCase | `IJsonReader` |
| Methods (public/private) | CamelCase | `ReadValue()` |
| Private members | `m` + CamelCase | `mPos` |
| Public struct members | camelCase | `name`, `object`, `index` |
| Constructor parameters | camelCase with `in` prefix to avoid shadowing | `inValue`, `inName` |
| Local variables | camelCase | `hexVal` |
| Template type parameters | `T` + CamelCase | `TArchive`, `TValue` |
| Type aliases | `snake_case_type` | `key_type` |
| Macros | `BITSERIALIZER_` + `SCREAMING_SNAKE_CASE` | `BITSERIALIZER_HAS_FILESYSTEM` |
| Namespaces | CamelCase | `BitSerializer::Json::Detail` |

> [!NOTE]
> The `T` prefix marks a template **parameter** (a placeholder), not a template **class**. Concrete types keep plain CamelCase even when they are class templates (e.g. `ArchiveScope<TMode>`); non-template types never take the `T` prefix either (e.g. `ScopeUnopened`).

- **Braces**: Allman style (opening brace on separate line), always use braces
- **Comments**: Doxygen-style (`/** @brief ... */`) in English

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

Requires the MSVC environment (see Windows build above). Single file: `clang-tidy -p=build-tidy --config-file=.clang-tidy --extra-arg=/EHsc <file.cpp>`.

**Deprecated members:** the *implicit* copy/move constructor reads members one by one, so a class with `[[deprecated]]` members fails clang-tidy as soon as it's copied (e.g. `return e;`) even if your code never reads them. Fix by declaring an explicit copy constructor that initializes deprecated members from their non-deprecated counterparts — *not* with `#pragma`/`NOLINT` (and a `#pragma` inside a member initializer list is ill-formed). See `ParsingException` in `serialization_detail/errors_handling.h`.

### Pull requests

See [CONTRIBUTING.md](CONTRIBUTING.md) for PR workflow (rebase on `dev`, linear history).
