# BitSerializer [![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://lbesson.mit-license.org/) [![Build Status](https://dev.azure.com/real0793/BitSerializer/_apis/build/status/BitSerializer-CI?branchName=master)](https://dev.azure.com/real0793/BitSerializer/_build/latest?definitionId=4&branchName=master) 
___

##### WORK IN PROGRESS! Next version 0.9

### Design goals:
- Make a thin wrapper around existing libraries for have one common serialization interface.
- Make easy serialization for all kind of C++ types and STL containers.
- Produce clear JSON for easy integration with Javascript.
- Good test coverage for keep stability of project.
- Cross-platform (Windows, Linux, MacOS).

### Main features:
- One common interface for different kind of formats (currently supported JSON, XML and YAML).
- Simple syntax which is similar to serialization in the Boost library.
- Validation of deserialized values with producing an output list of errors.
- Support serialization for enum types (via declaring names map).
- Support serialization for all STD containers.
- Support serialization to streams and files.
- Encoding to various UTF formats.
- Useful string conversion submodule (supports enums, classes, UTF encoding).

#### Supported formats:
| BitSerializer sub-module | Format | Encoding | Pretty format | Based on |
| ------ | ------ | ------ |:------:| ------ |
| [bitserializer-cpprestjson](docs/bitserializer_cpprest_json.md) | JSON | UTF-8 | ❌ | [C++ REST SDK](https://github.com/Microsoft/cpprestsdk) |
| [bitserializer-rapidjson](docs/bitserializer_rapidjson.md) | JSON | UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE | ✅ | [RapidJson](https://github.com/Tencent/rapidjson) |
| [bitserializer-pugixml](docs/bitserializer_pugixml.md) | XML | UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE | ✅ | [PugiXml](https://github.com/zeux/pugixml) |
| [bitserializer_rapidyaml](docs/bitserializer_rapidyaml.md) | YAML | UTF-8 | N/A | [RapidYAML](https://github.com/biojppm/rapidyaml) |

#### Requirements:
  - C++ 17 (VS2017, GCC-8, CLang-7).
  - Dependencies which are required by selected type of archive.

##### What's new in version 0.9:
- [ ! ] Added XML serialization support (based on library PugiXml).
- [ ! ] Added YAML serialization support (based on library RapidYaml).
- [ ! ] Add CI with builds for Windows, Linux (GCC, Clang) and MaOS (AppleClang).
- [ + ] Add formatting options for output text (but formatting is not supported in CppRestJson).
- [ + ] Add support encoding to various UTF based formats (defines in serialization options).
- [ + ] Add optional writing the BOM to output stream/file.
- [ + ] Add ability for pretty format of output text.
- [ + ] Add UTF encoding when serializing std::wstring.
- [ + ] Add serialization for all STD containers which were missed before.
- [ + ] Add serialization C++ union type.
- [ \* ] Split implementation of serialization for std types into separate files.
- [ \* ] Change string type for path in archive from std::wstring to std::string (in UTF-8 encoding).
- [ \* ] For archive based on RapidJson was changed in-memory encoding from UTF-16 to UTF-8.
- [ \* ] Add path into exceptions about I/O errors with files.
- [ \* ] Fix registration enum types not in global namespace.
- [ \* ] Add constants with library version.

[Full log of changes](History.md)

### Performance
For check performance overhead, was developed a test which serializes a model via the BitSerializer and via the API provided by base libraries. The model for tests includes a various types that are supported by all formats. The source code of the test also available [here](tests/performance_tests).

| Base library name | Format | Operation | Native API | BitSerializer | Difference |
| ------ | ------ | ------ |  ------ | ------ | ------ |
| RapidJson | JSON | Save object | 26 msec | 28 msec | 2 msec **(-7.1%)** |
| RapidJson | JSON | Load object | 35 msec | 38 msec | 3 msec **(-7.9%)** |
| C++ REST SDK | JSON | Save object | 199 msec | 200 msec | 1 msec **(-0.5%)** |
| C++ REST SDK | JSON | Load object | 184 msec | 188 msec | 4 msec **(-2.1%)** |
| PugiXml | XML | Save object | 77 msec | 79 msec | 2 msec **(-2.5%)** |
| PugiXml | XML | Load object | 42 msec | 44 msec | 2 msec **(-4.5%)** |
| RapidYAML | YAML | Save object | 550 msec | 549 msec | 1 msec **(-0.2%)** |
| RapidYAML | YAML | Load object | 343 msec | 348 msec | 5 msec **(-1.4%)** |

Results are depend to system hardware and compiler options, there is important only **differences in percentages** which show BitSerializer's overhead over base libraries. The JSON implementation from C++ REST SDK has worse result, but need to say that on Windows platform it uses UTF-16 in memory when other libraries UTF-8.

___
## Table of contents
- [How to install](#markdown-header-how-to-install)
- [Hello world](#markdown-header-hello-world)
- [Serializing class](#markdown-header-serializing-class)
- [Serializing base class](#markdown-header-serializing-base-class)
- [Serializing third party class](#markdown-header-serializing-third-party-class)
- [Serializing enum types](#markdown-header-serializing-enum-types)
- [Serialize to multiple formats](#markdown-header-serialize-to-multiple-formats)
- [Serialization STD types](#markdown-header-serialization-std-types)
- [Specifics of serialization STD map](#markdown-header-specifics-of-serialization-std-map)
- [Conditions for checking the serialization mode](#markdown-header-conditions-for-checking-the-serialization-mode)
- [Validation of deserialized values](#markdown-header-validation-of-deserialized-values)
- [Serialization to streams and files](#markdown-header-serialization-to-streams-and-files)
- [Compile time checking](#markdown-header-compile-time-checking)
- [Error handling](#markdown-header-error-handling)
- [Thanks](#markdown-header-thanks)
- [License](#markdown-header-license)

### Details of archives
- [JSON archive "bitserializer-cpprestjson"](docs/bitserializer_cpprest_json.md)
- [JSON archive "bitserializer-rapidjson"](docs/bitserializer_rapidjson.md)
- [XML archive "bitserializer-pugixml"](docs/bitserializer_pugixml.md)
- [YAML archive "bitserializer-rapidyaml"](docs/bitserializer_rapidyaml.md)

___


### How to install
The library consists of header files only, but it uses third-party libraries which should be also installed.
The easiest way is to use one of supported package managers, in this case, third-party libraries will be installed automatically. Please follow [instructions](#markdown-header-details-of-archives) for specific archives.

### Hello world
Let's get started with traditional and simple "Hello world!" example.
```cpp
#include <cassert>
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer_cpprest_json/cpprest_json_archive.h"

using namespace BitSerializer::Json::CppRest;

int main()
{
	std::string expected = "Hello world!";
	auto json = BitSerializer::SaveObject<JsonArchive>(expected);
	std::string result;
	BitSerializer::LoadObject<JsonArchive>(result, json);

	assert(result == expected);
	std::cout << result << std::endl;

	return EXIT_SUCCESS;
}
```
[See full sample](samples/hello_world/hello_world.cpp)
There is no mistake as JSON format supported any type at root level.

### Serializing class
There are two ways to serialize a class:

  * Internal public method `Serialize()` - good way for your own classes.
  * External static function `Serialize()` - used for third party class (no access to sources).

Below example demonstrates how to implement internal serialization method:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer_cpprest_json/cpprest_json_archive.h"

using namespace BitSerializer;
using namespace BitSerializer::Json::CppRest;

class TestSimpleClass
{
public:
	TestSimpleClass()
		: testBool(true)
		, testString(L"Hello world!")
	{
		for (size_t i = 0; i < 3; i++)
		{
			for (size_t k = 0; k < 2; k++) {
				testTwoDimensionArray[i][k] = i * 10 + k;
			}
		}
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeKeyValue(L"TestBool", testBool);
		archive << MakeKeyValue(L"TestString", testString);
		archive << MakeKeyValue(L"TestTwoDimensionArray", testTwoDimensionArray);
	};

private:
	bool testBool;
	std::wstring testString;
	size_t testTwoDimensionArray[3][2];
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
For serializing a named object please use helper method `MakeKeyValue(key, value)`. The type of key should be supported by archive, but there also exists method `MakeAutoKeyValue(key, value)` which automatically converts to the preferred by archive key type. The good place for using this method is some common serialization code that can be used with various types of archives.

### Serializing base class
To serialize the base class, use the helper method `BaseObject()`, as in the next example.
```cpp
template <class TArchive>
void Serialize(TArchive& archive)
{
	archive << BaseObject<MyBaseClass>(*this);
	archive << MakeKeyValue(L"TestInt", TestInt);
};
```

### Serializing third party class
For serialize a third party class (non-intrusive serialization), which source cannot be modified, need to implement two types of `Serialize()` methods in the namespace BitSerializer. The first method responsible to serialize a value with key, the second - without. This is a basic concept of BitSerializer which helps to control at compile time a possibility of type serialization in the current level of archive. For example, you can serialize any type to a root level of JSON, but you can't do it with key. In other case, when you in the object scope of JSON, you can serialize values only with keys.
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"

class TestThirdPartyClass
{
public:
	TestThirdPartyClass(int x, int y)
		: x(x), y(y)
	{ }

	int x, y;
};

namespace BitSerializer
{
	namespace Detail
	{
		class TestThirdPartyClassSerializer
		{
		public:
			TestThirdPartyClassSerializer(TestThirdPartyClass& value)
				: value(value)
			{ }

			template <class TArchive>
			void Serialize(TArchive& archive)
			{
				archive << MakeAutoKeyValue(L"x", value.x);
				archive << MakeAutoKeyValue(L"y", value.y);
			}

			TestThirdPartyClass& value;
		};
	}	// namespace Detail

	template<typename TArchive, typename TKey>
	void Serialize(TArchive& archive, TKey&& key, TestThirdPartyClass& value)
	{
		auto serializer = Detail::TestThirdPartyClassSerializer(value);
		Serialize(archive, key, serializer);
	}
	template<typename TArchive>
	void Serialize(TArchive& archive, TestThirdPartyClass& value)
	{
		auto serializer = Detail::TestThirdPartyClassSerializer(value);
		Serialize(archive, serializer);
	}
}	// namespace BitSerializer


using namespace BitSerializer::Json::RapidJson;

int main()
{
	auto testObj = TestThirdPartyClass(100, 200);
	auto result = BitSerializer::SaveObject<JsonArchive>(testObj);
	std::cout << result << std::endl;
	return 0;
}
```
[See full sample](samples/serialize_third_party_class/serialize_third_party_class.cpp)

### Serializing enum types
To be able to serialize enum types, you must register a map with string equivalents in the your HEADER file.
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
} END_ENUM_MAP()
```

### Serialize to multiple formats
One of the advantages of BitSerializer is the ability to serialize in several formats via one interface. In the following example shows saving an object to JSON and XML:
```cpp
class CPoint
{
public:
	CPoint(int x, int y)
		: x(x), y(y)
	{ }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeAutoKeyValue("x", x);
		archive << MakeAutoKeyValue("y", y);
	}

	int x, y;
};

int main()
{
	auto testObj = CPoint(100, 200);

	const auto jsonResult = BitSerializer::SaveObject<JsonArchive>(testObj);
	std::cout << "JSON: " << jsonResult << std::endl;

	const auto xmlResult = BitSerializer::SaveObject<XmlArchive>(testObj);
	std::cout << "XML: " << xmlResult << std::endl;
	return 0;
}
```
The output result of this code:
```
JSON: {"x":100,"y":200}
XML: <?xml version="1.0"?><root><x>100</x><y>200</y></root>
```
The code for serialization has difference only in template parameter - **JsonArchive** and **XmlArchive**.
But here are some moments which need comments. As you can see in the XML was created node with name "root". This is auto generated name when it was not specified explicitly for root node. The library does this just to smooth out differences in the structure of formats. But you are free to set name of root node if needed:
```cpp
const auto xmlResult = BitSerializer::SaveObject<XmlArchive>(MakeAutoKeyValue("Point", testObj));
```
The second thing which you would like to customize is default structure of output XML. In this example it does not looks good from XML perspective, as it has specific element for this purpose which known as "attribute". The BitSerializer also alow to customize the serialization behaviour for different formats:
```cpp
	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		// Serialize as attributes when archive type is XML
		if constexpr (TArchive::archive_type == ArchiveType::Xml)
		{
			archive << MakeAutoAttributeValue("x", x);
			archive << MakeAutoAttributeValue("y", y);
		}
		else
		{
			archive << MakeAutoKeyValue("x", x);
			archive << MakeAutoKeyValue("y", y);
		}
	}
```
With these changes, the result of this code will look like this:
```
JSON: {"x":100,"y":200}
XML: <?xml version="1.0"?><Point x="100" y="200"/>
```
[See full sample](samples/multiformat_customization/multiformat_customization.cpp)

### Serialization STD types
BitSerializer has on board serialization for all STD containers. Serialization of other STD types will be implemented in future. For add support of required STD type just need to include related header file.
```cpp
#include "bitserializer/types/std/array.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/types/std/deque.h"
#include "bitserializer/types/std/bitset.h"
#include "bitserializer/types/std/list.h"
#include "bitserializer/types/std/forward_list.h"
#include "bitserializer/types/std/queue.h"
#include "bitserializer/types/std/stack.h"
#include "bitserializer/types/std/set.h"
#include "bitserializer/types/std/unordered_set.h"
#include "bitserializer/types/std/map.h"
#include "bitserializer/types/std/unordered_map.h"
#include "bitserializer/types/std/pair.h"
```
### Specifics of serialization STD map
Due to the fact that the map key is used as a key in JSON, it must be convertible to a string (by default supported all of fundamental types). This needs to proper serialization JavaScript objects. If you want to use your own class as a key, you can add conversion methods to it. You also can implement specialized serialization for your type of map in extreme cases.
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
For able to serialize `std::map`, which has custom type as a key, you can implement two internal methods in this type:
```cpp
class YourCustomKey
{
	std::string ToString() const { }
	std::wstring ToWString() const { }
}
```
Below more complex example with loads a vector of maps.
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

### Conditions for checking the serialization mode
To check the current serialization mode, use two static methods - `IsLoading()` and `IsSaving()`. As they are «constexpr», you will not have any overhead.
```cpp
class Foo
public:
    template <class TArchive>
    inline void Serialize(TArchive& archive)
    {
    	if constexpr (TArchive::IsLoading()) {
	        // Code which executes in loading mode
	    }
	    else {
    		// Code which executes in saving mode
    	}
	
    	if constexpr (TArchive::IsSaving()) {
		    // Code which executes in saving mode
	    }
	    else {
    		// Code which executes in loading mode
    	}
    }
}
```

### Validation of deserialized values
BitSerializer allows to add an arbitrary number of validation rules to the named values, the syntax is quite simple:
```cpp
archive << MakeKeyValue("testFloat", testFloat, Required(), Range(-1.0f, 1.0f));
```
After deserialize, you can check the status in context and get errors:
```cpp
if (!Context.IsValid())
{
    const auto& validationErrors = Context.GetValidationErrors();
}
```
Basically implemented few validators: `Required`, `Range`, `MinSize`, `MaxSize`.
Validator 'Range' can be used with all types which have operators '<' and '>'.
Validators `MinSize` and `MaxSize` can be applied to all values which have `size()` method.
This list will be extended in future.
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"

using namespace BitSerializer;
using namespace BitSerializer::Json::RapidJson;

class TestSimpleClass
{
public:
	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeKeyValue("TestBool", mTestBool, Required());
		archive << MakeKeyValue("TestInt", mTestInt, Required(), Range(0, 100));
		archive << MakeKeyValue("TestDouble", mTestDouble, Required(), Range(-1.0, 1.0));
		archive << MakeKeyValue("TestString", mTestString, MaxSize(8));
		// Sample with validation via lambda
		archive << MakeKeyValue("TestString2", mTestString, [](const std::string& val, const bool isLoaded) -> std::optional<std::wstring>
		{
			if (!isLoaded || val.find_first_of(' ') == std::string::npos)
				return std::nullopt;
			return L"The field must not contain spaces";
		});
	}

private:
	bool mTestBool = false;
	int mTestInt = 0;
	double mTestDouble = 0.0;
	std::string mTestString;
	std::string mTestString2;
};

int main()
{
	auto simpleObj = TestSimpleClass();
	auto json = "{ \"TestInt\": 2000, \"TestDouble\": 1.0, \"TestString\" : \"Very looooooooong string!\", \"TestString2\" : \"1 23\" }";
	BitSerializer::LoadObject<JsonArchive>(simpleObj, json);
	if (!BitSerializer::Context.IsValid())
	{
		std::wcout << L"Validation errors: " << std::endl;
		const auto& validationErrors = BitSerializer::Context.GetValidationErrors();
		for (const auto& keyErrors : validationErrors)
		{
			std::cout << "Path: " << keyErrors.first << std::endl;
			for (const auto& err : keyErrors.second)
			{
				std::wcout << L"\t" << err << std::endl;
			}
		}
	}

	return EXIT_SUCCESS;
}
```
[See full sample](samples/validation/validation.cpp)

The result of execution this code:
```text
Validation errors:
Path: /TestBool
        This field is required
Path: /TestInt
        Value must be between 0 and 100
Path: /TestString
        The maximum size of this field should be not greater than 8
```
Returned paths for invalid values is dependent to archive type, in this sample it's JSON Pointer (RFC 6901).

### Serialization to streams and files
All archives in the BitSerializer support streams, usually it's a sequence of bytes in UTF encoding (with the exception of binary formats).
Also supported detect/writing BOM ([Byte order mark](https://en.wikipedia.org/wiki/Byte_order_mark)).

```cpp
class CPoint
{
public:
	CPoint() = default;
	CPoint(int x, int y)
		: x(x), y(y)
	{ }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeAutoKeyValue("x", x);
		archive << MakeAutoKeyValue("y", y);
	}

	int x = 0, y = 0;
};

int main()
{
	auto testObj = CPoint(100, 200);

	SerializationOptions serializationOptions;
	serializationOptions.streamOptions.encoding = Convert::UtfType::Utf8;
	serializationOptions.streamOptions.writeBom = false;

	// Save to string stream
	std::stringstream outputStream;
	BitSerializer::SaveObject<JsonArchive>(testObj, outputStream, serializationOptions);
	std::cout << outputStream.str() << std::endl;

	// Load from string stream
	CPoint loadedObj;
	BitSerializer::LoadObject<JsonArchive>(loadedObj, outputStream);

	assert(loadedObj.x == testObj.x && loadedObj.y == testObj.y);
	return 0;
}
```
[See full sample](samples/validation/serialize_to_stream.cpp)

Two other API methods are used for serialization to files:
```cpp
	BitSerializer::SaveObjectToFile<JsonArchive>(testObj, "D:\test_obj.json");
	BitSerializer::LoadObjectFromFile<JsonArchive>(testObj, "D:\test_obj.json");
```
They are just wrappers of serialization methods into streams.

### Compile time checking
The new C++ 17 ability «if constexpr» helps to generate clear error messages.
If you try to serialize an object that is not supported at the current level of the archive, you will receive a simple error message.
```cpp
template <class TArchive>
inline void Serialize(TArchive& archive)
{
    // Error    C2338	BitSerializer. The archive doesn't support serialize fundamental type without key on this level.
    archive << testBool;
    // Proper use
	archive << MakeKeyValue(L"testString", testString);
};
```

### Error handling
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

Thanks
----
- Artsiom Marozau for developing an archive with support YAML.
- Andrey Mazhyrau for help with cmake scripts, fix GCC and Linux related issues.
- Alexander Stepaniuk for support and participation in technical discussions.

License
----
MIT, Copyright (C) 2018-2020 by Pavel Kisliak
