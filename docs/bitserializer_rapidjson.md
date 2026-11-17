### [BitSerializer](../README.md) / JSON (based on RapidJson)

> [!WARNING]
> This archive is deprecated. Use the built-in [`BitSerializer::Json::JsonArchive`](bitserializer_json.md) from `bitserializer/json_archive.h` instead. The RapidJSON-based archive remains available during the transition period.

Supported load/save JSON from:

- std::string: UTF-8 (without BOM)
- std::stream: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE (auto-detection encoding with/without BOM)

String input is treated as already-decoded text; use streams for BOM/other encodings
(see [Input and output data types](../README.md#input-and-output-data-types)).

This implementation of JSON archive is based on [RapidJson](https://github.com/Tencent/rapidjson) which is one of fastest libraries for parse **JSON**.

### Migration to the built-in archive

The built-in JSON archive has the same `LoadObject`/`SaveObject` interface, so most user code only needs a different include, namespace and build target:

| RapidJSON-based archive | Built-in archive |
|---|---|
| `#include "bitserializer/rapidjson_archive.h"` | `#include "bitserializer/json_archive.h"` |
| `BitSerializer::Json::RapidJson::JsonArchive` | `BitSerializer::Json::JsonArchive` |
| `BUILD_RAPIDJSON_ARCHIVE` | `BUILD_JSON_ARCHIVE` |
| `BitSerializer::rapidjson-archive` | `BitSerializer::json-archive` |
| CMake component `rapidjson` | CMake component `json` |
| `raw_type` is a RapidJSON DOM document | `raw_type` is `RawPayload<std::string, ArchiveType::Json>` |

Behavioral differences to consider when migrating:

- `NaN` and `Infinity` cannot be saved by the built-in archive; saving them throws `SerializationException` with `SerializationErrorCode::OutOfRange`.
- Raw payloads are captured as JSON text rather than a mutable DOM. If the previous code inspected or modified the DOM, parse the payload explicitly or model it as regular C++ types.

The built-in archive follows RFC 8259 strictly; the RapidJSON archive is also strict by default, so parsing of ordinary JSON should not change. JSONC (comments and trailing commas) is available via the separate `JsoncArchive`.

### How to install
This archive is based on the third-party [RapidJson](https://github.com/Tencent/rapidjson) library. For installation instructions, see [How to install](../README.md#how-to-install) in the main README. Include the header and link the archive:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
```
```cmake
find_package(bitserializer CONFIG REQUIRED COMPONENTS rapidjson)
target_link_libraries(main PRIVATE BitSerializer::rapidjson-archive)
```

Legacy package-manager options remain available during the transition period: enable `rapidjson-archive` in VCPKG or set `with_rapidjson=True` in Conan.

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
As base library (RapidJson) has the functionality for output to human readable format, the BitSerializer also allows to do this:
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/rapidjson_archive.h"

using namespace BitSerializer;
using namespace BitSerializer::Json::RapidJson;

class Point
{
public:
    Point(const int x, const int y) : X(x), Y(y) { }

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
    std::vector<Point> points = { Point(10, 20), Point(30, 40) };

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