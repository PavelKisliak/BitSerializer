### [BitSerializer](../README.md) / Message pack (MsgPack)

### Features

 - The most efficient and fastest format supported by BitSerializer
 - Fully compliant with the MsgPack specification, including `Timestamp` extension type(\*)
 - Serialization from/to memory and `std::stream`
 - Supports loading object fields in any order with conditions (for version control)
 - No third party dependencies

(\*) Except using non-trivial types as keys for map (like `object`, `array`, `raw`, `nil`, etc), but the `Timestamp` extension is allowed.

### MsgPack vs JSON
MsgPack archive is fully JSON compatible, you can easily switch between them or support both in your application.
Because JSON is human-readable, it's well suited for debugging purposes, but using MsgPack in the production will help to get better performance and save traffic.

MsgPack has some advanced features which allows to make serialized data more compact:
- All numbers are stored in the binary format
- Any type is allowed as object keys (JSON supports only strings)
- Support for binary arrays such as `std::vector<char>`
- Binary representation of timestamps (`std::time_t`, `std::chrono::time_point` and `std::chrono::duration`)

All of them don't break compatibility with JSON, BitSerialzer stores data in the most suitable format for target archive.

The table below shows several examples of serialized data in JSON and MsgPack:

| Archive | Serialized data  | Size
| -------: | :------- | :------- |
|JSON|[0,10,20,30,40,50,60,70,80,90]| 30 |
|MsgPack|C4 0A 00 0A 14 1E 28 32 3C 46 50 5A| 12 |
|JSON|{"2044-01-01T00:30:00Z":"Event1","2044-01-01T01:05:00Z":"Event2"}| 65 |
|MsgPack |82 D6 FF 8B 30 A5 08 A6 45 76 65 6E 74 31 D6 FF 8B 30 AD 3C A6 45 76 65 6E 74 32| 27 |
|JSON|{"string":"Hello world!","integer":1925,"boolean":true,"floating":3.141592654,"array":[100,200,300,400,500]}| 108 |
|MsgPack|85 A6 73 74 72 69 6E 67 AC 48 65 6C 6C 6F 20 77 6F 72 6C 64 21 A7 69 6E 74 65 67 65 72 D1 07 85 A7 62 6F 6F 6C 65 61 6E C3 A8 66 6C 6F 61 74 69 6E 67 CB 40 09 21 FB 54 52 45 50 A5 61 72 72 61 79 95 64 D1 00 C8 D1 01 2C D1 01 90 D1 01 F4| 79 |

MsgPack allows to save about 35% of traffic on mixed data and significantly more when it is necessary to transfer binary arrays.<br>
MsgPack stores numbers in the binary format, that making serialization is up to 3 times faster 🚀.

[See source code](../samples/msgpack_vs_json/msgpack_vs_json.cpp)

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
bitserializer/0.65

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

### Samples
The following two examples are designed specially to demonstrate MsgPack:
- [MsgPack vs JSON](../samples/msgpack_vs_json/msgpack_vs_json.cpp) - demonstrates the features of MsgPack
- [Versioning](../samples/versioning/versioning.cpp) - demonstrates how to implement versioning.
All other examples may also be useful, as serialization to MsgPack is similar to other archives, please learn them with using main [README.md](../README.md).
