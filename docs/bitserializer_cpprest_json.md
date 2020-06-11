### [BitSerializer](../README.md) / JSON (based on C++ REST SDK)

Supported load/save JSON from:

- std::string: UTF-8
- std::stream: UTF-8 (with/without BOM)

This implementation of JSON archive is based on [C++ REST SDK](https://github.com/Microsoft/cpprestsdk) - one of most powerful library for work with HTTP.

### How to install
The recommended way is to use one of supported package managers, but you can do it manually via Cmake install command (in this case you should care about dependencies by yourself).
#### VCPKG
**Note:** Adding version 0.10 to VCPKG still [in progress](https://github.com/microsoft/vcpkg/pull/11683) (available only version 0.9).
```shell
vcpkg install bitserializer[cpprestjson-archive]:x64-windows
```
Specify required triplet for your platform instead of "x64-windows".
If your project is based on VS solution you can just include next header files for start use:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/cpprestjson_archive.h"
```
If you are using Cmake, you need to link the library:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::cpprestjson-archive)
```

### Implementation detail
One of the important things related to JSON implementations in CppRestSdk is character dimension on different platforms - for Windows it's 16-bit (UTF-16), for all other - 8-bit char (UTF-8). Accordingly, this affects the key type in BitSerializer, and usages of the universal adapter **MakeAutoKeyValue()** is already becoming very relevant for this archive (if you want to make your code cross platform). One more option - usages of macros from CppRestSdk: **_XPLATSTR("MyKey")**, but in this case you will lost compatibility with other type of BitSerializer archives.

The JSON specification allows to store on root not just objects and arrays, but also more primitive types such as string, number and boolean.
The BitSerializer also supports this abilities, have a look to [Hello world example](../samples/hello_world/hello_world.cpp).
