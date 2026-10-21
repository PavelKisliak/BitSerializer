### [BitSerializer](../README.md) / YAML (based on RapidYAML)

Supported load/save **YAML** from:

- std::string: UTF-8
- std::stream: UTF-8 (with/without BOM)

This implementation of **YAML** archive is based on [RapidYAML](https://github.com/biojppm/rapidyaml), which shows good performance in comparison with **YamlCpp**.

### How to install
This archive is based on the third-party [RapidYAML](https://github.com/biojppm/rapidyaml) library. For installation instructions, see [How to install](../README.md#how-to-install) in the main README. Include the header and link the archive:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidyaml_archive.h"
```
```cmake
find_package(bitserializer CONFIG REQUIRED COMPONENTS rapidyaml)
target_link_libraries(main PRIVATE BitSerializer::rapidyaml-archive)
```

### Implementation detail
One of unique features in **YAML** is **dictionaries**, they are a little more advanced than **JSON**, they allow to make a sequence with named objects, for example:
```yaml
- shape1: { type: square, width: 100, height: 100 }
- shape2: { type: circle, radius: 50 }
```
Such data cannot be represented with a plain object (a mapping), because it preserves the order of elements and allows duplicate keys. This is also the default representation for `std::multimap` / `std::unordered_multimap`, and it can be applied to any associative container via the `MapAsDictionary` wrapper:
```cpp
#include "bitserializer/types/std/map.h"

std::map<std::string, CShape> shapes;
BitSerializer::LoadObject<YamlArchive>(BitSerializer::MapAsDictionary(shapes), sourceYaml);
BitSerializer::SaveObject<YamlArchive>(BitSerializer::MapAsDictionary(shapes), outputYaml);
```
The default representation of a map (a plain mapping of key-value fields) is available as well, and can be loaded with an explicit mode via `MapAsObject`:
```cpp
BitSerializer::LoadObject<YamlArchive>(BitSerializer::MapAsObject(shapes, BitSerializer::MapLoadMode::UpdateExisting), sourceYaml);
```
The load mode can also be configured globally via `SerializationOptions::mapOptions` (available values: `Replace`, `UpdateExisting` or `InsertOrUpdate`).

In addition, there are several limitations related to the implementation of the underlying library:

- **Rapid YAML** does not support streams, BitSerializer handle this, but with memory overhead.

### Example
Below is a short example of loading a YAML dictionary into a `std::map` and saving it back:
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidyaml_archive.h"
#include "bitserializer/types/std/map.h"

using namespace BitSerializer;
using YamlArchive = BitSerializer::Yaml::RapidYaml::YamlArchive;

class Device
{
public:
    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        archive << KeyValue("IP", mIp);
        archive << KeyValue("Owner", mOwner);
    }

    std::string mIp;
    std::string mOwner;
};

int main()
{
    const char* srcYaml = R"(
- Tablet:  { IP: 192.168.0.1, Owner: Artsiom }
- Desktop: { IP: 192.168.0.2, Owner: Pavel }
)";

    // Loading a dictionary (a sequence of single-key objects)
    std::map<std::string, Device> devices;
    BitSerializer::LoadObject<YamlArchive>(BitSerializer::MapAsDictionary(devices), srcYaml);

    // Save back to the same representation
    std::string outputYaml;
    BitSerializer::SaveObject<YamlArchive>(BitSerializer::MapAsDictionary(devices), outputYaml);
    std::cout << outputYaml << std::endl;

    return 0;
}
```
[See full sample](../samples/serialize_map/serialize_map.cpp)
