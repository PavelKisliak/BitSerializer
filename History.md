# BitSerializer (History log)

#### Version 0.8 (10 October 2018):

[!] The package for VCPKG was splitted into two: "bitserializer" (core without any dependencies) and "bitserializer-json-restcpp" (requires "cpprestsdk").
[+] Add CMake support (it needs just for samples and tests, as the library is headers only).
[+] Add validation of deserialized values.
[+] Added directory with samples.
[\*] Enhanced architecture for support different kind of formats (for example allow to implement ANSI/Unicode streams in one archive).
[\*] Fixed compilation issues on latest Visual Studio 15.8.6 and GCC.
[\*] Changed (unified) interface methods: LoadObjectFromStream() -> LoadObject(), SaveObjectToStream() -> SaveObject().

#### Version 0.7 (22 March 2018)
[!] First public release.
