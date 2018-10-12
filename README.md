# BitSerializer
___
The library is designed for simple serialization of arbitrary C++ types to various output formats.

This is the first version open for public access, currently it includes support for only one JSON format, which requires an external C++ REST SDK library. The historical  purpose of creating this library was to simplify the serialization of data for the http server.

The good tests coverage helps to keep stability of project. If you are see kind of issue, please describe it in «[Issues](https://bitbucket.org/Pavel_Kisliak/bitserializer/issues?status=new&status=open)» section.

#### Main features:
  - Support for different formats (currently only JSON).
  - Produces a clear JSON, which is convenient to use with Javascript.
  - Checking at compile time the permissibility of saving types depending on the structure of the output format.
  - Simple syntax (similar to serialization in boost library).
  - Validation of deserialized values.
  - Support for serialization ANSI and wide strings.
  - Support for serialization of most STL containers.
  - Support for serialization of enum types (registration of a names map is required).
  - As a bonus, the subsystem of converting strings to / from arbitrary types.

#### Supported Formats:
  - JSON (the implementation is based on the [C++ REST SDK](https://github.com/Microsoft/cpprestsdk)).

#### Requirements:
  - C++ 17
  - [C++ REST SDK](https://github.com/Microsoft/cpprestsdk)

#### How to use:
The library is contains only header files, but you should install one or more third party libraries which are depend from selected type of archive (please follow instructions for these libraries). As currently the BitSerializer implements only one type of archive, you need to install «CppRestSDK». If you are a Windows user, the best way is to use [Vcpkg manager](https://github.com/Microsoft/vcpkg), in this case, the «CppRestSDK» will be automatically installed as dependency.
```shell
vcpkg install bitserializer bitserializer:x64-windows
```
Now you need just include main file of BitSerializer which implements serialization and file, which implements required format (JSON for example).
```cpp
#include "bitserializer\bit_serializer.h"
#include "bitserializer\archives\json_restcpp_archive.h"
```

___
## Examples of using

#### Hello world!
```cpp
#include <iostream>
#include "bitserializer\bit_serializer.h"
#include "bitserializer\archives\json_restcpp_archive.h"

using namespace BitSerializer;

int main()
{
	std::string expected = "Hello world!";
	auto json = SaveObject<JsonArchive>(expected);
	std::string result;
	LoadObject<JsonArchive>(result, json);
	assert(result == expected);
	std::cout << result;
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

#### Serializing class
There are two ways to serialize a class:

  * Your own class (sources can be modified), possible to create internal or external method Serialize(), but internal is more convenient.
  * Third party class (no access to sources), only external method in namespace BitSerializer.

Next example demonstrates how to implement internal serialization method:
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
	auto result = SaveObject<JsonArchive>(simpleObj);
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
	archive << MakeKeyValue("TestInt", TestInt);
};
```

#### Serializing third party class
For serialize third party class, which source cannot be modified, need to implement two types of Serialize() methods in the namespace BitSerializer. The first method responsible to serialize a value with key, the second - without. This is a basic concept of BitSerializer which helps to control at compile time the possibility the type serialization in a current level of archive. For example, you can serialize any type to a root level of JSON, but you can't do it with key. In other case, when you in the object scope of JSON, you can serialize values only with keys.

```cpp
#include <iostream>
#include "bitserializer\bit_serializer.h"
#include "bitserializer\archives\json_restcpp_archive.h"

class TestThirdPartyClass
{
public:
	TestThirdPartyClass(int x, int y)
		: x(x), y(y)
	{ }

	int x;
	int y;
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
			inline void Serialize(TArchive& archive)
			{
				archive << MakeKeyValue("x", value.x);
				archive << MakeKeyValue("y", value.y);
			}

			TestThirdPartyClass& value;
		};
	}	// namespace Detail

	template<typename TArchive>
	inline void Serialize(TArchive& archive, const typename TArchive::key_type& key, TestThirdPartyClass& value)
    {
		auto serializer = Detail::TestThirdPartyClassSerializer(value);
		Serialize(archive, key, serializer);
	}
	template<typename TArchive>
	inline void Serialize(TArchive& archive, TestThirdPartyClass& value)
	{
		auto serializer = Detail::TestThirdPartyClassSerializer(value);
		Serialize(archive, serializer);
	}
}   // namespace BitSerializer

using namespace BitSerializer;

int main()
{
	auto simpleObj = TestThirdPartyClass(100, 200);
	auto result = SaveObject<JsonArchive>(simpleObj);
	return 0;
}
```

#### Serializing enum types
To be able to serialize enum types, you must register a map with string equivalents in the HEADER file.
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

#### Conditions for checking the serialization mode
To check the current serialization mode, use two methods - IsLoading() and IsSaving(). They are haven't CPU overhead, because they are «constexpr».
```cpp
class Foo
public:
    template <class TArchive>
    inline void Serialize(TArchive& archive)
    {
    	if constexpr (archive.IsLoading()) {
	        // Code which executes in loading mode
	    }
	    else {
    		// Code which executes in saving mode
    	}
	
    	if constexpr (archive.IsSaving()) {
		    // Code which executes in saving mode
	    }
	    else {
    		// Code which executes in loading mode
    	}
    }
}
```

#### Validation of deserialized values

BitSerializer allows to add an arbitrary number of validation rules to the named values, the syntax is quite simple:
```cpp
archive << MakeKeyValue("TestFloat", TestFloat, Required(), Range(-1.0f, 1.0f));
```
After deserialize, you can check the status in context and get errors:
```cpp
if (!Context.IsValid())
{
    const auto& validationErrors = Context.GetValidationErrors();
}
```
Basically implemented few validators: Required, Range, MinSize, MaxSize.
Validator 'Range' can be used with all types which have operators '<' and '>'.
Validators 'MinSize' and 'MaxSize' can be applied to all values which have size() method.
This list will be extended in future.

Below real example:
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/archives/json_restcpp_archive.h"

using namespace BitSerializer;

class TestSimpleClass
{
public:
	TestSimpleClass() { }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeKeyValue("TestInt", TestInt, Required(), Range(0, 100));
		archive << MakeKeyValue("TestFloat", TestFloat, Required(), Range(-1.0f, 1.0f));
		archive << MakeKeyValue("TestString", TestString, MaxSize(8));
	};

private:
	int TestInt;
	float TestFloat;
	std::wstring TestString;
};

int main()
{
	auto simpleObj = TestSimpleClass();
	LoadObject<JsonArchive>(simpleObj, L"{	\"TestInt\": 2000, \"TestString\" : \"Very looooooooong string!\"  }");
	if (!Context.IsValid())
	{
		std::wcout << L"Validation errors: "<< std::endl;
		const auto& validationErrors = Context.GetValidationErrors();
		for (const auto& keyErrors : validationErrors)
		{
			std::wcout << L"Path: " << keyErrors.first << std::endl;
			for (const auto& err : keyErrors.second)
			{
				std::wcout << L"\t" << err << std::endl;
			}
		}
	}
	return 0;
}
```
The result of execution this code:
```text
Validation errors:
Path: /TestFloat
        This field is required
Path: /TestInt
        Value must be between 0 and 100
Path: /TestString
        The maximum size of this field should be not greater than 8
```
Returned paths for invalid values is dependent to archive type, in this sample it's JSON Pointer (RFC 6901).

#### Compile time checking
The new C++ 17 ability «if constexpr» helps to generate clear error messages.
If you try to serialize an object that is not supported at the current level of the archive, you will receive a simple error message.
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
