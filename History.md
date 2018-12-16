# BitSerializer (History log)

#### Version 0.8 (30 November 2018):
- [ ! ] The package for VCPKG was splitted into two: "bitserializer" (core without any dependencies) and "bitserializer-cpprestjson" (requires "cpprestsdk").
- [ + ] Added new implementation for JSON format based on library RapidJson (currently supported only UTF16).
- [ + ] Added validation of deserialized values.
- [ + ] Added performance test.
- [ + ] Added directory with samples.
- [ + ] Added CMake support (it needs just for samples and tests, as the library is headers only).
- [ + ] Added function MakeAutoKeyVale() to make key/value which is able to automatically adapt key to target archive.
- [ \* ] Enhanced architecture for support different kind of formats (for example allow to implement ANSI/Unicode streams in one archive).
- [ \* ] Fixed compilation issues on latest Visual Studio 15.8.6 and GCC.
- [ \* ] Changed (unified) interface methods: LoadObjectFromStream() -> LoadObject(), SaveObjectToStream() -> SaveObject().

#### Version 0.7 (22 March 2018)
- [!] First public release.
