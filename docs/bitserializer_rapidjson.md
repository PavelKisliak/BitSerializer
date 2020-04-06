### [BitSerializer](../README.md) / JSON (based on RapidJson)

Supported load/save JSON from:

- std::string: UTF-8
- std::stream: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE (with/without BOM)

This implementation of JSON archive is based on [RapidJson](https://github.com/Tencent/rapidjson) which is one of fastest libraries for parse **JSON**.

#### How to install (VCPKG)
```shell
vcpkg install bitserializer-rapidjson:x64-windows
```
Specify required triplet for your platform instead of "x64-windows".
After installation packages just include next files:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"
```

### Implementation detail
The JSON specification allows to store on root not just objects and arrays, but also more primitive types such as string, number and boolean.
The BitSerializer also supports this abilities, have a look to [Hello world example](../samples/hello_world/hello_world.cpp).

### Pretty format
As base library (RapidJson) has the functionality for output to human readable format, the BitSerializer also allows to do this:
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"

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