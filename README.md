# BitSerializer

The library is designed for simple serialization of arbitrary C++ types to various output formats.

This is the first version open for public access, currently it includes support for only one JSON format, which requires an external C ++ REST SDK library. The purpose of creating this library was to simplify the serialization of data for the http server. 

#### Main features:
  - Support for different formats (currently only JSON).
  - Produces a clear JSON, which is convenient to use with Javascript.
  - Checking at compile time the permissibility of saving types depending on the structure of the output format.
  - Simple syntax (similar to serialization in boost library).
  - Support for serialization of most STL containers.
  - Support for serialization of enum types (registration of a names map is required).
  - As a bonus, the subsystem of converting strings to / from arbitrary types.

#### Supported Formats:
  - JSON (the implementation is based on the [C++ REST SDK](https://github.com/Microsoft/cpprestsdk)).

#### Requirements:
  - C++ 17
  - [C++ REST SDK](https://github.com/Microsoft/cpprestsdk)

#### How to use:
The library contains only header files, need to include main file which implements serialization and file, wich implements required format (JSON for example). For install dependencies for formats (like C++ REST SDK), please follow instructions for these libraries.
```cpp
#include "bitserializer\bit_serializer.h"
#include "bitserializer\archives\json_restcpp_archive.h"
```


## Examples of using

#### Hello world!
```cpp
#include "bitserializer\bit_serializer.h"
#include "bitserializer\archives\json_restcpp_archive.h"

using namespace BitSerializer;

int main()
{
	// Supported serialization of ansi and wide strings
	std::string str = "Hello world!";
	auto json = SaveObject<JsonArchive>(str);
	str.clear();
	LoadObject<JsonArchive>(str, json);
	assert(str == "Hello world!");
	return 0;
}
```

#### Save std::map
Due to the fact that the map key is used as a key in JSON, it must be convertible to a string (by default supported all of fundamental types), if you want to use your own class as a key, you can add conversion methods to it. You also can implement specialized Serialize() method in extreme cases.
```cpp
std::map<std::string, int> testMap = 
	{ { "One", 1 },{ "Two", 2 },{ "Three", 3 },{ "Four", 4 },{ "Five", 5 } };
auto jsonResult = BitSerializer::SaveObject<JsonArchive>(testMap);
```
Returns result
```json
{
	"Five": 5,
	"Four": 4,
	"One": 1,
	"Three": 3,
	"Two": 2
}
```
#### Loading a vector of maps
Input JSON
```json
[{
	"One": 1,
	"Three": 3,
	"Two": 2
}, {
	"Five": 5,
	"Four": 4
}]
```
Code:
```cpp
std::vector<std::map<std::string, int>> testVectorOfMaps;
const std::wstring inputJson = L"[{\"One\":1,\"Three\":3,\"Two\":2},{\"Five\":5,\"Four\":4}]";
BitSerializer::LoadObject<JsonArchive>(testVectorOfMaps, inputJson);
```

#### Serializing custom class
To support serialization of your type, you must implement the Serialize() method, which can also be implemented as an external method in the BitSerializer namespace (take a look at the implementation in the library).
```cpp
#include "bitserializer\bit_serializer.h"
#include "bitserializer\archives\json_restcpp_archive.h"

using namespace BitSerializer;

class TestSimpleClass
{
public:
	TestSimpleClass()
	{
		TestBool = true;
		TestString = L"Hello world!";
		for (size_t i = 0; i < 3; i++) {
			for (size_t k = 0; k < 2; k++) {
				TestTwoDimensionArray[i][k] = i * 10 + k;
			}
		}
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeKeyValue("TestBool", TestBool);
		archive << MakeKeyValue("TestString", TestString);
		archive << MakeKeyValue("TestTwoDimensionArray", TestTwoDimensionArray);
	};

private:
	bool TestBool;
	std::wstring TestString;
	size_t TestTwoDimensionArray[3][2];
};

int main()
{
	auto simpleObj = TestSimpleClass();
	auto result = BitSerializer::SaveObject<JsonArchive>(simpleObj);
    return 0;
}
```
Returns result
```json
{
	"TestBool": true,
	"TestString": "Hello world!",
	"TestTwoDimensionArray": [
		[0, 1],
		[10, 11],
		[20, 21]
	]
}
```

#### Serializing base class
To serialize the base class, use the helper method BaseObject(), as in the next example.
```cpp
template <class TArchive>
void Serialize(TArchive& archive)
{
	archive << BaseObject<MyBaseClass>(*this);
	archive << MakeKeyValue("TestInt2", TestInt2);
};
```

#### Serializing enum types
To be able to serialize enum types, you must register a map with string equivalents.
```cpp
// file HttpMethods.h
#pragma once
#include "bitserializer\string_conversion.h"

enum class HttpMethod {
	Delete = 1,
	Get = 2,
	Head = 3
};

REGISTER_ENUM_MAP(HttpMethod)
{
	{ HttpMethod::Delete,   "delete" },
	{ HttpMethod::Get,      "get" },
	{ HttpMethod::Head,     "head" }
}
END_ENUM_MAP()
```

#### Compile time checking
If you try to serialize an object that is not supported at the current level of the archive, you will receive a compiler error message.
```cpp
template <class TArchive>
inline void Serialize(TArchive& archive)
{
    // Error    C2338	BitSerializer. The archive doesn't support serialize fundamental type without key on this level.
    archive << TestBool;
    // Proper use
		archive << MakeKeyValue("TestString", TestString);
};
```

#### Error handling
```cpp
try
{
	int testInt;
	BitSerializer::LoadObject<JsonArchive>(testInt, L"10 ?");
}
catch (const BitSerializer::SerializationException& ex)
{
	// Parsing error: Malformed token
	std::string message = ex.what();
}
```

License
----
MIT, Copyright (C) 2018 by Pavel Kisliak

The library currently was tested only on VS 2017 and still in development, please use it at your own risk.
