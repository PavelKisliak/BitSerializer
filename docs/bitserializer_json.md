### [BitSerializer](../README.md) / JSON (built-in)

> [!NOTE]
> The built-in JSON archive is included in prebuilt packages. The `json-archive` VCPKG feature and `with_json` Conan option may not be available in package managers yet; if they are not, use CMake directly (see below).

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

### Strict input
The whole input must contain exactly one JSON value. Trailing non-whitespace content is rejected
with a `ParsingException` (for `JsoncArchive`, trailing comments are allowed). This applies to both
string and stream input.
Unescaped control characters (U+0000-U+001F) inside strings are also rejected with a
`ParsingException`; they must be escaped (`\n`, `\u0000`, etc.).

### Non-finite numbers
JSON has no representation for `NaN` and `Infinity`. Saving such a value throws a
`SerializationException` with the error code `SerializationErrorCode::OutOfRange`.
If a non-finite value must be preserved, encode it explicitly (for example, as a string field).

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

Comments are a read-only extension for typed serialization: `JsoncArchive` always writes plain JSON, so its output is
identical to `JsonArchive`. The only exception is explicit payload passthrough — a raw JSONC block captured with
`JsoncArchive::raw_type` is written back verbatim (comments included).

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

#### Raw payload passthrough and JSON dialects

The raw payload type of an archive is tagged with the archive's format:

- `JsonArchive::raw_type` is `RawPayload<std::string, ArchiveType::Json>`;
- `JsoncArchive::raw_type` is `RawPayload<std::string, ArchiveType::Jsonc>`.

Because strict JSON is a subset of JSONC, a raw block captured from a strict JSON document can be embedded into a
JSONC archive, but not the other way around: a JSONC block may contain comments, which are not valid in strict JSON.
These rules are enforced at compile time by `IsRawPayloadCompatible(source, target)`:

| Raw payload format | Embedded into `JsonArchive` | Embedded into `JsoncArchive` |
|--------------------|:---------------------------:|:----------------------------:|
| `RawPayload<..., Json>`  | ✅ | ✅ |
| `RawPayload<..., Jsonc>` | ❌ (compile-time error) | ✅ (comments preserved) |

Reading a raw block from a JSONC document always produces a `JsoncArchive::raw_type` value, so it can never be
accidentally written into a strict JSON document.

