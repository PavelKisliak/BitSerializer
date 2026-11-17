# BitSerializer (History log)

## 0.90 (17 Nov 2026)

### New features
- New built-in JSON/JSONC archive (about 3x faster than the previous RapidJSON-based archive):
    - Strict RFC 8259 JSON parsing and JSONC comments/trailing commas.
    - Native stream I/O for memory and files.
- New standard type support:
    - `std::variant` serialization (initial implementation by [@hoshinohikari](https://github.com/hoshinohikari)):
        - Index-, name- and discriminator-based representations (`VariantAsIndexed`, `VariantAsNamed`, `VariantAsDiscriminated`).
        - Custom tag/content names and selectable default representation via `SerializationOptions::variantOptions`.
        - `std::monostate`.
    - `std::basic_string_view<T>` (save-only).
    - `std::span` (C++20; `std::span<const T>` is save-only).
    - `std::chrono::year_month_day`/`hh_mm_ss` (C++20).
- New validators: `GreaterThan`, `GreaterThanOrEqual`, `LessThan`, `LessThanOrEqual`, `MultipleOf` and `Uuid`.
- Added `PropertyValue` wrapper that automatically chooses an XML attribute or an element/key-value depending on the archive.
- More control over the representation of map-like containers:
    - Wrappers `MapAsObject`, `MapAsDictionary` and `MapAsPairs`.
    - Selectable default `MapLoadMode` via `SerializationOptions::mapOptions`.
- Added `BITSERIALIZER_DECLARE_STRING_TYPE` for zero-copy serialization of custom string types:
    - Usable as map keys.
    - Supported by string validators and refiners.
- Payload passthrough for the built-in JSON and MsgPack archives.
- RapidYaml v0.16 support.

### Changes
- `trimStringFields` now applies to all string-like values; `maxValidationErrors` limits fields with errors rather than individual messages.
- Loading a boolean from numbers greater than 1 now reports `MismatchedTypes` instead of `Overflow`.
- `SerializationException` is preserved when `ConvertByPolicy` is used.
- `std::pair` serialization no longer uses static locals, so custom allocators work correctly (thanks [@hoshinohikari](https://github.com/hoshinohikari)).
- Significant performance improvements (thanks [@Cyberex](https://github.com/Cyberex) for testing on real ARM hardware):
    - `fast_float` parsing acceleration (optional; bundled in prebuilt packages).
    - Built-in JSON: SIMD scanning/escaping and buffered stream output.
    - CSV and MsgPack: SIMD CSV scanning and buffered MsgPack stream I/O.
    - Faster key comparison across archives.
- `SaveObject` now honors the requested output type and supports output streams.

### Breaking changes
- Built-in JSON behavior differs from the deprecated RapidJSON archive:
    - Saving `NaN`/`Infinity` throws `SerializationException` (previously it did not produce a valid JSON value).
    - Raw payloads are stored as opaque JSON text (not a DOM); loading scans them only to determine their extent.
    - Saving re-emits the captured value verbatim without validation.
- `std::multimap` is now saved as a dictionary by default; use `MapAsPairs` for the previous representation.

### Deprecations
- RapidJSON-based JSON archive; use `BitSerializer::Json::JsonArchive` ([migration guide](docs/bitserializer_rapidjson.md)).
    - The archive remains available during the transition period.
- `ParsingException::Line`/`Offset` -> `line`/`offset`.
- `CRawTime`/`CTimeRef` -> `UnixTime`/`TimeRef`.
- `EnumAsBin` -> `EnumAsInteger` (the old name remains available as a deprecated alias).

### Bug fixes
- Fixed PugiXml/RapidJSON array-scope edge cases and skipped array values in MsgPack.
- Fixed `TrimWhitespace` for whitespace-only strings and `Fallback` copying of lvalue values.
- Fixed validator messages and edge cases, including a GCC 14 `-Warray-bounds` warning in the `Email` validator.
- Fixed UTF validation/detection, enum matching/numeric parsing and float-to-string precision for legacy compilers.
- Fixed `ToLowerCase`/`ToUpperCase` to use locale-independent ASCII case conversion.
- Fixed the fallback floating-point conversion path (legacy compilers without `from_chars`) to be locale-independent.
- Fixed PugiXml wide-character mode (`PUGIXML_WCHAR_MODE`), which previously failed to compile in several paths.

### Build and packaging
- Added first-class Bazel support (`MODULE.bazel`/`BUILD.bazel`, integration tests and CI).
- Bazel consumers pull PugiXml/RapidYAML only when they add those third-party modules explicitly; MsgPack-only builds stay free of them.
- Prebuilt packages bundle `fast_float` headers and include the built-in JSON archive.

## 0.85 (11 Jan 2026)
- [ ! ] Introduced deserialization postprocessors (`Fallback`, `TrimWhitespace`, `ToLowerCase`, `ToUpperCase`).
- [ ! ] Added the ability to pass through the payload of raw data structures (thanks @hoshinohikari for the idea!).
- [ + ] Added new serialization option `trimStringFields` (automatically trims whitespace from all string fields).
- [ + ] Added new sample `payload_passthrough`; see also the corresponding section in `README.md`.
- [ * ] Reworked all comments in Doxygen style.
- [ * ] Added prefix `BITSERIALIZER_` to the enum registration macros: `REGISTER_ENUM` and `DECLARE_ENUM_STREAM_OPS`.
- [ * ] Moved some header files containing public types to the root directory of `bitserializer`.
- [ * ] Fixed typo where the alias for `MaxSize` was incorrectly named `Validate::MinSize` (thanks @mattlutz-acre).
- [ * ] Changed `OverflowTypeException` to `MismatchedTypeException` when attempting to load a negative value into an unsigned type.
- [ * ] [Convert] Changed exception type to `invalid_argument` when attempting to convert a negative value to an unsigned type.
- [ * ] [CSV] Optimized deserialization performance (+22% faster from memory and +32% faster from stream).
- [ * ] [MsgPack] Fixed crash in certain cases when loading corrupted MsgPack data (caused by a nested exception).
- [ * ] [CSV, PugiXml, RapidYaml] Fixed cross-platform compatibility for serializing special floating-point values (`INF`, `NAN`).
- [ * ] [RapidJson, PugiXml, RapidYaml] Fixed loading of optional objects and arrays from `null` values (thanks @mattlutz-acre).

## 0.80 (3 May 2025)
- [ ! ] Improved code quality and sanity checks of commits with the help of ClangTidy and Valgrind.
- [ ! ] Changed the priority of serialization methods: the internal `Serialize()` method now has a higher priority than the global one.
- [ ! ] Validators were moved to the `BitSerializer::Validate` namespace to avoid conflicts when `using namespace BitSerializer`.
- [ + ] Added support for building a shared (dynamic) library.
- [ + ] Added visualization of benchmarks and testing of serialization to STD streams.
- [ + ] Added support for serialization of class hierarchy when the base class uses global `SerializeObject()` function.
- [ + ] Added extra arguments to `Convert` functions (can be used to pass an allocator or an existing string).
- [ * ] Fixed an issue with serialization of custom arrays, related to detection of the `size()` function (thanks @marton78).
- [ + ] Added new sample `serialize_custom_array` and new related chapter in the "README.md".
- [ * ] Optimized loading into `std` containers.
- [ + ] Added publishing of binaries to GitHub releases.
- [ * ] Rewritten `Hello world!` example to better demonstrate the library's capabilities.
- [ * ] [CSV] Fixed reading escaped values from `std::istream`.
- [ * ] [MsgPack] Fixed serialization of `std::tuple` (thanks @marton78).
- [ * ] [MsgPack, RapidJson] Fixed serialization of `long` and `unsigned long` (they did not compile on all platforms).
- [ * ] [RapidJson] Fixed serialization of `std::unordered_map` and `std::unordered_multimap`.
- [ * ] [RapidJson] Fixed loading `null` into a string type; it no longer leads to a `MismatchedTypes` error (similar to numbers).
- [ * ] [RapidYaml] Added support for the latest released library v0.8.0 (+70% to performance and now works on ARM).
- [ - ] Removed the deprecated CppRestJson archive; use RapidJson instead.
- [ - ] Removed deprecated classes and functions - `Version`, `MakeKeyValue`, `AutoKeyValue`, `MakeAutoKeyValue`.
- [ - ] Removed deprecated classes and functions - `MakeAttributeValue`, `AutoAttributeValue`, `MakeAutoAttributeValue`.

## 0.75 (5 Jan 2025)
- [ ! ] Added support for the ARM architecture (including platforms with big-endian byte order).
- [ + ] Added new option `SerializationOptions::utfEncodingErrorPolicy` to configure handling of UTF encoding errors.
- [ + ] Added support for serialization types: `std::unordered_multiset`, `std::unordered_multimap`, `std::u8string` (C++20).
- [ + ] Added optional `overwrite` flag to `SaveObjectToFile()` function (`false` by default).
- [ + ] Added `PhoneNumber` validator.
- [ * ] Fixed saving of constant values (thanks @marton78).
- [ * ] Fixed serialization of `std::set`, `std::multiset`, `std::unordered_map` and `std::unordered_multimap` with a custom allocator.
- [ * ] [Convert] Improved UTF encoders; they now return more details about errors.
- [ * ] [Convert] Moved all UTF encoders and related code into sub-namespace `BitSerializer::Convert::Utf`.
- [ * ] [RapidJson] Fixed conflict with the `GetObject` macro from Windows headers (thanks @psallandre).
- [ * ] [PugiXml] Slightly improved performance of saving to stream.
- [ - ] [CppRestJson] The JSON archive based on the CppRestSdk library is deprecated; use the [JSON archive "bitserializer-rapidjson"](docs/bitserializer_rapidjson.md).

## 0.70 (31 May 2024)
- [ ! ] Added new archive for serialization to [MsgPack](docs/bitserializer_msgpack.md) (built-in implementation, no dependencies).
- [ ! ] Improved performance for all archives (especially for **CSV**).
- [ ! ] Deprecated `AutoKeyValue` and `AutoAttributeValue` (use regular `KeyValue` and `AttributeValue` for all cases).
- [ ! ] Deprecated `Version` structure, please use similar macros `BITSERIALIZER_VERSION_*`.
- [ ! ] Optimized string serialization, especially for your own types (use internal function `Detail::SerializeString()`).
- [ + ] Added support for serialization types: `std::filesystem::path`, `std::atomic type`, `std::byte` and `std::valarray`.
- [ + ] Added a `config.h` file with version macros and several other library options (e.g., disable `std::filesystem`).
- [ + ] Added the `EnumAsBin` wrapper to serialize enum types as integers (registration is not required in this case).
- [ * ] Implemented `Overflow` and `Mismatched` policies for `std::chrono` types serialization.
- [ * ] Fixed handling of mismatched types when the target value is an array or object.
- [ * ] Changed `OverflowTypeException` to `MismatchedTypeException` when trying to load a float into an int.
- [ + ] Added the ability to pass custom error messages to validators.
- [ + ] Added email validator.
- [ + ] Added new samples "msgpack_vs_json" and "versioning".
- [ + ] Added new option `maxValidationErrors` to `SerializationOptions`.
- [ * ] A `SerializationException` with error code `UnregisteredEnum` is thrown when serializing an unregistered enum.
- [ + ] [Convert] Added new API function `IsConvertible<TIn, TOut>()`.
- [ + ] [Convert] Use modern `from_chars()` and `to_chars()` for converting float types (if available).
- [ + ] [Convert] Allowed converting any fundamental type to any other fundamental type.
- [ * ] [Convert] Fixed floating-point number conversion after failure due to overflow.
- [ * ] [Convert] Increased the number of significant digits for float from 6 to 7.
- [ * ] [Convert] Changed exception type to `invalid_argument` when converting a string with a floating-point number to an integer.
- [ * ] [Convert] Fixed conversion (and serialization) of negative `std::duration` with fractions of a second.
- [ * ] Fixed compatibility with C++ 20.
- [ - ] Removed deprecated `REGISTER_ENUM_MAP`.
- [ * ] [RapidYaml] Replaced usages of the deprecated API function (compatibility is preserved).
- [ * ] [RapidYaml] Changed serialization of boolean values to "true|false", as in other archives (thanks @psallandre).

## 0.65 (12 Sep 2023)
- [ ! ] The repository has been migrated to GitHub.
- [ ! ] Added support for serialization of `chrono::time_point`, `chrono::duration` and `time_t`.
- [ ! ] Conversion sub-module: Added conversion of `chrono::time_point`, `chrono::duration` and `time_t`.
- [ ! ] Functions `MakeKeyValue` and `MakeAutoKeyValue` were marked as deprecated (please use constructors directly).
- [ ! ] Functions `MakeAttributeValue` and `MakeAutoAttributeValue` were marked as deprecated (please use constructors directly).
- [ + ] Added support for serialization of `std::tuple` (as an array in the target archive; cannot be used with the CSV archive).
- [ * ] Applied `MismatchedTypesPolicy` when serializing enum types.
- [ * ] Applied the `Overflow` and `Mismatched` policies when loading keys of map types.
- [ * ] Fixed detection of integer overflow (used by `OverflowNumberPolicy`).
- [ * ] Fixed serialization of `std::multimap` (was broken in v0.50).
- [ * ] Optimized loading of `std::unordered_map` (pre-reserve the size if possible).
- [ * ] Changed the measurement units in the performance tests from **kb/s** to **fields/ms**.
- [ - ] Removed deprecated global `SerializationContext`.
- [ * ] [RapidJson, CSV] Optimized the performance.

## 0.50 (5 Dec 2022)
- [ ! ] Added a new archive for serialization to CSV; it supports all UTF encodings with auto-detection (built-in implementation, no dependencies).
- [ ! ] API breaking change: deprecated the global `BitSerializer::Context`; validation errors now propagate only via `ValidationException`.
- [ ! ] Removed all static memory allocations to be compatible with custom allocators.
- [ + ] Added policy `OverflowNumberPolicy` for the case when the target type is too small to load the number.
- [ + ] Added policy `MismatchedTypesPolicy` for the case when the type of the target field does not match the loaded value.
- [ + ] Added default `SerializationOptions`.
- [ * ] Added `ParsingException` with information about the line number or offset (depending on the format type).
- [ * ] Added the simplified macro `REGISTER_ENUM`, replacing the deprecated `REGISTER_ENUM_MAP`.
- [ + ] Conversion sub-module: Added an error policy for UTF encoding (error mark, throw exception or skip).
- [ * ] Conversion sub-module: Added throwing `invalid_argument` when converting an invalid string to a number.
- [ * ] Conversion sub-module: Converting a string containing a floating-point number to an integer now throws `out_of_range`.
- [ * ] Conversion sub-module: Fixed handling of raw pointers in the UTF-16Be and UTF-32Be encoders.
- [ * ] Conversion sub-module: Fixed macro `DECLARE_ENUM_STREAM_OPS` (it could not be used in namespaces).
- [ * ] [CppRestJson] Fixed serialization of booleans in objects (they were serialized as numbers).
- [ * ] [RapidYaml] Fixed compatibility with the latest version of the RapidYaml library (0.4.1).
- [ * ] [RapidYaml] Fixed serialization of negative int8.
- [ * ] [RapidYaml] Fixed an issue with error handling during multithreaded serialization.
- [ * ] [RapidJson, CppRestJson, RapidYaml] Fixed the path in validation errors: the index in arrays was shifted by 1.

## 0.44 (1 Oct 2021)
- [ ! ] Changed the minimum requirement for the Clang compiler from version 7 to 8.
- [ ! ] API breaking change: the global `Serialize()` function must now return `bool`.
- [ ! ] API breaking change: validation messages are now stored in UTF-8; `GetValidationErrors()` returns a vector of `std::string`.
- [ ! ] Simplified implementing non-intrusive serialization: only one global function must now be defined (compatibility is preserved).
- [ ! ] The internal string conversion method `FromString()` now accepts any `std::string_view` type instead of `std::string`.
- [ - ] The internal string conversion method `ToWString()` was deprecated; implement `ToU16String()` and/or `ToU32String()` when needed.
- [ + ] Conversion sub-module: Added support for a globally defined `To(in, out)` function for user classes.
- [ + ] Conversion sub-module: Added support for a globally defined `to_string(in)` function for user classes.
- [ + ] Implemented internal encoders for UTF-16 and UTF-32 (the previous version only had UTF-8).
- [ + ] Added support for `std::u16string` and `std::u32string` in the convert sub-module.
- [ + ] Added support for serialization of `std::u16string` and `std::u32string`, and the ability to use them as keys.
- [ + ] Added support for serialization of the C++ `nullptr` type (used for smart pointers).
- [ + ] Added support for serialization of `std::optional`, `std::unique_ptr` and `std::shared_ptr`.
- [ + ] Added new samples "string_conversions", "serialize_custom_string" and "serialize_map_to_yaml".
- [ + ] Added documentation for [string conversion submodule](docs/bitserializer_convert.md).
- [ * ] Removed stream operators for enums and classes; they must now be explicitly declared via the `DECLARE_ENUM_STREAM_OPS` macro.
- [ * ] Updated compatibility with the new version of the RapidYaml library (v0.1.0).
- [ * ] Fixed handling of YAML parse errors (previously the RapidYaml library called `abort()`).
- [ * ] Fixed error handling when loading incompatible types in the PugiXml archive.
- [ * ] Fixed saving temporary strings in the RapidJson archive.
- [ * ] Conversion sub-module: boolean values are converted to `"true"`/`"false"` strings (cast to `<int>` if you expect `"1"`/`"0"`).

## 0.10 (1 Jun 2020)
- [ ! ] Changed the main concept from a separate library for each format to an all-in-one library with components.
- [ ! ] Changed the include paths for archives (all archive implementations are now in the `bitserializer` directory).

## 0.9 (5 May 2020)
- [ ! ] Added XML serialization support (based on library PugiXml).
- [ ! ] Added YAML serialization support (based on library RapidYaml).
- [ ! ] Added CI with builds for Windows, Linux (GCC, Clang) and macOS (AppleClang).
- [ + ] Added formatting options for output text (formatting is not supported in CppRestJson).
- [ + ] Added support for encoding to various UTF-based formats (defined in serialization options).
- [ + ] Added optional BOM writing to the output stream/file.
- [ + ] Added the ability to pretty-format output text.
- [ + ] Added UTF encoding when serializing `std::wstring`.
- [ + ] Added serialization for all STD containers that were previously missing.
- [ + ] Added serialization of the C++ union type.
- [ \* ] Split the implementation of serialization for STD types into separate files.
- [ \* ] Changed the string type for the path in the archive from `std::wstring` to `std::string` (UTF-8 encoding).
- [ \* ] For the RapidJson-based archive, the in-memory encoding was changed from UTF-16 to UTF-8.
- [ \* ] Added the path to exceptions about file I/O errors.
- [ \* ] Fixed registration of enum types not in the global namespace.
- [ \* ] Added constants with the library version.

## 0.8 (30 Nov 2018)
- [ ! ] The package for VCPKG was split into two: "bitserializer" (core without any dependencies) and "bitserializer-cpprestjson" (requires "cpprestsdk").
- [ + ] Added a new implementation for the JSON format based on the RapidJson library (currently supports only UTF-16).
- [ + ] Added validation of deserialized values.
- [ + ] Added performance test.
- [ + ] Added directory with samples.
- [ + ] Added CMake support (needed only for samples and tests, since the library is header-only).
- [ + ] Added function `MakeAutoKeyValue()` to create a key/value pair that automatically adapts the key to the target archive.
- [ \* ] Enhanced the architecture to support different kinds of formats (for example, allowing ANSI/Unicode streams in one archive).
- [ \* ] Fixed compilation issues on the latest Visual Studio 15.8.6 and GCC.
- [ \* ] Changed (unified) interface methods: `LoadObjectFromStream() -> LoadObject(), SaveObjectToStream() -> SaveObject()`.

## 0.7 (22 Mar 2018)
- [ ! ] First public release.
