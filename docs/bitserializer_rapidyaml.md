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
            "features": [ "rapidyaml-archive" ],
            "version>=": "0.44"
        }
    ]
}
```
If your project is based on VS solution you can just include next header files for start use:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidyaml_archive.h"
```
If you are using Cmake, you need to link the library:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::rapidyaml-archive)
```
#### Conan
The **YAML** archive requires **RapidYaml** library, but unfortunately it is not available in the Conan right now.

### Implementation detail
One of unique features in **YAML** is **dictionaries**, are a little more advanced than **JSON**, it allows to make sequence with named objects, for example:
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

- Error handling is not thread-safe as structure `ryml::Callbacks` is defined globally.
- **Rapid YAML** does not support streams, BitSerializer handle this, but with memory overhead.
- MacOS does not supported, as the base library does not support too.

Hope that author of [RapidYAML](https://github.com/biojppm/rapidyaml) will take care about it in future.

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
		archive << MakeAutoKeyValue("IP", mIp);
		archive << MakeAutoKeyValue("Owner", mOwner);
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
