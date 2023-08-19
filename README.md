# BitSerializer ![Generic badge](https://img.shields.io/badge/Version-0.50-blue) [![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](license.txt) [![Build Status](https://dev.azure.com/real0793/BitSerializer/_apis/build/status%2FGitHub-BitSerializer?branchName=master)](https://dev.azure.com/real0793/BitSerializer/_build/latest?definitionId=5&branchName=master)
___

### Design goals:
- Make a thin wrapper around existing libraries for have one common serialization interface.
- Make it easy to serialize for all built-in C++ and STD types.
- Follow ISO data exchange standards for easy integration with other systems.
- Good test coverage for keep stability of project.
- Cross-platform (Windows, Linux, MacOS).

### Main features:
- One common interface for different kind of formats (currently supported JSON, XML, YAML and CSV).
- Simple syntax which is similar to serialization in the Boost library.
- Customizable validation of deserialized values with producing an output list of errors.
- Support serialization for enum types (via declaring names map).
- Support serialization for the most commonly used STD containers and types including modern `std::u16string` and `std::u32string`.
- Support serialization to streams and files.
- Encoding to various UTF formats.
- Useful [string conversion submodule](docs/bitserializer_convert.md) (supports enums, classes, UTF encoding).

#### Supported formats:
| BitSerializer sub-module | Format | Encoding | Pretty format | Based on |
| ------ | ------ | ------ |:------:| ------ |
| [cpprestjson-archive](docs/bitserializer_cpprest_json.md) | JSON | UTF-8 | ✖ | [C++ REST SDK](https://github.com/Microsoft/cpprestsdk) |
| [rapidjson-archive](docs/bitserializer_rapidjson.md) | JSON | UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE | ✅ | [RapidJson](https://github.com/Tencent/rapidjson) |
| [pugixml-archive](docs/bitserializer_pugixml.md) | XML | UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE | ✅ | [PugiXml](https://github.com/zeux/pugixml) |
| [rapidyaml-archive](docs/bitserializer_rapidyaml.md) | YAML | UTF-8 | N/A | [RapidYAML](https://github.com/biojppm/rapidyaml) |
| [csv-archive](docs/bitserializer_csv.md) | CSV | UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE | N/A | Built-in |

#### Requirements:
  - C++ 17 (VS2017, GCC-8, CLang-8, AppleCLang-12).
  - Dependencies which are required by selected type of archive.

(*) Minimal requirement for RapidYaml archive is VS2019 (with using the latest version of RapidYaml library 0.4.1).


##### What's new in version 0.50:
- [ ! ] Added new archive for serialization to CSV, supports all UTF encodings with auto-detection (built-in implementation, no dependencies).
- [ ! ] API breaking change - deprecated global `BitSerializer::Context`, now validation errors will propagate only via `ValidationException`.
- [ ! ] Removed all static memory allocations for be compatible with custom allocators.
- [ + ] Added policy `OverflowNumberPolicy` for case when size of target type is not enough for loading number.
- [ + ] Added policy `MismatchedTypesPolicy` for case when type of target field does not match the value being loaded.
- [ + ] Added default `SerializationOptions`.
- [ * ] Added `ParsingException` with information about line number or offset (depending on format type).
- [ * ] Added new simplified macro `REGISTER_ENUM` - replacement for `REGISTER_ENUM_MAP` which is deprecated.
- [ + ] Conversion sub-module: Added error policy for encode UTF (error mark, throw exception or skip).
- [ * ] Conversion sub-module: Added throwing `invalid_argument` exception when converting from invalid string to number.
- [ * ] Conversion sub-module: Converting a string containing floating point number to integer, now will throw `out_of_range` exception.
- [ * ] Conversion sub-module: Fixed work with raw pointers in the UTF-16Be and UTF-32Be encoders.
- [ * ] Conversion sub-module: Fixed macro `DECLARE_ENUM_STREAM_OPS` (can't be used in namespaces).
- [ * ] [CppRestJson] Fixed serialization of booleans in the object (was serialized as number).
- [ * ] [RapidYaml] Fixed compatibility with latest version of the RapidYaml library (0.4.1).
- [ * ] [RapidYaml] Fixed serialization negative int8.
- [ * ] [RapidYaml] Fixed issue with error handling when multi-thread serialization.
- [ * ] [RapidJson, CppRestJson, RapidYaml] Fixed path in the validation errors, index in arrays was shifted by 1.

[Full log of changes](History.md)

### Performance
For check performance overhead, was developed a single thread test that serializes a model via the BitSerializer and via the API provided by base libraries. The model for tests includes a various types that are supported by all formats.

| Base library name | Format | Operation | Native API | BitSerializer | Difference |
| ------ | ------ | ------ |  ------ | ------ | ------ |
| RapidJson | JSON | Save object | 421799 Kb/s  | 452667 Kb/s | **(-6.8%)** |
| RapidJson | JSON | Load object | 274952 Kb/s | 310186 Kb/s | **(-11.4%)** |
| C++ REST SDK | JSON | Save object | 83944 Kb/s | 85608 Kb/s | **(-1.9%)** |
| C++ REST SDK | JSON | Load object | 83153 Kb/s | 83875 Kb/s | **(-0.9%)** |
| PugiXml | XML | Save object | 386395 Kb/s | 395489 Kb/s | **(-2.3%)** |
| PugiXml | XML | Load object | 583627 Kb/s | 669654 Kb/s | **(-12.8%)** |
| RapidYAML | YAML | Save object | 28929 Kb/s | 28929 Kb/s | **(0%)** |
| RapidYAML | YAML | Load object | 105062 Kb/s | 113461 Kb/s | **(-7.4%)** |
| Built-in | CSV | Save object | N/A | 213350 Kb/s | N/A |
| Built-in | CSV | Load object | N/A | 212364 Kb/s | N/A |

Measured in **Kb/s** - speed of read/write to the output archive, more is better. Results are depend to system hardware and compiler options, there is important only **differences in percentages** which show BitSerializer's overhead over base libraries. The source code of the test also available [here](tests/performance_tests).

___
## Table of contents
- [How to install](#markdown-header-how-to-install)
- [Hello world](#markdown-header-hello-world)
- [Unicode support](#markdown-header-unicode-support)
- [Serializing class](#markdown-header-serializing-class)
- [Serializing base class](#markdown-header-serializing-base-class)
- [Serializing third party class](#markdown-header-serializing-third-party-class)
- [Serializing enum types](#markdown-header-serializing-enum-types)
- [Serializing custom classes representing a string or number](#markdown-header-serializing-custom-classes-representing-a-string-or-number)
- [Serializing to multiple formats](#markdown-header-serializing-to-multiple-formats)
- [Serialization STD types](#markdown-header-serialization-std-types)
- [Specifics of serialization STD map](#markdown-header-specifics-of-serialization-std-map)
- [Conditions for checking the serialization mode](#markdown-header-conditions-for-checking-the-serialization-mode)
- [Serialization to streams and files](#markdown-header-serialization-to-streams-and-files)
- [Error handling](#markdown-header-error-handling)
- [Validation of deserialized values](#markdown-header-validation-of-deserialized-values)
- [Compile time checking](#markdown-header-compile-time-checking)
- [Thanks](#markdown-header-thanks)
- [License](#markdown-header-license)

### Details of archives
- [JSON archive "bitserializer-cpprestjson"](docs/bitserializer_cpprest_json.md)
- [JSON archive "bitserializer-rapidjson"](docs/bitserializer_rapidjson.md)
- [XML archive "bitserializer-pugixml"](docs/bitserializer_pugixml.md)
- [YAML archive "bitserializer-rapidyaml"](docs/bitserializer_rapidyaml.md)
- [CSV archive "bitserializer-csv"](docs/bitserializer_csv.md)

___


### How to install
BitSerializer requires the installation of third-party libraries, this will depend on which archives you need.
The easiest way is to use one of supported package managers, in this case, third-party libraries will be installed automatically.
Please follow [instructions](#markdown-header-details-of-archives) for specific archives.
#### VCPKG
Just add BitSerializer to manifest file (`vcpkg.json`) in your project:
```json
{
    "dependencies": [
        {
            "name": "bitserializer",
            "features": [ "cpprestjson-archive", "rapidjson-archive", "pugixml-archive", "rapidyaml-archive", "csv-archive" ]
        }
    ]
}
```
Enumerate features which you need, by default all are disabled.
Alternatively, you can install the library via the command line:
```shell
> vcpkg install bitserializer[cpprestjson-archive,rapidjson-archive,pugixml-archive,rapidyaml-archive,csv-archive]
```
In the square brackets enumerated all available formats, install only which you need.
#### Conan
The recipe of BitSerializer is available on [Conan-center](https://github.com/conan-io/conan-center-index), just add BitSerializer to `conanfile.txt` in your project and enable archives which you need via options (by default all are disabled):
```
[requires]
bitserializer/0.50

[options]
bitserializer:with_cpprestsdk=True
bitserializer:with_rapidjson=True
bitserializer:with_pugixml=True
bitserializer:with_rapidyaml=True
bitserializer:with_csv=True
```
Alternatively, you can install via below command (this is just example without specifying generator, arguments for target compiler, architecture, etc):
```shell
> conan install bitserializer/0.50@ -o bitserializer:with_cpprestsdk=True -o bitserializer:with_rapidjson=True -o bitserializer:with_pugixml=True -o bitserializer:with_csv=True -o > bitserializer:with_rapidyaml=True --build missing
```
#### Installation via CMake on a Unix system
```sh
$ git clone https://github.com/PavelKisliak/BitSerializer.git
$ # Enable only archives which you need (by default all are disabled)
$ cmake bitserializer -B bitserializer/build -DBUILD_CPPRESTJSON_ARCHIVE=ON -DBUILD_RAPIDJSON_ARCHIVE=ON -DBUILD_PUGIXML_ARCHIVE=ON -DBUILD_RAPIDYAML_ARCHIVE=ON -DBUILD_CSV_ARCHIVE=ON
$ sudo cmake --build bitserializer/build --config Debug --target install
$ sudo cmake --build bitserializer/build --config Release --target install
```
You will also need to install dev-packages of base libraries, currently available only `rapidjson-dev` and `libpugixml-dev`, the rest need to be built manually (CSV archive does not require any dependencies).

#### How to use with CMake
```cmake
find_package(bitserializer CONFIG REQUIRED)
# Link only archives which you need
target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::cpprestjson-archive
    BitSerializer::rapidjson-archive
    BitSerializer::pugixml-archive
    BitSerializer::rapidyaml-archive
    BitSerializer::csv-archive
)
```

### Hello world
Let's get started with traditional and simple "Hello world!" example.
```cpp
#include <cassert>
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/cpprestjson_archive.h"

using JsonArchive = BitSerializer::Json::CppRest::JsonArchive;

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
There is no mistake as JSON format supported any type (object, array, number or string) at root level.

### Unicode support
Besides multiple input and output UTF-formats that BitSerializer supports, it also allows to serialize any of `std::basic_string` types, under the hood, they are transcoding to output format. You also free to use any string type as keys (with using `MakeAutoKeyValue()`), but remember that transcoding takes additional time and of course it is better to use string types which are natively supported by a particular archive, usually `std::string` (UTF-8). In the example below, we show how BitSerializer allows to play with string types:
```cpp
class TestUnicodeClass
{
public:
	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		// Serialize UTF-8 string with key in UTF-16
		archive << MakeAutoKeyValue(u"Utf16Key", mUtf8StringValue);

		// Serialize UTF-16 string with key in UTF-32
		archive << MakeAutoKeyValue(U"Utf32Key", mUtf16StringValue);

		// Serialize UTF-32 string with key in UTF-8
		archive << MakeAutoKeyValue(u8"Utf8Key", mUtf32StringValue);
	};

private:
	std::string mUtf8StringValue;
	std::u16string mUtf16StringValue;
	std::u32string mUtf32StringValue;
};
```

### Serializing class
There are two ways to serialize a class:

  * Internal public method `Serialize()` - good way for your own classes.
  * External static function `SerializeObject()` - used for third party class (no access to sources).

Below example demonstrates how to implement internal serialization method:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"

using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

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
		using namespace BitSerializer;
		archive << MakeKeyValue("TestBool", testBool);
		archive << MakeKeyValue("TestString", testString);
		archive << MakeKeyValue("TestTwoDimensionArray", testTwoDimensionArray);
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
	archive << MakeKeyValue("TestInt", TestInt);
};
```

### Serializing third party class
As alternative for internal `Serialize()` method also exists approach with defining global functions, it will be useful in next cases:

 - Sources of serializing class cannot be modified (for example from third party library).
 - When class represents list of some values (such as `std::vector`).
 - When you would like to override internal serialization, globally defined functions have higher priority.
 - When you strongly follow single responsibility principle and wouldn't like to include serialization code into class.

You need to implement one of below functions in any namespace:

 - `SerializeObject()` - when you would like to represent your class as object in a target format (e.g. JSON object).
 - `SerializeArray()` - when you would like to represent your class as array in a target format (e.g. JSON array).

The example below shows how to implement the most commonly used external function SerializeObject().
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

class TestThirdPartyClass
{
public:
	TestThirdPartyClass(int x, int y)
		: x(x), y(y)
	{ }

	int x;

	int GetY() const noexcept { return y; }
	void SetY(int y) noexcept { this->y = y; }

private:
	int y;
};

template<typename TArchive>
void SerializeObject(TArchive& archive, TestThirdPartyClass& testThirdPartyClass)
{
	// Serialize public property
	archive << MakeAutoKeyValue("x", testThirdPartyClass.x);

	// Serialize private property
	if constexpr (TArchive::IsLoading()) {
		int y = 0;
		archive << MakeAutoKeyValue("y", y);
		testThirdPartyClass.SetY(y);
	}
	else {
		const int y = testThirdPartyClass.GetY();
		archive << MakeAutoKeyValue("y", y);
	}
}


int main()
{
	auto testObj = TestThirdPartyClass(100, 200);
	const auto result = BitSerializer::SaveObject<JsonArchive>(testObj);
	std::cout << result << std::endl;
	return 0;
}
```
[See full sample](samples/serialize_third_party_class/serialize_third_party_class.cpp)

### Serializing custom classes representing a string or number
This chapter describes how to implement serialization for classes which should be represented in the output format as base types, like number, boolean or string. Let's imagine that you would like to implement serialization of your own `std::string` alternative, which is called `CMyString`. For this purpose you need two global functions with the following signatures:
```cpp
template <class TArchive, typename TKey>
bool Serialize(TArchive& archive, TKey&& key, CMyString& value)

template <class TArchive>
bool Serialize(TArchive& archive, CMyString& value)
```
These two functions are necessary for serialization any type with and without **key** into the output archive. For example, object in the JSON format, has named properties, but JSON-array can contain only values.
The BitSerializer's archives works with `std::basic_string<>`, so you need to convert your custom string before serialize. The best way is to implement string conversion methods as required by BitSerialzer ([see more](docs/bitserializer_convert.md)), this will also help you get the ability to serialize `std::map`, where the custom string is used as the key.
This all looks a little more complicated than serializing the object, but the code is pretty simple, please have a look at the example below:
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/types/std/map.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

// Some custom string type
class CMyString
{
public:
	CMyString() = default;
	CMyString(const char* str) : mString(str) { }

	bool operator<(const CMyString& rhs) const { return this->mString < rhs.mString; }

	// Required methods for conversion from/to std::string (can be implemented as external functions)
	std::string ToString() const { return mString; }
	void FromString(std::string_view str) { mString = str; }

private:
	std::string mString;
};

// Serializes CMyString with key
template <class TArchive, typename TKey>
bool Serialize(TArchive& archive, TKey&& key, CMyString& value)
{
	constexpr auto hasStringWithKeySupport = can_serialize_value_with_key_v<TArchive, std::string, TKey>;
	static_assert(hasStringWithKeySupport, "BitSerializer. The archive doesn't support serialize string type with key on this level.");

	if constexpr (hasStringWithKeySupport)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::string str;
			if (archive.SerializeValue(std::forward<TKey>(key), str))
			{
				value.FromString(str);
				return true;
			}
		}
		else
		{
			std::string str = value.ToString();
			return archive.SerializeValue(std::forward<TKey>(key), str);
		}
	}
	return false;
}

// Serializes CMyString without key
template <class TArchive>
bool Serialize(TArchive& archive, CMyString& value)
{
	constexpr auto hasStringSupport = can_serialize_value_v<TArchive, std::string>;
	static_assert(hasStringSupport, "BitSerializer. The archive doesn't support serialize string type without key on this level.");

	if constexpr (hasStringSupport)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::string str;
			if (archive.SerializeValue(str))
			{
				value.FromString(str);
				return true;
			}
		}
		else
		{
			std::string str = value.ToString();
			return archive.SerializeValue(str);
		}
	}
	return false;
}

int main()
{
	// Save list of custom strings to JSON
	std::vector<CMyString> srcStrList = { "Red", "Green", "Blue" };
	std::string jsonResult;
	SerializationOptions serializationOptions;
	serializationOptions.formatOptions.enableFormat = true;
	BitSerializer::SaveObject<JsonArchive>(srcStrList, jsonResult, serializationOptions);
	std::cout << "Saved JSON: " << jsonResult << std::endl;

	// Load JSON-object to std::map based on custom strings
	std::map<CMyString, CMyString> mapResult;
	const std::string srcJson = R"({ "Background": "Blue", "PenColor": "White", "PenSize": "3", "PenOpacity": "50" })";
	BitSerializer::LoadObject<JsonArchive>(mapResult, srcJson);
	std::cout << std::endl << "Loaded map: " << std::endl;
	for (const auto& val : mapResult)
	{
		std::cout << "\t" << val.first.ToString() << ": " << val.second.ToString() << std::endl;
	}

	return 0;
}
```
[See full sample](samples/serialize_custom_string/serialize_custom_string.cpp)

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

REGISTER_ENUM(HttpMethod, {
	{ HttpMethod::Delete,   "delete" },
	{ HttpMethod::Get,      "get" },
	{ HttpMethod::Head,     "head" }
})
```

### Serializing to multiple formats
One of the advantages of BitSerializer is the ability to serialize to multiple formats via a single interface. The following example shows how to store an object in JSON and XML:
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
The second thing which you would like to customize is default structure of output XML. In this example it does not looks good from XML perspective, as it has specific element for this purpose which known as "attribute". The BitSerializer also allow to customize the serialization behavior for different formats:
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
#include "bitserializer/types/std/optional.h"
#include "bitserializer/types/std/memory.h"
```
Few words about serialization smart pointers. There is no any system footprints in output archive, for example empty smart pointer will be serialized as `NULL` type in JSON or in any other suitable way for other archive types. When an object is loading into an empty smart pointer, it will be created, and vice versa, when the loaded object is `NULL` or does not exist, the smart pointer will be reset. Polymorphism are not supported you should take care about such types by yourself.

### Specifics of serialization STD map
Due to the fact that the map key is used as a key (in JSON for example), it must be convertible to `std::string` (by default supported all of fundamental types).
```cpp
std::map<std::string, int> testMap = 
	{ { "One", 1 }, { "Two", 2 }, { "Three", 3 }, { "Four", 4 }, { "Five", 5 } };
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

Below is a more complex example, where loading a vector of maps from JSON.
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
const std::string inputJson = "[{\"One\":1,\"Three\":3,\"Two\":2},{\"Five\":5,\"Four\":4}]";
BitSerializer::LoadObject<JsonArchive>(testVectorOfMaps, inputJson);
```

If you want to use your own class as a key, you should add conversion methods to it. There are several options with internal and external functions, please look details [here](docs\bitserializer_convert.md). You also can override serialization function `SerializeObject()` for your type of map. For example, you can implement two internal methods in your type:
```cpp
class YourCustomKey
{
	std::string ToString() const { }
	void FromString(std::string_view str)
}
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

### Serialization to streams and files
All archives in the BitSerializer support streams as well as serialization to files. In comparison to serialization to `std::string`, streams/files also supports UTF encodings.
BitSerializer can detect encoding of input stream by BOM ([Byte order mark](https://en.wikipedia.org/wiki/Byte_order_mark)) and via data analysis, but last is only supported by RapidJson, PugiXml and CSV archives. The output encoding and BOM is configurable via `SerializationOptions`.
The following example shows how to save/load to `std::stream`:
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

For save/load to files, BitSerializer provides the following functions (which are just wrappers of serialization methods to streams):
```cpp
	BitSerializer::SaveObjectToFile<TArchive>(obj, path);
	BitSerializer::LoadObjectFromFile<TArchive>(obj, path);
```

### Error handling
First, let's list what are considered as errors and will throw exception:

 - Syntax errors in the input source (e.g. JSON)
 - When one or more user's validation rules were not passed
 - When a type from the archive (source format, like JSON) does not match to the target value (can be configured via `MismatchedTypesPolicy`)
 - When size of target type is not enough for loading value (can be configured via `OverflowNumberPolicy`)
 - When target array with fixed size does not match the number of loading items
 - Invalid configuration in the `SerializationOptions`
 - Input/output file can't be opened for read/write
 - Unsupported UTF encoding

By default, any missed field in the input format (e.g. JSON) is not treated as an error, but you can add `Required()` validator if needed.
You can handle `std::exception` just for log errors, but if you need to provide the user more details, you may need to handle below exceptions:

 - `SerializationException` - base BitSerializer exception, contains `SerializationErrorCode`
 - `ParsingException` - contains information about line number or offset (depending on format type)
 - `ValidationException` - contains map of fields with validation errors

```cpp
try
{
	int testInt;
	BitSerializer::LoadObject<JsonArchive>(testInt, L"10 ?");
}
catch (const BitSerializer::ParsingException& ex)
{
	// Parsing error: Malformed token
	std::string message = ex.what();
	size_t line = ex.Line;
	size_t offset = ex.Offset;
}
catch (const BitSerializer::ValidationException& ex)
{
	// Handle validation errors
	const auto& validationErrors = ex.GetValidationErrors();
}
catch (const std::exception& ex)
{
	// Handle any other errors
	std::string message = ex.what();
}
```

### Validation of deserialized values
BitSerializer allows to add an arbitrary number of validation rules to the named values, the syntax is quite simple:
```cpp
archive << MakeKeyValue("testFloat", testFloat, Required(), Range(-1.0f, 1.0f));
```
For handle validation errors, need to catch special exception `ValidationException`, it is thrown at the end of deserialization when all errors have been collected.
The map of validation errors can be get by calling method `GetValidationErrors()`, it contains paths to fields with errors lists.

Basically implemented few validators: `Required`, `Range`, `MinSize`, `MaxSize`.
Validator `Range` can be used with all types which have operators '<' and '>'.
Validators `MinSize` and `MaxSize` can be applied to all values which have `size()` method.
This list will be extended in future.
```cpp
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

class UserModel
{
public:
	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		using namespace BitSerializer;

		archive << MakeKeyValue("Id", mId, Required());
		archive << MakeKeyValue("Age", mAge, Required(), Range(0, 150));
		archive << MakeKeyValue("FirstName", mFirstName, Required(), MaxSize(16));
		archive << MakeKeyValue("LastName", mLastName, Required(), MaxSize(16));
		// Custom validation with lambda
		archive << MakeKeyValue("NickName", mNickName, [](const std::string& value, const bool isLoaded) -> std::optional<std::string>
		{
			// Loaded string should has text without spaces or should be NULL
			if (!isLoaded || value.find_first_of(' ') == std::string::npos)
				return std::nullopt;
			return "The field must not contain spaces";
		});
	}

private:
	uint64_t mId = 0;
	uint16_t mAge = 0;
	std::string mFirstName;
	std::string mLastName;
	std::string mNickName;
};

int main()
{
	UserModel user;
	const char* json = R"({ "Id": 12420, "Age": 500, "FirstName": "John Smith-Cotatonovich", "NickName": "Smith 2000" })";
	try
	{
		BitSerializer::LoadObject<JsonArchive>(user, json);
	}
	catch (BitSerializer::ValidationException& ex)
	{
		const auto& validationErrors = ex.GetValidationErrors();
		std::cout << "Validation errors: " << std::endl;
		for (const auto& keyErrors : validationErrors)
		{
			std::cout << "Path: " << keyErrors.first << std::endl;
			for (const auto& err : keyErrors.second)
			{
				std::cout << "\t" << err << std::endl;
			}
		}
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what();
	}

	return EXIT_SUCCESS;
}
```
[See full sample](samples/validation/validation.cpp)

The result of execution this code:
```text
Validation errors:
Path: /Age
        Value must be between 0 and 150
Path: /FirstName
        The maximum size of this field should be not greater than 16
Path: /LastName
        This field is required
Path: /NickName
        The field must not contain spaces
```
Returned paths for invalid values is dependent to archive type, in this sample it's JSON Pointer (RFC 6901).

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
	archive << MakeKeyValue("testString", testString);
};
```

Thanks
----
- Artsiom Marozau for developing an archive with support YAML.
- Andrey Mazhyrau for help with cmake scripts, fix GCC and Linux related issues.
- Alexander Stepaniuk for support and participation in technical discussions.
- Evgeniy Gorbachov for help with implementation STD types serialization.
- Mateusz Pusz for code review and useful advices.

License
----
MIT, Copyright (C) 2018-2022 by Pavel Kisliak, made in Belarus 🇧🇾
