### [BitSerializer](../README.md) / JSON (based on C++ REST SDK)

Supported load/save JSON from:

- std::string: UTF-8
- std::stream: UTF-8 (with/without BOM)

This implementation of JSON archive is based on [C++ REST SDK](https://github.com/Microsoft/cpprestsdk) - one of most powerful library for work with HTTP.

#### How to install (VCPKG)
```shell
vcpkg install bitserializer-cpprestjson:x64-windows
```
Specify required triplet for your platform instead of "x64-windows".
Include next files:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer_cpprest_json/cpprest_json_archive.h"
```

### Implementation detail
One of the important things related to JSON implementations in CppRestSdk is character dimension on different platforms - for Windows it's 16-bit (UTF-16), for all other - 8-bit char (UTF-8). Accordingly, this affects the key type in BitSerializer, and usages of the universal adapter **MakeAutoKeyValue()** is already becoming very relevant for this archive (if you want to make your code cross platform). One more option - usages of macros from CppRestSdk: _XPLATSTR("MyKey"), but in this case you will lost compatibility with other type of BitSerializer archives.

The JSON specification allows to store on root not just objects and arrays but also more primitive types such as string, number and boolean.
The BitSerializer also supports this abilities, have a look to [Hello world example](../samples/hello_world/hello_world.cpp).
