### [BitSerializer](../../README.md) / Performance benchmarks

If your changes may affect performance (serialization speed or output size), please verify that there is no significant regression by running the benchmarks.

### Running benchmarks

Build the benchmark executable in Release mode (benchmarks in Debug mode use minimal test time — only enough to verify functionality):

```cmd
:: Windows (from VS Developer Command Prompt)
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON -DBUILD_JSON_ARCHIVE=ON ...
ninja -C build archives_benchmarks

:: Run (from command-line build)
build/bin/archives_benchmarks.exe --list
build/bin/archives_benchmarks.exe --bench bs-json,rapidjson --stages save,load --stage-time 10
```

In Release mode, each archive and stage runs for **30 seconds** to produce repeatable results. The benchmark also warms up the CPU for 10 seconds before starting. Use `--bench`, `--stages`, `--stage-time` and `--warmup` to run a subset without rebuilding; see [benchmarks/archives/README.md](../../benchmarks/archives/README.md) for the full CLI and the `run_repeated.py` helper for repeated runs.

> [!IMPORTANT]
> Avoid using the system during the test for accurate results. Close other applications and background processes that may cause fluctuations.

### Benchmark details

- Pins the process to a single CPU core and sets maximum priority to minimize fluctuations
- Compares BitSerializer archives against raw usage of underlying libraries (RapidJSON, PugiXml, RapidYAML, nlohmann-json)
- Measures serialization speed (fields/ms) for save and load operations
- Uses a representative flat model (5 numeric and 5 string fields); see [Test model](../../benchmarks/archives/README.md#test-model) for the intent and balance
- Reports are saved as JSON files in `benchmark_results/` directory (requires the built-in JSON archive, `BUILD_JSON_ARCHIVE=ON`)

### Why the full round-trip benchmark matters

Performance changes must be validated with the **full round-trip benchmark** (save → load of a representative model), not only with microbenchmarks of individual functions. Compiler code generation can shift for reasons unrelated to the algorithm you changed:

- A real case: after templating the JSON reader on the format (`ArchiveType::Json`/`Jsonc`), strict-JSON load regressed ~6% (16500 → ~15493 fields/ms) even though the strict path was unchanged — `if constexpr` removed all JSONC handling and the hot functions' assembly was byte-identical to before. The cause was purely compiler behavior: MSVC generated slower code for the strict instantiation when the larger JSONC instantiation was compiled in the same translation unit. Splitting the explicit instantiations into separate TUs restored the performance.

Lessons:

- **Measure the round-trip.** A change that looks neutral in isolation can still move the whole pipeline.
- **Microbenchmarks of a single function are still useful** for profiling and hypothesis testing (e.g. whether `__forceinline`/`noinline` helps a hot loop), but they cannot detect whole-TU/binary codegen effects — never rely on them alone.
- **Judge small effects by the combined save+load result across repeated runs.** Layout can move save and load independently, so a load-only change may even shift the save number. A repeatable improvement in the sum, supported by a function-level microbenchmark, is acceptable even if one of the stages is slower; a repeatable regression in the sum should block the change.
- **Compare in a single session.** Machine state drifts between sessions; take several back-to-back runs (min-time per metric) of the configurations you are contrasting.
- **Don't shorten the run.** Keep the default stage time (30 s per stage). Cutting it to a few seconds sharply increases run-to-run variance on a busy machine, so the min-time results become unreliable and can even flip the sign of a small (few %) difference.

### Microbenchmarks (intra-binary A/B)

For **function-level** questions — "is `std::to_chars` faster than `std::to_string` here?", "does `__forceinline` help this loop?" — use the intra-binary A/B harness in [`benchmarks/microbenchmarks/`](../../benchmarks/microbenchmarks/README.md).

It compiles the **base and variant implementations into the same binary** and measures them alternately in one process. Because both share the build's code layout, the measured *difference* is not affected by the layout lottery — this makes it reliable even on MSVC, where function alignment is unavailable. A conservative noise band (block-to-block spread of the delta) is reported next to each delta, and a delta is only flagged as significant when it exceeds that band.

```cmd
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_MICROBENCHMARKS=ON -DBUILD_JSON_ARCHIVE=ON ^
      -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
ninja -C build microbenchmark
build\bin\microbenchmark.exe
```

Microbenchmarks answer "which implementation of this function is faster", not "is the whole pipeline faster" — still validate final changes with the round-trip benchmark above. See the [harness README](../../benchmarks/microbenchmarks/README.md) for the API and recommendations.

The harness also covers the **full archive Save/Load pipeline on a small model** (see the [archive A/B section](../../benchmarks/microbenchmarks/README.md#archive-saveload-ab)) — useful for A/B of two archives or two implementations without the heavy round-trip run.

For the recommended optimization protocol (function-level A/B → archive-level A/B → round-trip validation), see the [A/B workflow](../../benchmarks/microbenchmarks/README.md#optimizing-an-archive-ab-workflow).

### Code-layout sensitivity and the toolchain rule

The benchmark reports the **minimum** time over millions of single-call samples. The minimum is an extreme-value statistic, so it is very sensitive to the *alignment* of the hot functions: a benign code change anywhere in the linked image — even code that never runs in the timed region — can shift them by one cache line and move the reported speed by ~5-10%. This is a "layout lottery", not a real regression. It can be reproduced by adding a dummy function to a hot translation unit and rebuilding: the unchanged algorithm then reports a different `min`.

To keep builds comparable, the benchmark target aligns its functions to 64 bytes (`-mllvm -align-all-functions=6` on Clang, `-falign-functions=64 -falign-loops=32` on GCC), which collapses the spread to ~1-2%. This is applied to the benchmark target only; the library build is untouched (function alignment has no ABI impact, so it stays compatible with any DLL/static build).

**Rule: run authoritative A/B benchmarks with Clang or GCC.**
- Clang/GCC can freeze function alignment; MSVC (`cl.exe`) cannot (no per-function alignment attribute or flag), so its spread stays at ~7% and small deltas are masked by layout noise.
- On Windows, `clang-cl` (ships with Visual Studio) is enough — no separate toolchain needed: add `-DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl` to the CMake command.
- Absolute numbers differ between compilers — only compare deltas measured with the same compiler and flags.
- MSVC runs remain useful as a smoke check, but treat deltas below ~7% as noise.

### Error handling: exceptions vs `Expected`

Exceptions are the primary error-handling mechanism by design. An experimental migration of the `Convert` submodule to `Expected`-based error handling showed a regression in real serialization benchmarks (CSV save −9%, load −5%): the overhead is paid on every successful call, whereas exceptions are zero-cost on the happy path and cost only on rare error paths. `Expected`/error codes are therefore not used in the hot serialization code.
