### [BitSerializer](../../README.md) / Testing

Build commands (including the coverage build flags) live with the platform setup in [CONTRIBUTING.md](../../CONTRIBUTING.md) for humans and [AGENTS.md](../../AGENTS.md) for non-interactive/agent sessions.

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
ctest -T test --output-on-failure -j <N>   # Run all; N = logical CPUs
```

`-j` is the number of tests run concurrently. Use `%NUMBER_OF_PROCESSORS%` in `cmd`, `$env:NUMBER_OF_PROCESSORS` in PowerShell, `$(getconf _NPROCESSORS_ONLN)` (or `$(nproc)`) on Linux/macOS. Lower it if the tests exhaust memory or timing-sensitive tests become flaky. Avoid bare `ctest -j`: the optional level is only supported since CMake 3.29, while this project supports CMake 3.10+.

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
| `TestSkippedArrayValues<TArchive>()` | Null/overflow-skipped array elements still consume a slot (fixed array, pre-filled/empty containers, nested array) |
| `TestMismatchedArrayValue<TArchive>(input)` | A type-mismatched (`Skip`) array element still consumes a slot; `input` is the archive-specific serialized array |
| `TestMismatchedObjectArrayValue<TArchive>()` | A scalar in place of an object inside an array is skipped and still consumes a slot (object-scope mismatch) |
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
- `TestStringLikeType` / `TestStdStringLikeType` / `TestWStringLikeType` — string-like types registered via `BITSERIALIZER_DECLARE_STRING_TYPE(_EXPLICIT)`; defined in `string_like_test_types.h` (include it explicitly). `TestStringLikeType` uses the explicit macro (`Data()`/`Size()`/`FromString()`), `TestStdStringLikeType` the short form (`data()`/`size()`/`assign()`), `TestWStringLikeType` covers `wchar_t`. Use these instead of declaring local string wrappers in tests.
- `TestPointClass` (2 ints), `TestUnion` (int+float), `TestEnum` — simple fixtures.

**`auto_fixture.h`** — random test-data generator. Core type is `AutoFixture::Fixture` (seeded `std::mt19937`, `GetContainerSize()`), used via:
- Free functions `::BuildFixture(value)` / `::BuildFixture<T>()` — the common case; use the shared default `Fixture` (size 7).
- Explicit `AutoFixture::Fixture().Build(value)` / `.Build<T>()` — configure via fluent `WithContainerSize(n)` / `WithSeed(s)`.

Dispatch is via ADL on `Fixture&`: per-type overloads are free `AutoFixture::BuildFixture(Fixture&, T&)` functions in `namespace AutoFixture`. Custom types opt in with `static void BuildFixture(T&)` (member) or a free `AutoFixture::BuildFixture(Fixture&, T&)`.

**`auto_fixture/std/*.h`** — one header per std type (vector, map, variant, chrono, memory, etc.). Include only the ones your test uses; `auto_fixture/std/all.h` pulls in every std fixture (use in the `*_std_containers_tests.cpp` files). Prefer per-type includes elsewhere.

**`archive_stub.h`** — format-independent archive for unit tests. Key specifics:
- `key_type` is `std::wstring`; root scope supports only values/arrays, **not keyed objects** — wrap objects via `TestClassWithSubType`
- `preferred_output_type` is `TestIoDataRoot` (internal IOData tree), not a string — roundtrip works without any text parsing

### Unit vs integration tests

Prefer unit tests for behavior and edge cases; keep integration tests to the minimum needed to prove the public archive surface and its wiring.

- **Unit tests** (`tests/unit_tests/`) cover the main functionality and edge cases of a component: readers/writers, `ArchiveStub`, conversion, validators, options and error policies. Cover every relevant variant there (e.g. string vs stream reader, JSON vs JSONC) with typed or parameterized tests.
- **Integration tests** (`tests/integration_tests/`) only verify what a unit test cannot: `LoadObject`/`SaveObject` wiring, archive aliases and format selection, stream/file overloads, installation/acceptance checks, and one small smoke test per archive.
- **Do not duplicate** the same behavior in both layers. If a unit test already covers the cases, the integration test should assert only the wiring, with one representative input per path (e.g. string and stream), instead of repeating the whole case matrix.
- **Decision rule**: if a test can be written against a `Detail::` class or `ArchiveStub`, it belongs to unit tests; if it only makes sense through `LoadObject`/`SaveObject` or a public archive type, it belongs to integration tests.
- **Example**: strict trailing-garbage validation is covered by typed unit tests for both readers and both JSON/JSONC dialects; the integration suite only checks that `LoadObject` reaches `Finalize()` for string, stream and object input.

### Integration test parity across archives

Keep archive integration tests consistent in composition and ordering where the format allows it. When adding or changing an integration test for one archive, check whether the same scenario applies to the other archives and add the analogous test there (format limitations, e.g. CSV without arrays of objects, may exclude some cases). Prefer reusing an existing `Test*` helper from `src/testing_tools/common_test_methods.h`; if the scenario is generic but no helper exists, add a new reusable helper there instead of duplicating the arrange/act/assert code. Keep the order of related test blocks aligned across archive test files (raw payload, skipped array values, error policies, etc.) so differences between archives remain visible.

### Testing patterns

- **STL types unit tests** (`std_types_tests`) run against `ArchiveStub` only — base serialization logic is shared, so per-archive coverage is done by smoke tests (`SerializeStdTypes`) in each archive's integration tests.
- **Error policy tests**: serialize a *source* type, deserialize into an *incompatible target* type rather than crafting broken input data manually. E.g. variant out-of-range index = save `variant<int, std::string, double>` (index 2 active) → load into `variant<int, std::string>`.
- **Custom options** are passed explicitly: `SerializationOptions options; options.mismatchedTypesPolicy = ...; LoadObject<TArchive>(obj, output, options);`
- **Prefer the `TestSerialize*` helpers** over hand-rolling arrange/save/load/assert. For a single named field with a known value use `TestClassWithSubType`; for several generated fields use `TestClassWithSubTypes`.
