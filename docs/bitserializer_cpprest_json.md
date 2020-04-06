### [BitSerializer](../README.md) / JSON

Supported load/save JSON from:

- std::string: UTF-8
- std::stream: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE (with/without BOM)

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
