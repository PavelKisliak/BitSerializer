### [BitSerializer](../README.md) / YAML (based on RapidYAML)

Supported load/save **YAML** from:

- std::string: UTF-8
- std::stream: UTF-8 (with/without BOM)

This implementation of **YAML** archive is based on [RapidYAML](https://github.com/biojppm/rapidyaml), which shows good performance in comparison with **YamlCpp**.

### How to install
The recommended way is to use one of supported package managers, but you can do it manually via Cmake install command (in this case you should take care of the dependencies yourself).
#### VCPKG
Add BitSerializer to manifest file (`vcpkg.json`) with `rapidyaml-archive` feature:
```json
{
    "dependencies": [
        {
            "name": "bitserializer",
            "features": [ "rapidyaml-archive" ]
        }
    ]
}
```
If your project is based on VS solution you can just include next header files for start use:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidyaml_archive.h"
```
If you are using CMake, you need to link the library:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::rapidyaml-archive)
```
#### Conan
Add the BitSerializer recipe to `conanfile.txt` in your project and enable `with_csv` option:
```
[requires]
bitserializer/0.65

[options]
bitserializer:with_rapidyaml=True
```
Usage the library will be related to selected Conan generator, if your choice is `cmake_find_package_multi`, than linking will be classic:
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
