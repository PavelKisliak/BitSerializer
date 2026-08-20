### [BitSerializer](../README.md) / JSON (built-in)

> [!WARNING]
> The built-in JSON archive is available starting from the next release. The `json-archive` VCPKG feature and `with_json` Conan option are not yet published in package managers. For now, use CMake directly (see below).

Supported load/save JSON from:

- std::string: UTF-8
- std::stream: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE (auto-detection encoding with/without BOM)

This is the built-in JSON archive implementation with no external dependencies. It is about 40% faster than the RapidJSON-based implementation and offers the same functionality.

### How to install
The recommended way is to use one of supported package managers, but you can do it manually just via CMake commands.
#### VCPKG
Add BitSerializer to manifest file (`vcpkg.json`) with `json-archive` feature:
```json
{
    "dependencies": [
        {
            "name": "bitserializer",
            "features": [ "json-archive" ]
        }
    ]
}
```
The latest available version: [![Vcpkg Version](https://img.shields.io/vcpkg/v/bitserializer?color=blue)](https://vcpkg.link/ports/bitserializer)

If your project is based on VS solution you can just include next header files for start use:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/json_archive.h"
```
If you are using CMake, you need to link the library:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::json-archive)
```
#### Conan 2
Add the BitSerializer recipe to `conanfile.txt` in your project and enable `with_json` option:
```
[requires]
bitserializer/x.xx

[options]
bitserializer/*:with_json=True
```
Replace `x.xx` with the latest available version: [![Conan Center](https://img.shields.io/conan/v/bitserializer?color=blue)](https://conan.io/center/recipes/bitserializer)

No external dependencies required.

### Implementation detail
The JSON specification allows storing not only objects and arrays in the root, but also more primitive types such as string, number, and boolean.
This is also not a problem for BitSerializer:
```cpp
int main()
{
    std::string expected = "Hello world!";
    auto json = BitSerializer::SaveObject<JsonArchive>(expected);

    std::string result;
    BitSerializer::LoadObject<JsonArchive>(result, json);

    assert(result == expected);
    std::cout << result << std::endl;

    return EXIT_SUCCESS;
}
```

### Pretty format
The built-in JSON archive supports output to human readable format:
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/json_archive.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::JsonArchive;

class CPoint
{
public:
    CPoint(const int x, const int y) : X(x), Y(y) { }

    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        archive << KeyValue("x", X);
        archive << KeyValue("y", Y);
    }

    int X, Y;
};

int main()
{
    std::vector<CPoint> points = { CPoint(10, 20), CPoint(30, 40) };

    SerializationOptions serializationOptions;
    serializationOptions.formatOptions.enableFormat = true;
    serializationOptions.formatOptions.paddingChar = ' ';
    serializationOptions.formatOptions.paddingCharNum = 2;

    std::string result;
    BitSerializer::SaveObject<JsonArchive>(points, result, serializationOptions);
    std::cout << result << std::endl;

    return 0;
}
```
This code outputs to the console:
```json
[
  {
    "x": 10,
    "y": 20
  },
  {
    "x": 30,
    "y": 40
  }
]
```
