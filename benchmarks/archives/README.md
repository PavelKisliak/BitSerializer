# Archives benchmark

Round-trip serialization benchmark for BitSerializer archives and competing libraries (RapidJSON, PugiXml, RapidYAML, nlohmann-json). It measures Save/Load to memory and to `std::iostream` and writes JSON reports.

## Build

Build in Release mode (Debug builds use a minimal stage time, enough only to verify functionality):

```cmd
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON -DBUILD_JSON_ARCHIVE=ON ...
ninja -C build archives_benchmarks
```

Enable the archives you want via `BUILD_*_ARCHIVE` options. Reports are written through the **built-in JSON archive**, so `BUILD_JSON_ARCHIVE=ON` is required for them (the benchmark itself works without it and prints a warning when reports are requested).

## Command line

```
archives_benchmarks [options]
```

| Option | Description |
|---|---|
| `--bench <list>` | Benchmarks to run (default: `all`). See the keys below. |
| `--stages <list>` | Stages to run (default: `all`). |
| `--list` | List the benchmarks compiled into this binary and their stages. |
| `--stage-time <sec>` | Time per stage (default: 30; debug: 1). |
| `--warmup <sec>` | CPU warm-up before the run (default: 10; debug: 0; `0` disables it). |
| `--output <dir>` | Report directory (default: `benchmark_results`). |
| `--no-report` | Do not write JSON reports. |
| `--help`, `-h` | Show help. |

Run `--list` to see what is compiled in the current binary.

## Benchmark keys

| Key | Benchmark |
|---|---|
| `bs-json` | `BitSerializer-Json` |
| `bs-jsonc` | `BitSerializer-Jsonc` |
| `bs-csv` | `BitSerializer-Csv` |
| `bs-msgpack` | `BitSerializer-MsgPack` |
| `bs-rapidjson` | `BitSerializer-RapidJson` |
| `bs-pugixml` | `BitSerializer-PugiXml` |
| `bs-rapidyaml` | `BitSerializer-RapidYaml` |
| `rapidjson` | `RapidJson` |
| `pugixml` | `PugiXml` |
| `rapidyaml` | `RapidYaml` |
| `nlohmann-msgpack` | `NlohmannJson-MsgPack` |

Meta keys: `bitserializer` (all `bs-*`), `competitors` (all external libraries), `all` (everything compiled in).

Keys are case-insensitive and can be separated by commas or spaces.

## Stages

| Stage | Alias |
|---|---|
| `save-memory` | `save` (with `save-stream`), `memory` (with `load-memory`) |
| `load-memory` | `load` (with `load-stream`), `memory` (with `save-memory`) |
| `save-stream` | `save`, `stream` |
| `load-stream` | `load`, `stream` |

`all` selects every stage. A benchmark that does not support the selected stages is skipped with a message.

## Examples

```cmd
archives_benchmarks --list
archives_benchmarks --bench bs-json --stages save,load --stage-time 10
archives_benchmarks --bench bitserializer --stage-time 60
archives_benchmarks --bench bs-rapidjson,rapidjson --stages load-memory --warmup 0
archives_benchmarks --bench all --no-report
```

## Test model

The benchmark serializes an array of 30 `BasicTestModel` records (10 fields each):

- 5 numeric: `active` (`bool`), `id` (`int32`, short value `-100`), `stock` (`uint64`, long value `123456789`), `price` (`float`), `rating` (`double`);
- 5 string: `name` (short), `detailedProductDescription` (long), `localizedName` (UTF-8, Cyrillic), `note` (escaped quotes), `specifications` (multiline).

The values are deliberately ordinary. The `id`/`stock` pair covers short and long number formatting, and the strings cover plain and escaped text (including a long key). The model represents a typical record, not a conformance or edge-case suite: max values, NaN/Inf, empty strings and nested objects are intentionally out of scope — for isolated type coverage use the models in [`benchmarks/microbenchmarks/benchmark_models.h`](../microbenchmarks/benchmark_models.h).

Keys have a deliberate short/medium/long mix: `id` (2), `name` and `note` (4), `stock` and `price` (5), `active` and `rating` (6), `localizedName` (13), `specifications` (14), and `detailedProductDescription` (26). The long key exceeds the common `std::string` SSO buffer (15 characters on MSVC/libstdc++, 22 on libc++), so it exercises the heap path where keys are copied (for example, nlohmann-json) and affects output size and key lookup in the other writers; most serializers use the literal key directly.

All competitors serialize the same fields. Changing the model changes the output size, speed and the published baseline; `BasicTestModel::GetTotalFieldsCount()` must be updated together with the fields.

## Measurement precision

The benchmark reports the minimum single-iteration time per stage. On Windows the clock tick is 100 ns; an iteration of the fastest stage (MsgPack, ~3.6 µs) is only ~36 ticks, so the reported best can move by one tick (~3%) between runs. This is measurement quantization, not a regression; the other archives have longer iterations and a smaller quantum.

Run the benchmark several times (`run_repeated.py`) and compare the `best` values (or the `median` for a conservative estimate). The ~3% figure is a practical noise band for the fastest stages, not a hard cutoff: a single stage can move by a tick or by code layout, and a change that only affects loading may even shift the reported save result. For that reason, prefer the **combined save+load result**: a repeatable improvement in the sum is stronger evidence than a sub-3% change in one stage. Judge the change by the round-trip result: accept it when the repeated save+load sum improves, even if one of the stages is slower; conversely, reject it when the sum regresses repeatably. Use the intra-binary [microbenchmark harness](../microbenchmarks/README.md) to confirm the mechanism, especially for small or short-lived deltas.

## Reports

Reports are written to `--output` (default `benchmark_results`):

- `serialization_speed_report.json` — fields per millisecond for each library and stage;
- `serialization_output_size_report.json` — serialized size for each library.

If the built-in JSON archive is not enabled, a warning is printed and no reports are written.

## Repeated runs

`run_repeated.py` runs the benchmark several times and aggregates the results. It is copied next to the executable during the build and looks for `archives_benchmarks` in the current directory by default.

```cmd
python run_repeated.py --repeat 5 --bench bs-json --stages save,load --stage-time 10
```

Script options:

| Option | Description |
|---|---|
| `--repeat <n>` | Number of runs (default: 3). |
| `--aggregate <mode>` | Value written to the aggregated report: `best` (default), `median`, `mean`. |
| `--exe <path>` | Path to `archives_benchmarks` (default: current directory, then next to the script). |
| `--output <dir>` | Base directory for reports (default: `benchmark_results`). |
| `--clean` | Remove per-run directories after aggregation. |

All other options (`--bench`, `--stages`, `--stage-time`, `--warmup`, `--list`) are passed to the benchmark. `--no-report` is not supported because the reports are needed for aggregation.

Report layout:

```
benchmark_results/
├── serialization_speed_report.json        # aggregated, same shape as the benchmark output
├── serialization_output_size_report.json
├── run-1/
│   ├── serialization_speed_report.json
│   └── serialization_output_size_report.json
├── run-2/
└── ...
```

The aggregated speed report has the same shape as the benchmark's own report; `--aggregate` changes only the values. The console also prints `best`, `median`, `mean` and `std` for every library and stage, which helps to judge the run-to-run spread.

## Notes

- `--bench` only filters benchmarks that are compiled into the binary. Which archives and competitors are available is decided by the CMake `BUILD_*_ARCHIVE` options.
- For function-level A/B microbenchmarks, see [`benchmarks/microbenchmarks`](../microbenchmarks/README.md).
