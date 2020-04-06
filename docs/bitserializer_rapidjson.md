### [BitSerializer](../README.md) / JSON

Supported load/save JSON from:

- std::string: UTF-8
- std::stream: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE (with/without BOM)

This implementation of JSON archive is based on [RapidJson](https://github.com/Tencent/rapidjson) which is one of fastest libraries for parse **JSON**.

#### How to install (VCPKG)
```shell
vcpkg install bitserializer-rapidjson:x64-windows
```
Specify required triplet for your platform instead of "x64-windows".
Include next files:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"
```

### Implementation detail
