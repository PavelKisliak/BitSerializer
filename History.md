# BitSerializer (History log)

##### What's new in version 0.50 (5 December 2022):
- [ ! ] Added new archive for serialization to CSV, supports all UTF encodings with auto-detection (built-in implementation, no dependencies).
- [ ! ] API breaking change - deprecated global `BitSerializer::Context`, now validation errors will propagate only via `ValidationException`.
- [ ! ] Removed all static memory allocations for be compatible with custom allocators.
- [ + ] Added policy `OverflowNumberPolicy` for case when size of target type is not enough for loading number.
- [ + ] Added policy `MismatchedTypesPolicy` for case when type of target field does not match the value being loaded.
- [ + ] Added default `SerializationOptions`.
- [ * ] Added `ParsingException` with information about line number or offset (depending on format type).
- [ * ] Added new simplified macro `REGISTER_ENUM` - replacement for `REGISTER_ENUM_MAP` which is deprecated.
- [ + ] Conversion sub-module: Added error policy for encode UTF (error mark, throw exception or skip).
- [ * ] Conversion sub-module: Added throwing `invalid_argument` exception when converting from invalid string to number.
- [ * ] Conversion sub-module: Converting a string containing floating point number to integer, now will throw `out_of_range` exception.
- [ * ] Conversion sub-module: Fixed work with raw pointers in the UTF-16Be and UTF-32Be encoders.
- [ * ] Conversion sub-module: Fixed macro `DECLARE_ENUM_STREAM_OPS` (can't be used in namespaces).
- [ * ] [CppRestJson] Fixed serialization of booleans in the object (was serialized as number).
- [ * ] [RapidYaml] Fixed compatibility with latest version of the RapidYaml library (0.4.1).
- [ * ] [RapidYaml] Fixed serialization negative int8.
- [ * ] [RapidYaml] Fixed issue with error handling when multi-thread serialization.
- [ * ] [RapidJson, CppRestJson, RapidYaml] Fixed path in the validation errors, index in arrays was shifted by 1.

##### What's new in version 0.44 (1 October 2021):

- [ ! ] Changed minimum requirement for CLang compiler from version 7 to 8.
- [ ! ] API breaking change - global `Serialize()` function should now return `bool`.
- [ ! ] API breaking change - validation messages now storing in UTF-8, function GetValidationErrors() returns vector of `std::string`.
- [ ! ] Simplified implementing non-intrusive serialization, now only one global function should be defined (compatibility is preserved).
- [ ! ] Internal string conversion method `FromString()` now should have as argument any of `std::string_view` types instead of `std::string`.
- [ - ] Internal string conversion method `ToWString()` was deprecated, please implement `ToU16String()` and/or `ToU32String()` when you needed.
- [ + ] Conversion sub-module: added support for globally defined function `To(in, out)` for user's classes.
- [ + ] Conversion sub-module: added support for globally defined function `to_string(in)` for user's classes.
- [ + ] Implemented internal encoders for UTF-16 and UTF-32 (in the previous version there was only UTF-8).
- [ + ] Added support for `std::u16string` and `std::u32string` in the convert sub-module.
- [ + ] Added support serialization of `std::u16string` and `std::u32string` and ability to use them as keys.
- [ + ] Added support serialization for C++ nullptr type (uses for serialization smart pointers).
- [ + ] Added support serialization for std::optional type, std::unique_ptr and std::shared_ptr.
- [ + ] Added new samples "string_conversions", "serialize_custom_string" and "serialize_map_to_yaml".
- [ + ] Added documentation for [string conversion submodule](docs/bitserializer_convert.md).
- [ * ] Removed streams operators for enum and classes, now need to explicitly declare them via macro DECLARE_ENUM_STREAM_OPS.
- [ * ] Update compatibility with new version of RapidYaml library v.0.1.0.
- [ * ] Fixed handling YAML parse errors (previously was called abort() in the RapidYaml library).
- [ * ] Fixed handling errors when loading incompatible types in PugiXml archive.
- [ * ] Fixed saving temporary strings in RapidJson archive.
- [ * ] Conversion sub-module: boolean type will be converted to "true|false" strings (please cast to <int> if you expect "1|0").

##### Version 0.10 (1 June 2020):

- [ ! ] Changed main concept with separate library for each format to all-in-one library with components.
- [ ! ] Changed include paths for archives (all archive implementations are now in the "bitserializer" directory).

##### Version 0.9 (5 May 2020):
- [ ! ] Added XML serialization support (based on library PugiXml).
- [ ! ] Added YAML serialization support (based on library RapidYaml).
- [ ! ] Add CI with builds for Windows, Linux (GCC, Clang) and MacOS (AppleClang).
- [ + ] Add formatting options for output text (but formatting is not supported in CppRestJson).
- [ + ] Add support encoding to various UTF based formats (defines in serialization options).
- [ + ] Add optional writing the BOM to output stream/file.
- [ + ] Add ability for pretty format of output text.
- [ + ] Add UTF encoding when serializing std::wstring.
- [ + ] Add serialization for all STD containers which were missed before.
- [ + ] Add serialization C++ union type.
- [ \* ] Split implementation of serialization for std types into separate files.
- [ \* ] Change string type for path in archive from std::wstring to std::string (in UTF-8 encoding).
- [ \* ] For archive based on RapidJson was changed in-memory encoding from UTF-16 to UTF-8.
- [ \* ] Add path into exceptions about I/O errors with files.
- [ \* ] Fix registration enum types not in global namespace.
- [ \* ] Add constants with library version.

#### Version 0.8 (30 November 2018):
- [ ! ] The package for VCPKG was split into two: "bitserializer" (core without any dependencies) and "bitserializer-cpprestjson" (requires "cpprestsdk").
- [ + ] Added new implementation for JSON format based on library RapidJson (currently supported only UTF16).
- [ + ] Added validation of deserialized values.
- [ + ] Added performance test.
- [ + ] Added directory with samples.
- [ + ] Added CMake support (it needs just for samples and tests, as the library is headers only).
- [ + ] Added function `MakeAutoKeyValue()` to make key/value which is able to automatically adapt key to target archive.
- [ \* ] Enhanced architecture for support different kind of formats (for example allow to implement ANSI/Unicode streams in one archive).
- [ \* ] Fixed compilation issues on latest Visual Studio 15.8.6 and GCC.
- [ \* ] Changed (unified) interface methods: `LoadObjectFromStream() -> LoadObject(), SaveObjectToStream() -> SaveObject()`.

#### Version 0.7 (22 March 2018)
- [!] First public release.
