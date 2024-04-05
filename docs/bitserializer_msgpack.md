### [BitSerializer](../README.md) / Message pack

### Features

 - The most efficient and fastest format supported by BitSerializer
 - Fully compliant with the MsgPack specification, including `Timestamp` extension type(\*)
 - Serialization from/to memory and `std::stream`
 - Supports loading object fields in any order with conditions (for version control)
 - No third party dependencies

(\*) Except using not-trivial types as keys for map (like `object`, `array`, `raw`, `nil`, etc), but the `Timestamp` extension is allowed.

### How to install
The MsgPack archive does not require any third party dependencies, but since this part is not "header only", it needs to be built. Currently library supports only static linkage. The recommended way is to use one of supported package managers, but you can do it manually just via CMake commands.
For avoid binary incompatibility issues, please build with the same compiler options that are used in your project (C++ standard, optimizations flags, runtime type, etc).
#### VCPKG (not published yet)
Add BitSerializer to manifest file (`vcpkg.json`) with `msgpack-archive` feature:
```json
{
    "dependencies": [
        {
            "name": "bitserializer",
            "features": [ "msgpack-archive" ]
        }
    ]
}
```
If your project is based on VS solution you can just include next header files for start use:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/msgpack_archive.h"
```
If you are using CMake, you need to link the library:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::msgpack-archive)
```
#### Conan (not published yet)
Add the BitSerializer recipe to `conanfile.txt` in your project and enable `with_msgpack` option:
```
[requires]
bitserializer/0.50

[options]
bitserializer:with_msgpack=True
```
Usage the library will be related to selected Conan generator, if your choice is `cmake_find_package_multi`, than linking will be classic:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::msgpack-archive)
```
#### CMake install to Unix system
```sh
$ git clone https://github.com/PavelKisliak/BitSerializer.git
$ cmake bitserializer -B bitserializer/build -DBUILD_MSGPACK_ARCHIVE=ON
$ sudo cmake --build bitserializer/build --config Debug --target install
$ sudo cmake --build bitserializer/build --config Release --target install
```
#### CMake install to your project directory
You can install BitSerializer to your "ThirdParty" directory in your project.
Set correct path in `%TargetInstallDir%` (for example 'D:/MyProject/libs/bitserializer') before run.
```shell
> git clone https://Pavel_Kisliak@bitbucket.org/Pavel_Kisliak/bitserializer.git
> cmake bitserializer -B bitserializer/build -DCMAKE_INSTALL_PREFIX:PATH=%TargetInstallDir% -DBUILD_MSGPACK_ARCHIVE=ON
> sudo cmake --build bitserializer/build --config Debug --target install
> sudo cmake --build bitserializer/build --config Release --target install
```
You will need to explicitly specify the path where to find the library:
```cmake
find_package(bitserializer CONFIG REQUIRED
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/libs/bitserializer
    NO_DEFAULT_PATH)
target_link_libraries(main PRIVATE BitSerializer::msgpack-archive)
```
