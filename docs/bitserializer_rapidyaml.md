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
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::rapidyaml-archive)
```

### Implementation detail
One of unique features in **YAML** is **dictionaries**, they are a little more advanced than **JSON**, they allow to make a sequence with named objects, for example:
```yaml
- shape1: { type: square, width: 100, height: 100 }
- shape2: { type: circle, rarius: 50 }
```
Currently `BitSerializer` does not support loading such data, it is only possible to serialize dictionaries only as object:
```yaml
shape1: { type: square, width: 100, height: 100 }
shape2: { type: circle, rarius: 50 }
```

In addition, there are several limitations related to the implementation of the underlying library:

- **Rapid YAML** does not support streams, BitSerializer handle this, but with memory overhead.

### Example
Below example shows how to load and save `std::map` from/to **YAML**.
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidyaml_archive.h"
#include "bitserializer/types/std/map.h"

using namespace BitSerializer;
using YamlArchive = BitSerializer::Yaml::RapidYaml::YamlArchive;

class СDevice
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
    const char* srcStr = R"(
Tablet:
   IP: 192.168.0.1
   Owner: Artsiom
Desktop:
   IP: 192.168.0.2
   Owner: Pavel
Laptop:
   IP: 192.168.0.3
   Owner: Alex
)";

    // Loading from YAML
    std::map<std::string, СDevice> devices;
    BitSerializer::LoadObject<YamlArchive>(devices, srcStr);

    std::cout << "Loaded devices list from YAML: " << std::endl;
    for (const auto& device : devices)
    {
        std::cout << "Type: " << device.first
            << "\tIP: " << device.second.mIp
            << "\tOwner: " << device.second.mOwner
            << std::endl;
    }

    // Save back to YAML
    std::string outputStr;
    BitSerializer::SaveObject<YamlArchive>(devices, outputStr);
    std::cout << std::endl << "Saved result in YAML:" << std::endl << outputStr << std::endl;

    return 0;
}
```
[See full sample](../samples/serialize_map_to_yaml/serialize_map_to_yaml.cpp)
