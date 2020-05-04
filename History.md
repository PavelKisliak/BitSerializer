# BitSerializer (History log)

##### Version 0.9 (5 May 2020):
- [ ! ] Added XML serialization support (based on library PugiXml).
- [ ! ] Added YAML serialization support (based on library RapidYaml).
- [ ! ] Add CI with builds for Windows, Linux (GCC, Clang) and MaOS (AppleClang).
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
