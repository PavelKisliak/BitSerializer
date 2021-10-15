### [BitSerializer](../README.md) / JSON (based on RapidJson)

Supported load/save JSON from:

- std::string: UTF-8
- std::stream: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE (with/without BOM)

This implementation of JSON archive is based on [RapidJson](https://github.com/Tencent/rapidjson) which is one of fastest libraries for parse **JSON**.

### How to install
The recommended way is to use one of supported package managers, but you can do it manually via Cmake install command (in this case you should care about dependencies by yourself).
#### VCPKG
Add BitSerializer to manifest file (`vcpkg.json`) with `rapidjson-archive` feature:
```json
{
    "dependencies": [
        {
            "name": "bitserializer",
            "features": [ "rapidjson-archive" ],
            "version>=": "0.44"
        }
    ]
}
```
If your project is based on VS solution you can just include next header files for start use:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
```
If you are using Cmake, you need to link the library:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::rapidjson-archive)
```
#### Conan
Add the BitSerializer recipe to `conanfile.txt` in your project and enable `with_rapidjson` option:
```
[requires]
bitserializer/0.44

[options]
bitserializer:with_rapidjson=True
```
The dependent library **RapidJson** will be automatically installed.

### Implementation detail
The JSON specification allows to store on root not just objects and arrays, but also more primitive types such as string, number and boolean.
The BitSerializer also supports this abilities, have a look to [Hello world example](../samples/hello_world/hello_world.cpp).

### Pretty format
As base library (RapidJson) has the functionality for output to human readable format, the BitSerializer also allows to do this:
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/rapidjson_archive.h"

using namespace BitSerializer;
using namespace BitSerializer::Json::RapidJson;

class CPoint
{
public:
	CPoint(const int x, const int y) : X(x), Y(y) { }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeKeyValue("x", X);
		archive << MakeKeyValue("y", Y);
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