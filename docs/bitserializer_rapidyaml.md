### [BitSerializer](../README.md) / YAML (based on RapidYAML)

Supported load/save JSON from:

- std::string: UTF-8
- std::stream: UTF-8 (with/without BOM)

This implementation of **YAML** archive is based on [RapidYAML](https://github.com/biojppm/rapidyaml), which shows good performance in comparison with **YamlCpp**.

### How to install
The recommended way is to use one of supported package managers, but you can do it manually via Cmake install command (in this case you should care about dependencies by yourself).
#### VCPKG
**Note:** Adding version 0.10 to VCPKG still [in progress](https://github.com/microsoft/vcpkg/pull/11683) (available only version 0.9).
```shell
vcpkg install bitserializer[rapidyaml-archive]:x64-windows
```
Specify required triplet for your platform instead of "x64-windows".
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

### Implementation detail
Exists some issues which are related to base library implementation:

- Error handling is not thread-safe as structure `ryml::Callbacks` is defined globally.
- **Rapid YAML** does not support streams, BitSerializer handle this, but with memory overhead.
- MacOS does not supported, as the base library does not support too.

Hope that author of [RapidYAML](https://github.com/biojppm/rapidyaml) will take care about it in future.
