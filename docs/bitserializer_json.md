### [BitSerializer](../README.md) / JSON (built-in)

> [!WARNING]
> The built-in JSON archive is available starting from the next release. The `json-archive` VCPKG feature and `with_json` Conan option are not yet published in package managers. For now, use CMake directly (see below).

Supported load/save JSON from:

- std::string: UTF-8
- std::stream: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE (auto-detection encoding with/without BOM)

This is the built-in JSON archive implementation with no external dependencies. It is about 40% faster than the RapidJSON-based implementation and offers the same functionality.

### How to install
This archive is built-in and does not require any external dependencies. For installation instructions, see [How to install](../README.md#how-to-install) in the main README. Include the header and link the archive:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/json_archive.h"
```
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::json-archive)
```

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

### JSONC (comments and trailing commas)
In addition to the strict JSON archive, the library provides the `JsoncArchive` alias, which reads JSON with
extensions commonly known as JSONC:

- `//` line comments
- `/* ... */` block comments
- a single trailing comma before a closing `]` or `}`

Comments are a read-only extension: `JsoncArchive` always writes plain JSON, so its output is identical to
`JsonArchive`.

```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/json_archive.h"

using namespace BitSerializer;
using JsoncArchive = BitSerializer::Json::JsoncArchive;

class CPoint
{
public:
    CPoint() = default;
    CPoint(const int x, const int y) : X(x), Y(y) { }

    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        archive << KeyValue("x", X);
        archive << KeyValue("y", Y);
    }

    int X = 0, Y = 0;
};

int main()
{
    std::string jsonc = R"({
        // A line comment
        "x": 10,   /* a block comment */
        "y": 20
    })";

    CPoint point;
    BitSerializer::LoadObject<JsoncArchive>(point, jsonc);

    // The output is plain JSON
    auto json = BitSerializer::SaveObject<JsoncArchive>(point);
    std::cout << json << std::endl; // {"x":10,"y":20}

    return EXIT_SUCCESS;
}
```

The `JsoncArchive` alias is a JSON dialect (advertised via `ArchiveType::Jsonc`, see `ArchiveType`)
that enables all supported extensions. Readers are instantiated per archive type rather than per feature
combination, so adding new formats (e.g. JSON5) does not multiply the set of compiled instantiations.

