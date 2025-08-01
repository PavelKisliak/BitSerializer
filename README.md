# BitSerializer ![Generic badge](https://img.shields.io/badge/Release-v0.80-blue) [![Vcpkg Version](https://img.shields.io/vcpkg/v/bitserializer?color=blue)](https://vcpkg.link/ports/bitserializer) [![Conan Center](https://img.shields.io/conan/v/bitserializer?color=blue)](https://conan.io/center/recipes/bitserializer) [![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](license.txt) [![Build Status](https://dev.azure.com/real0793/BitSerializer/_apis/build/status%2FGitHub-BitSerializer?branchName=master)](https://dev.azure.com/real0793/BitSerializer/_build/latest?definitionId=5&branchName=master)

___

### Main features:
- One common interface allows easy switching between formats JSON, XML, YAML, and MsgPack.
- Modular architecture lets you include only the serialization archives you need.
- Functional serialization style similar to the Boost library.
- Support loading named fields in any order with conditional logic to preserve model compatibility.
- Customizable validation produces a detailed list of errors for deserialized values.
- Seamless handling of optional and required fields, bypassing the need to use `std::optional`.
- Configurable set of policies to control overflow and type mismatch errors.
- Serialization support for almost all STD containers and types (including Unicode strings like `std::u16string`).
- Enums can be serialized as integers or strings, giving you full control over representation.
- Support serialization to memory, streams and files.
- Full Unicode support with automatic detection and transcoding (except YAML).
- A powerful [string conversion submodule](docs/bitserializer_convert.md) supports enums, classes, chrono types, and UTF encoding.

#### Supported formats:
| Component | Format | Encoding | Pretty format | Based on |
| ------ | ------ | ------ |:------:| ------ |
| [rapidjson-archive](docs/bitserializer_rapidjson.md) | JSON | UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE | ✅ | [RapidJson](https://github.com/Tencent/rapidjson) |
| [pugixml-archive](docs/bitserializer_pugixml.md) | XML | UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE | ✅ | [PugiXml](https://github.com/zeux/pugixml) |
| [rapidyaml-archive](docs/bitserializer_rapidyaml.md) | YAML | UTF-8 | N/A | [RapidYAML](https://github.com/biojppm/rapidyaml) |
| [csv-archive](docs/bitserializer_csv.md) | CSV | UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE | N/A | Built-in |
| [msgpack-archive](docs/bitserializer_msgpack.md) | MsgPack | Binary | N/A | Built-in |

#### Requirements:
 - C++ 17 (VS 2019, GCC-8, CLang-8, AppleCLang-12).
 - Supported platforms: Windows, Linux, MacOS (x86, x64, arm32, arm64, arm64be¹).
 - JSON, XML and YAML archives are based on third-party libraries (there are plans to reduce dependencies).

 ¹ Versions of the RapidYaml base library less than v0.7.1 may be unstable on ARM architecture.

#### Limitations:
 - Work without exceptions is not supported.

___
## Table of contents
- [Hello world](#hello-world)
- [Performance overview](#performance-overview)
- [How to install](#how-to-install)
- [Unicode support](#unicode-support)
- [Serializing class](#serializing-class)
- [Serializing base class](#serializing-base-class)
- [Serializing third party class](#serializing-third-party-class)
- [Serializing a class that represents an array](#serializing-a-class-that-represents-an-array)
- [Serializing custom class representing a string](#serializing-custom-class-representing-a-string)
- [Serializing enum types](#serializing-enum-types)
- [Serializing to multiple formats](#serializing-to-multiple-formats)
- [Serialization STD types](#serialization-std-types)
- [Specifics of serialization STD map](#specifics-of-serialization-std-map)
- [Serialization date and time](#serialization-date-and-time)
- [Conditional loading and versioning](#conditional-loading-and-versioning)
- [Serialization to streams and files](#serialization-to-streams-and-files)
- [Error handling](#error-handling)
- [Validation of deserialized values](#validation-of-deserialized-values)
- [Compile time checking](#compile-time-checking)
- [What else to read](#what-else-to-read)
- [Thanks](#thanks)

___

### Hello world
Let's get started with a traditional "Hello world!" example that demonstrates BitSerializer's serialization features, such as validation (e.g., email and phone number formats), handling optional fields, and converting between different data formats (JSON to CSV). The example highlights the flexibility of the library in handling different data types, including `std::chrono` and Unicode strings, and ensures data integrity through required and optional field constraints.
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
#include "bitserializer/csv_archive.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/types/std/chrono.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;
using CsvArchive = BitSerializer::Csv::CsvArchive;

struct CUser
{
    // Mandatory fields
    uint64_t Id = 0;
    std::u16string Name;
    std::chrono::system_clock::time_point Birthday;
    std::string Email;
    // Optional fields (maybe absent or `null` in the source JSON)
    std::string PhoneNumber;
    std::u32string NickName;

    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        archive << KeyValue("Id", Id, Required());
        // Using the `Required()` validator with a custom error message (can be ID of localization string)
        archive << KeyValue("Birthday", Birthday, Required("Birthday is required"));
        archive << KeyValue("Name", Name, Required(), Validate::MaxSize(32));
        archive << KeyValue("Email", Email, Required(), Validate::Email());
        // Optional field (should be empty or contain a valid phone number)
        archive << KeyValue("PhoneNumber", PhoneNumber, Validate::PhoneNumber());
        archive << KeyValue("NickName", NickName);
    }
};

int main()  // NOLINT(bugprone-exception-escape)
{
    const char* sourceJson = R"([
{ "Id": 1, "Birthday": "1998-05-15T00:00:00Z", "Name": "John Doe", "Email": "john.doe@example.com", "PhoneNumber": "+(123) 4567890", "NickName": "JD" },
{ "Id": 2, "Birthday": "1993-08-20T00:00:00Z", "Name": "Alice Smith", "Email": "alice.smith@example.com", "PhoneNumber": "+(098) 765-43-21", "NickName": "Ali" },
{ "Id": 3, "Birthday": "2001-03-10T00:00:00Z", "Name": "Bob Johnson", "Email": "bob.johnson@example.com", "PhoneNumber": null }
])";

    // Load list of users from JSON
    std::vector<CUser> users;
    BitSerializer::LoadObject<JsonArchive>(users, sourceJson);

    // Save to CSV
    std::string csv;
    BitSerializer::SaveObject<CsvArchive>(users, csv);

    std::cout << csv << std::endl;
    return EXIT_SUCCESS;
}
```
Example output:
```
Id,Birthday,Name,Email,PhoneNumber,NickName
1,1998-05-15T00:00:00.0000000Z,John Doe,john.doe@example.com,+(123) 4567890,JD
2,1993-08-20T00:00:00.0000000Z,Alice Smith,alice.smith@example.com,+(098) 765-43-21,Ali
3,2001-03-10T00:00:00.0000000Z,Bob Johnson,bob.johnson@example.com,,
```
BitSerializer treats missing fields as optional by default, but you can enforce mandatory fields using the `Required()` validator. This approach eliminates the need for workarounds like using `std::optional`, simplifying your business logic by avoiding repetitive checks for value existence. The library's robust validation system collects all invalid fields during deserialization, enabling comprehensive error reporting (with localization support if needed). Additionally, BitSerializer ensures type safety by throwing exceptions for type mismatches or overflow errors, such as when deserializing values that exceed the capacity of the target type.

### Performance overview
BitSerializer prioritizes reliability and usability, but we understand that performance remains a critical factor for serialization libraries.
This chapter provides an overview of the performance characteristics of BitSerializer across various serialization formats, as well as comparative tests with the used third-party libraries.

#### Key performance insights
- Formats implemented natively in BitSerializer (MsgPack and CSV) demonstrate excellent performance due to their DOM-free architecture. This approach eliminates intermediate object tree construction, enabling direct serialization/deserialization to/from streams.
- Formats relying on external libraries (RapidJSON, PugiXML, RapidYAML) show an average performance loss of ~5% compared to their native APIs. This minor trade-off is due to the unified BitSerializer abstraction layer, which provides consistent behavior across all supported formats.
- All formats support non-linear loading of named fields, but maximum performance can be achieved when loading in the same order. This feature is important for compatibility and flexibility when working with complex models (e.g. for updating models).

#### Comparing "Parsers" and "Serializers" library classes
It is important to note that comparing "Serialization" classes (like BitSerializer) with "Parser" classes (such as RapidJSON, NlohmannJson, PugiXML, or RapidYAML) may not always be entirely fair. These two categories of libraries differ fundamentally in their design and purpose:
- **Parsers:** Typically operate on a DOM-based model, where the entire document is loaded into memory before processing. This approach is well-suited for tasks requiring extensive manipulation of the data structure but can introduce overhead during serialization and deserialization.
- **Serializers:** Focus on streaming serialization, where data is processed incrementally without the need to build an intermediate DOM. This approach is generally faster and more memory-efficient but may lack some of the advanced manipulation features offered by parsers.

In this performance analysis, we have benchmarked BitSerializer against the base libraries it relies on (e.g., RapidJSON, PugiXML, and RapidYAML).
These libraries are primarily "Parser" classes, and the performance differences observed reflect the inherent trade-offs between DOM-based parsing and streaming serialization. 

We understand that comparing "Serialization" classes with "Parser" classes might not always be equitable due to the fundamental differences in their nature (e.g., DOM vs. streaming serialization).
However, this comparison provides valuable insights into how BitSerializer performs relative to the libraries it builds upon.
In the future, we plan to include benchmarks against other libraries from the "Serializers" class to offer a more direct comparison. 

#### Comparison of serialized data size
In addition to performance metrics, the size of the serialized output is another important factor to consider when choosing a serialization format.
Below is a comparison of the serialized output sizes (in bytes) for the same test model using different formats and libraries:

![image info](benchmarks/archives/benchmark_results/serialization_output_size_chart.png)

Binary formats like MsgPack produce significantly smaller outputs compared to text-based formats like JSON, XML, or YAML.
The CSV format is the most compact among all tested formats, making it an excellent choice for storage and transmission of tabular data.

#### Performance test methodology
- ***Metrics:*** To evaluate the performance of BitSerializer, we measure the number of fields processed per millisecond (`fields/ms`) during serialization and deserialization. This metric allows us to objectively compare the efficiency of different formats and libraries.
- **Test model:** The [test model](benchmarks/archives/test_model.h) consists of an array of objects containing various data types compatible with all supported formats. This ensures a fair comparison of formats since the same data structure is used for all tests.

#### Performance test results
![image info](benchmarks/archives/benchmark_results/serialization_speed_chart.png)

For most applications, BitSerializer provides the optimal combination of reliability, feature completeness, and performance. Developers working with MsgPack/CSV will see best-in-class speeds, while users needing JSON/XML/YAML benefit from consistent performance with minimal overhead compared to format-specific libraries.

### How to install
Some archives (JSON, XML and YAML) require third-party libraries, but you can install only the ones which you need.
The easiest way is to use one of supported package managers, in this case, third-party libraries will be installed automatically.
Please follow [instructions](#what-else-to-read) for specific archives.

#### VCPKG
Just add BitSerializer to manifest file (`vcpkg.json`) in your project:
```json
{
    "dependencies": [
        {
            "name": "bitserializer",
            "features": [ "rapidjson-archive", "pugixml-archive", "rapidyaml-archive", "csv-archive", "msgpack-archive" ]
        }
    ]
}
```
The latest available version: [![Vcpkg Version](https://img.shields.io/vcpkg/v/bitserializer?color=blue)](https://vcpkg.link/ports/bitserializer)

Enumerate features which you need, by default all are disabled. Use like as usual in the [Cmake](#how-to-use-with-cmake).

Alternatively, you can install the library via the command line:
```shell
> vcpkg install bitserializer[rapidjson-archive,pugixml-archive,rapidyaml-archive,csv-archive,msgpack-archive]
```
In the square brackets enumerated all available formats, install only which you need.

#### Conan 2
The recipe of BitSerializer is available on [Conan-center](https://github.com/conan-io/conan-center-index), just add BitSerializer to `conanfile.txt` in your project and enable archives which you need via options (by default all are disabled):
```
[requires]
bitserializer/x.xx

[options]
bitserializer/*:with_rapidjson=True
bitserializer/*:with_pugixml=True
bitserializer/*:with_rapidyaml=True
bitserializer/*:with_csv=True
bitserializer/*:with_msgpack=True
```
Replace `x.xx` with the latest available version: [![Conan Center](https://img.shields.io/conan/v/bitserializer?color=blue)](https://conan.io/center/recipes/bitserializer)

#### Installation via CMake on a Unix system
```sh
$ git clone https://github.com/PavelKisliak/BitSerializer.git
$ # Enable only archives which you need (by default all are disabled)
$ cmake bitserializer -B bitserializer/build -DBUILD_RAPIDJSON_ARCHIVE=ON -DBUILD_PUGIXML_ARCHIVE=ON -DBUILD_RAPIDYAML_ARCHIVE=ON -DBUILD_CSV_ARCHIVE=ON -DBUILD_MSGPACK_ARCHIVE=ON
$ sudo cmake --build bitserializer/build --config Debug --target install
$ sudo cmake --build bitserializer/build --config Release --target install
```
By default, will be built a static library, add the CMake parameter `-DBUILD_SHARED_LIBS=ON` to build shared.
Make sure your application and library are compiled with the same options (C++ standard, optimization flags, runtime type, etc.) to avoid binary incompatibility issues.
You will also need to install dev-packages of base libraries (CSV and MsgPack archives do not require any dependencies), currently available only `rapidjson-dev` and `libpugixml-dev`, the RapidYaml library needs to be compiled manually.

> [!IMPORTANT]
> Make sure your application and library are compiled with the same options (C++ standard, optimization flags, runtime type, etc.) to avoid binary incompatibility issues.

#### How to use with CMake
```cmake
find_package(bitserializer CONFIG REQUIRED)
# Link only archives which you need
target_link_libraries(${PROJECT_NAME} PRIVATE
    BitSerializer::rapidjson-archive
    BitSerializer::pugixml-archive
    BitSerializer::rapidyaml-archive
    BitSerializer::csv-archive
    BitSerializer::msgpack-archive
)
```

### Unicode support
BitSerializer provides comprehensive Unicode support by enabling serialization of any `std::basic_string` type (e.g., `std::u8string`, `std::u16string`, `std::u32string`) while automatically handling transcoding to the target output format. You can also use any string type as keys, but keep in mind that transcoding incurs additional processing overhead. For optimal performance, prefer UTF-8 strings, as they are natively supported by all archives and minimize transcoding costs. 

The example below demonstrates how BitSerializer seamlessly handles different string types and encodings: 
```cpp
class TestUnicodeClass
{
public:
    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        // Serialize a UTF-8 string with key in UTF-16
        archive << KeyValue(u"Utf16Key", mUtf8StringValue);

        // Serialize a UTF-16 string with key in UTF-32
        archive << KeyValue(U"Utf32Key", mUtf16StringValue);

        // Serialize a UTF-32 string with key in UTF-8
        archive << KeyValue(u8"Utf8Key", mUtf32StringValue);
    };

private:
    std::string mUtf8StringValue;       // UTF-8 encoded string
    std::u16string mUtf16StringValue;   // UTF-16 encoded string
    std::u32string mUtf32StringValue;   // UTF-32 encoded string
};
```
This flexibility allows you to work with various Unicode encodings without worrying about manual transcoding.
However, for best results, use UTF-8 consistently unless your application specifically requires other encodings.

### Serializing class
There are two ways to serialize a class:

  * Internal public method `Serialize()` - good way for your own classes.
  * External global function `SerializeObject()` - used for third party class (no access to sources).

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
        archive << KeyValue("TestBool", testBool);
        archive << KeyValue("TestString", testString);
        archive << KeyValue("TestTwoDimensionArray", testTwoDimensionArray);
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
For serializing a named object please use helper class `KeyValue` which takes `key` and `value` as constructor arguments. Usually the type of key is UTF-8 string, but you are free to use any other convertible type (`std::u16string`, `std::u32string` or any numeric types). For example, MsgPack archive has native support for numbers as keys, they will be converted to string when use with another archives. For get maximum performance, better to avoid any conversions.

### Serializing base class
To serialize the base class, use the helper method `BaseObject()`, like as in the next example.
```cpp
template <class TArchive>
void Serialize(TArchive& archive)
{
    archive << BaseObject<MyBaseClass>(*this);
    archive << KeyValue("TestInt", TestInt);
};
```
> [!NOTE]
> Version 0.75 and earlier support serialization of the base class only via the internal `Serialize()` method.

### Serializing third party class
As alternative for internal `Serialize()` method also exists approach with defining global functions, it will be useful in next cases:

 - Sources of serializing class cannot be modified (for example from third party library).
 - When class represents list of some values (such as `std::vector`), see [next chapter](#serializing-class-that-represent-an-array).
 - When you strongly follow single responsibility principle and wouldn't like to include serialization code into class.

> [!NOTE]
> Internal `Serialize()` method has higher priority than global one (in v0.75 was a priority for the global function).

You need to implement `SerializeObject()` in the same namespace as the serializing class, or in `BitSerializer`:
```cpp
class TestThirdPartyClass
{
public:
    TestThirdPartyClass(int x, int y) noexcept
        : x(x), y(y)
    { }

    // Example of public property
    int x;

    // Example of property that is only accessible via a getter/setter
    [[nodiscard]] int GetY() const noexcept { return y; }
    void SetY(const int inY) noexcept { this->y = inY; }

private:
    int y;
};

// Serializes TestThirdPartyClass.
template<typename TArchive>
void SerializeObject(TArchive& archive, TestThirdPartyClass& testThirdPartyClass)
{
    // Serialize public property
    archive << KeyValue("x", testThirdPartyClass.x);

    // Serialize private property
    if constexpr (TArchive::IsLoading())
    {
        int y = 0;
        archive << KeyValue("y", y);
        testThirdPartyClass.SetY(y);
    }
    else
    {
        const int y = testThirdPartyClass.GetY();
        archive << KeyValue("y", y);
    }
}
```
[See full sample](samples/serialize_third_party_class/serialize_third_party_class.cpp)

### Serializing a class that represents an array
In this chapter described how to serialize your own class that represent a list of values (similar to `std::vector`).
For this purpose, need to implement a global function `SerializeArray()` in the same namespace as the serializing class, or in `BitSerializer`.

Additionally, BitSerializer wants to know the number of elements in the list.
This is optional for a text archives like JSON, but mandatory for a binary archive like MsgPack since it stores the size prior the array elements.
The size of list can be obtained via one of the following ways:

 - Global function `size(const CMyArray&)` in the same namespace as the serializing class (highest priority).
 - Standard class method `size()`.
 - By enumerating array elements using iterators (like as for `std::forward_list`).

So, in case if your class has a different signature for the size getter than `size()`, then you need to implement it as a global function.

> [!WARNING]
> In the previous version of BitSerializer v0.75, was incorrect detecting internal `size()` method (if it's not in the `std` namespace).

Please take a look at the following example:
```cpp
// Some custom array type
template <typename T>
class CMyArray
{
public:
    CMyArray() = default;
    CMyArray(std::initializer_list<T> initList)
        : mArray(initList)
    { }

    [[nodiscard]] size_t GetSize() const noexcept { return mArray.size(); }
    void Resize(size_t newSize) { mArray.resize(newSize); }

    [[nodiscard]] const T& At(size_t index) const { return mArray.at(index); }
    [[nodiscard]] T& At(size_t index) { return mArray.at(index); }

    T& PushBack(T&& value) { return mArray.emplace_back(std::forward<T>(value)); }

private:
    std::vector<T> mArray;
};

// Returns the size of the CMyArray.
template <class T>
size_t size(const CMyArray<T>& cont) noexcept { return cont.GetSize(); }

// Serializes CMyArray.
template <class TArchive, class TValue>
void SerializeArray(TArchive& arrayScope, CMyArray<TValue>& cont)
{
    if constexpr (TArchive::IsLoading())
    {
        // Resize container when approximate size is known
        if (const auto estimatedSize = arrayScope.GetEstimatedSize(); estimatedSize != 0 && cont.GetSize() < estimatedSize) {
            cont.Resize(estimatedSize);
        }

        // Load
        size_t loadedItems = 0;
        for (; !arrayScope.IsEnd(); ++loadedItems)
        {
            TValue& value = (loadedItems < cont.GetSize()) ? cont.At(loadedItems) : cont.PushBack({});
            Serialize(arrayScope, value);
        }
        // Resize container for case when loaded items less than there are or were estimated
        cont.Resize(loadedItems);
    }
    else
    {
        for (size_t i = 0; i < cont.GetSize(); ++i)
        {
            Serialize(arrayScope, cont.At(i));
        }
    }
}
```
[See full sample](samples/serialize_custom_array/serialize_custom_array.cpp)

Additional recommendations:
 - Don't clear arrays, prefer loading values into existing elements (for better performance).
 - Resize array before loading if estimated size is not zero (but please keep in mind that the actual size may vary).
 - For fixed size arrays, always check the size of the array and the elements actually loaded (throw an exception if they differ).
 - Use [std containers serialization implementation](include/bitserializer/types/std) as examples.

### Serializing custom class representing a string
Most frameworks/engines have their own implementation of the string type, and most likely you will want to add support for serializing these types.
BitSerializer allows you to do this in a simple and efficient way by using `std::basic_string_view<>` as an intermediate type (supported any char type).

Let's imagine that you would like to implement serialization of your own `std::string` alternative, which is called `CMyString`.
For this purpose you would need two global functions in the same namespace as the serializing class, or in `BitSerializer`:
```cpp
template <class TArchive, typename TKey>
bool Serialize(TArchive& archive, TKey&& key, CMyString& value);

template <class TArchive>
bool Serialize(TArchive& archive, CMyString& value);
```
These two functions are necessary for serialization any type with and without **key** into the output archive.
For example, object in the JSON format, has named properties, but JSON-array can contain only values.

Additionally, you will need to implement string conversion methods (internal or global), please read more about ([convert sub-module](docs/bitserializer_convert.md)).
They will add support for using string types as keys, for example it will allow serialization of `std::map<CMyString, int>` where `CMyString` is used as a key.

This all looks a bit more complicated than serializing an object, but the code is pretty simple, please have a look at the example below:
```cpp
// Some custom string type
class CMyString
{
public:
    CMyString() = default;
    CMyString(const char* str) : mString(str) { }

    bool operator<(const CMyString& rhs) const { return this->mString < rhs.mString; }

    const char* data() const noexcept { return mString.data(); }
    size_t size() const noexcept { return mString.size(); }

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
    if constexpr (TArchive::IsLoading())
    {
        std::string_view stringView;
        if (Detail::SerializeString(archive, std::forward<TKey>(key), stringView))
        {
            value.FromString(stringView);
            return true;
        }
    }
    else
    {
        std::string_view stringView(value.data(), value.size());
        return Detail::SerializeString(archive, std::forward<TKey>(key), stringView);
    }
    return false;
}

// Serializes CMyString without key
template <class TArchive>
bool Serialize(TArchive& archive, CMyString& value)
{
    if constexpr (TArchive::IsLoading())
    {
        std::string_view stringView;
        if (Detail::SerializeString(archive, stringView))
        {
            value.FromString(stringView);
            return true;
        }
        return false;
    }
    else
    {
        std::string_view stringView(value.data(), value.size());
        return Detail::SerializeString(archive, stringView);
    }
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
Enum types can be serialized as integers or as strings, as you prefer.
By default, they serializing as strings, to serialize as integers, use the `EnumAsBin` wrapper:
```cpp
archive << KeyValue("EnumValue", EnumAsBin(enumValue));
```
To be able to serialize `enum` types as string, you need to register a map with string equivalents in the your HEADER file.
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
One of the advantages of BitSerializer is the ability to serialize into multiple formats through a single interface. The following example shows how to save an object to JSON and XML:
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
        archive << KeyValue("x", x);
        archive << KeyValue("y", y);
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
The serialization code differs only in the template parameter -  **JsonArchive** and **XmlArchive**.
But here are some moments which need comments. As you can see in the XML was created node with name "root". This is auto generated name when it was not specified explicitly for root node. The library does this just to smooth out differences in the structure of formats. But you are free to set name of root node if needed:
```cpp
const auto xmlResult = BitSerializer::SaveObject<XmlArchive>(KeyValue("Point", testObj));
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
            archive << KeyValue("x", x);
            archive << KeyValue("y", y);
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
BitSerializer has built-in serialization for all STD containers and most other commonly used types. For add support of required STD type just need to include related header file.
| Types  | Header |
| ------ | ------ |
| std::basic_string<>, std::pmr::basic_string<> | Part of the basic package |
| std::byte | Part of the basic package |
| std::atomic | #include "bitserializer/types/std/atomic.h" |
| std::array | #include "bitserializer/types/std/array.h" |
| std::vector, std::pmr::vector | #include "bitserializer/types/std/vector.h" |
| std::deque, std::pmr::deque | #include "bitserializer/types/std/deque.h" |
| std::bitset | #include "bitserializer/types/std/bitset.h" |
| std::list, std::pmr::list | #include "bitserializer/types/std/list.h" |
| std::forward_list, std::pmr::forward_list | #include "bitserializer/types/std/forward_list.h" |
| std::queue, std::priority_queue | #include "bitserializer/types/std/queue.h" |
| std::stack | #include "bitserializer/types/std/stack.h" |
| std::set, std::multiset, std::pmr::set, std::pmr::multiset | #include "bitserializer/types/std/set.h" |
| std::unordered_set, std::unordered_multiset,<br>std::pmr::unordered_set, std::pmr::unordered_multiset | #include "bitserializer/types/std/unordered_set.h" |
| std::map, std::multimap, std::pmr::map, std::pmr::multimap | #include "bitserializer/types/std/map.h" |
| std::unordered_map, std::unordered_multimap,<br>std::pmr::unordered_map, std::pmr::unordered_multimap | #include "bitserializer/types/std/unordered_map.h" |
| std::valarray | #include "bitserializer/types/std/valarray.h" |
| std::pair | #include "bitserializer/types/std/pair.h" |
| std::tuple | #include "bitserializer/types/std/tuple.h" |
| std::optional | #include "bitserializer/types/std/optional.h" |
| std::unique_ptr, std::shared_ptr | #include "bitserializer/types/std/memory.h" |
| std::chrono::time_point, std::chrono::time_point | #include "bitserializer/types/std/chrono.h" |
| std::time_t | #include "bitserializer/types/std/ctime.h" |
| std::filesystem::path | #include "bitserializer/types/std/filesystem.h" |

Few words about serialization smart pointers. There is no any system footprints in output archive, for example empty smart pointer will be serialized as `NULL` type in JSON or in any other suitable way for other archive types. When an object is loading into an empty smart pointer, it will be created, and vice versa, when the loaded object is `NULL` or does not exist, the smart pointer will be reset. Polymorphism are not supported you should take care about such types by yourself.

### Specifics of serialization STD map
BitSerializer does not add any system information when saving the map, for example serialization to JSON would look like this:
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
const std::string inputJson = R"([{"One":1,"Three":3,"Two":2},{"Five":5,"Four":4}])";
BitSerializer::LoadObject<JsonArchive>(testVectorOfMaps, inputJson);
```

Since all of the most well-known text formats (such as JSON) allow only text keys, BitSerializer attempts to convert the map key to a string (except binary formats like MsgPack).
Out of the box, the library supports all the fundamental types (e.g. `bool`, `int`, `float`) as well as some of the `std` ones (`filesystem::path`, `chrono::timepoint`, etc), but if you want to use your own type as the key, you need to implement the conversion to a string. There are several options with internal and external functions, see details [here](docs\bitserializer_convert.md). For example, you can implement two internal methods in your type:
```cpp
class YourCustomKey
{
    std::string ToString() const { }
    void FromString(std::string_view str)
}
```

### Serialization date and time
The  ISO 8601 standard was chosen as the representation for the date, time and duration for text type of archives (JSON, XML, YAML, CSV). The MsgPack archive has its own compact time format. For enable serialization of the `std::chrono` and `time_t`,  just include these headers:
```cpp
#include "bitserializer/types/std/chrono.h"
#include "bitserializer/types/std/ctime.h"
```

The following table contains all supported types with examples of string representations:

| Type | Format | Examples | References |
| ------ | ------ | ------ | ------ |
| `std::time_t` | YYYY-MM-DDThh:mm:ssZ | 1677-09-21T00:12:44Z<br>2262-04-11T23:47:16Z | [ISO 8601/UTC](https://en.wikipedia.org/wiki/ISO_8601) |
| `chrono::time_point` | [±]YYYY-MM-DDThh:mm:ss[.SSS]Z | 1872-01-01T04:55:32.021Z<br>2262-04-11T23:47:16Z<br>9999-12-31T23:59:59.999Z<br>+12376-01-20T00:00:00Z<br>-1241-06-23T00:00:00Z | [ISO 8601/UTC](https://en.wikipedia.org/wiki/ISO_8601)  |
| `chrono::duration` | [±]PnWnDTnHnMnS | P125DT55M41S<br>PT10H20.346S<br>P10DT25M<br>P35W5D | [ISO 8601/Duration](https://en.wikipedia.org/wiki/ISO_8601#Durations)  |

Time point notes:
- Only UTC representation is supported, fractions of a second are optional ([±]YYYY-MM-DDThh:mm:ss[.SSS]Z).
- ISO-8601 doesn't specify precision for fractions of second, BitSerializer supports up to 9 digits, which is enough for values with nanosecond precision.
- Both decimal separators (dot and comma) are supported for fractions of a second.
- According to standard, to represent years before 0000 or after 9999 uses additional '-' or '+' sign.
- The date range depends on the `std::chrono::duration` type, for example implementation of `system_clock` on Linux has range **1678...2262 years**.
- Keep in mind that `std::chrono::system_clock` has time point with different duration on Windows and Linux, prefer to store time in custom `time_point` if you need predictable range (e.g. `time_point<system_clock, milliseconds>`).
- According to the C++20 standard, the EPOCH date for `system_clock` types is considered as *1970-01-01 00:00:00 UTC* excluding leap seconds.
- For avoid mistakes, time points with **steady_clock**  type are not allowed due to floating EPOCH.
- Allowed rounding only fractions of seconds, in all other cases an exception is thrown (according to `OverflowNumberPolicy`).

Duration notes:
- Supported a sign character at the start of the string (ISO 8601-2 extension).
- Durations which contains years, month, or with base UTC (2003-02-15T00:00:00Z/P2M) are not allowed.
- The decimal fraction supported only for seconds part, maximum 9 digits.
- Both decimal separators (dot and comma) are supported for fractions of a second.
- Allowed rounding only fractions of seconds, in all other cases an exception is thrown (according to `OverflowNumberPolicy`).

Since `std::time_t` is equal to `int64_t`, need to use special wrapper `CTimeRef`, otherwise time will be serialized as number.
```cpp
template <class TArchive>
void Serialize(TArchive& archive)
{
    archive << KeyValue("Time", CTimeRef(timeValue));
}
```

### Conditional loading and versioning

The functional style of serialization used in BitSerializer has one advantage over the declarative one - you can write branches depending on the data.
To check the current serialization mode, use two static methods - `IsLoading()` and `IsSaving()`. As they are «constexpr», you will not have any overhead.
```cpp
class Foo
{
public:
    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        if constexpr (TArchive::IsLoading()) {
            // Code which executes in loading mode
        }
        else {
            // Code which executes in saving mode
        }
    }
}
```
This can be most useful when you need to support multiple versions of a model. By default, library does not add any system fields (like as a version of object), but it's not difficult to add version when you will need:
```cpp
// Old version of test object (no needs to keep old models, just as example)
struct TestUserV1
{
    std::string name;           // Deprecated, need to split to first and last name
    uint8_t age{};
    uint32_t lastOrderId{};     // Deprecated, need to remove

    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        archive << KeyValue("name", name, Required());
        archive << KeyValue("age", age);
        archive << KeyValue("lastOrderId", lastOrderId);
    }
};

// Actual model
struct TestUser
{
    // Introduce version field
    static constexpr int16_t CurrentVersion = 1;

    std::string firstName;
    std::string lastName;
    uint8_t age{};
    std::string country;

    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        // Load 'version' field if exists
        int16_t version = TArchive::IsSaving() ? CurrentVersion : 0;
        archive << KeyValue("version", version);

        if constexpr (TArchive::IsLoading())
        {
            if (version == 0)
            {
                // Import name from old format
                std::string name;
                archive << KeyValue("name", name, Required());
                const auto spacePos = name.find(' ');
                firstName = name.substr(0, spacePos);
                lastName = spacePos != std::string::npos ? name.substr(spacePos + 1) : "";
            }
            else
            {
                archive << KeyValue("firstName", firstName, Required());
                archive << KeyValue("lastName", lastName, Required());
            }
        }
        archive << KeyValue("age", age);
        archive << KeyValue("country", country);
    }
};

int main()
{
    // Save old version
    std::vector<TestUserV1> oldUsers {
        { "John Smith", 35, 1254 },
        { "Emily Roberts", 27, 4546 },
        { "James Murphy", 32, 10653 }
    };
    const auto archive = BitSerializer::SaveObject<MsgPackArchive>(oldUsers);

    // Loading with import to new version
    std::vector<TestUser> newUsers;
    BitSerializer::LoadObject<MsgPackArchive>(newUsers, archive);

    return 0;
}
```
[See full sample](samples/versioning/versioning.cpp)

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
        archive << KeyValue("x", x);
        archive << KeyValue("y", y);
    }

    int x = 0, y = 0;
};

int main()
{
    auto testObj = CPoint(100, 200);

    SerializationOptions serializationOptions;
    serializationOptions.streamOptions.encoding = Convert::Utf::UtfType::Utf8;
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
[See full sample](samples/serialize_to_stream/serialize_to_stream.cpp)

For save/load to files, BitSerializer provides the following functions (which are just wrappers of serialization methods to streams):
```cpp
template <typename TArchive, typename T, typename TString>
BitSerializer::SaveObjectToFile<TArchive>(T&& object, TString&& path, const SerializationOptions& serializationOptions = DefaultOptions, bool overwrite = false);

template <typename TArchive, typename T, typename TString>
BitSerializer::LoadObjectFromFile<TArchive>(T&& object, TString&& path, const SerializationOptions& serializationOptions = DefaultOptions);
```

> [!NOTE]
> Note that the stream implementation must support the `seekg()` operation to load fields non-linearly.

### Error handling
First, let's list what are considered as errors and will throw exception:

 - Syntax errors in the input source (e.g. JSON)
 - When one or more user's validation rules were not passed
 - When a type from the archive (source format, like JSON) does not match to the target value (can be configured via `MismatchedTypesPolicy`)
 - When an enum type is not registered or its value is invalid
 - When size of target type is not enough for loading value (can be configured via `OverflowNumberPolicy`)
 - When target array with fixed size does not match the number of loading items
 - Invalid configuration in the `SerializationOptions`
 - Input/output file can't be opened for read/write
 - UTF encoding/decoding errors (can be configured via `UtfEncodingErrorPolicy`)
 - Unsupported UTF encoding

By default, any missed field in the input format (e.g. JSON) is not treated as an error, but you can add `Required()` validator if needed.
You can handle `std::exception` just for log errors, but if you need to provide more detailed information to the user, you may need to handle the following exceptions:

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
archive << KeyValue("testFloat", testFloat, Required(), Validate::Range(-1.0f, 1.0f));
```
For handle validation errors, need to catch special exception `ValidationException`, it is thrown at the end of deserialization (when all errors have been collected).
By default, the number of errors is unlimited, but it can be set using `maxValidationErrors` in `SerializationOptions`.
The map of validation errors can be get by calling method `GetValidationErrors()` from the exception object, it contains paths to fields with errors lists.
The default error message can be overridden (you can also pass string ID for further localization):
```cpp
archive << KeyValue("Age", mAge, Required("Age is required"), Validate::Range(0, 150, "Age should be in the range 0...150"));
```

The list of validators "out of the box" is not so rich, but it will expand in the future.

| Signature           | Description   |
| ------------------- | --------------------- |
| `Required(errorMessage = nullptr)`        | Validates that field is deserialized |
| `Range(min, max, errorMessage = nullptr)`   | Validates range of value, can be applied for any type that has operators '<' and '>' (e.g. `std::chrono` types) |
| `MinSize(minSize, errorMessage = nullptr)`  | Validates that the container or string size is not less than the specified minimum |
| `MaxSize(maxSize, errorMessage = nullptr)`  | Validates that the size of a container or string does not exceed the specified maximum |
| `Email(errorMessage = nullptr)`           | The email validator, generally complies with the RFC standard with the exception of: quoted parts, comments, SMTPUTF8 and IP address as domain part |
| `PhoneNumber(minDigits = 7, maxDigits = 15, isPlusRequired = true, errorMessage = nullptr)` | The phone number validator, examples:<br>+555 (55) 555-55-55, (55) 555 55 55, 555 5 55 55 |

All validators are declared in the `BitSerializer::Validate` namespace, except `Required` which also has alias in the `BitSerializer`.
> [!NOTE]
> In the previously released v0.75, all validators were declared in the BitSerializer namespace.

Usage example:
```cpp
using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

class UserModel
{
public:
    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        archive << KeyValue("Id", mId, Required());
        archive << KeyValue("Age", mAge, Required("Age is required"), Validate::Range(0, 150, "Age should be in the range 0...150"));
        archive << KeyValue("FirstName", mFirstName, Required(), Validate::MaxSize(16));
        archive << KeyValue("LastName", mLastName, Required(), Validate::MaxSize(16));
        archive << KeyValue("Email", mEmail, Required(), Validate::Email());
        // Custom validation with lambda
        archive << KeyValue("NickName", mNickName, [](const std::string& value, bool isLoaded) -> std::optional<std::string>
        {
            // Loaded string should has text without spaces or should be NULL
            if (!isLoaded || value.find_first_of(' ') == std::string::npos) {
                return std::nullopt;
            }
            return "The field must not contain spaces";
        });
    }

private:
    uint64_t mId = 0;
    uint16_t mAge = 0;
    std::string mFirstName;
    std::string mLastName;
    std::string mEmail;
    std::string mNickName;
};

int main()
{
    UserModel user;
    const char* json = R"({ "Id": 12420, "Age": 500, "FirstName": "John Smith-Cotatonovich", "NickName": "Smith 2000", "Email": "smith 2000@mail.com" })";
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
        Age should be in the range 0...150
Path: /Email
        Invalid email address
Path: /FirstName
        The maximum size of this field should not exceed 16
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
    // Error    C2338   BitSerializer. The archive doesn't support serialize fundamental type without key on this level.
    archive << testBool;
    // Proper use
    archive << KeyValue("testString", testString);
};
```

### What else to read
Each of the supported archives has its own page with details (installation, features, samples, etc.):
- [JSON archive "bitserializer-rapidjson"](docs/bitserializer_rapidjson.md)
- [XML archive "bitserializer-pugixml"](docs/bitserializer_pugixml.md)
- [YAML archive "bitserializer-rapidyaml"](docs/bitserializer_rapidyaml.md)
- [CSV archive "bitserializer-csv"](docs/bitserializer_csv.md)
- [MsgPack archive "bitserializer-msgpack"](docs/bitserializer_msgpack.md)

Additionally, you may want to use the [string conversion submodule](docs/bitserializer_convert.md).

### Thanks
- Artsiom Marozau for developing an archive with support YAML.
- Andrey Mazhyrau for help with cmake scripts, fix GCC and Linux related issues.
- Alexander Stepaniuk for support and participation in technical discussions.
- Evgeniy Gorbachov for help with implementation STD types serialization.
- Mateusz Pusz for code review and useful advices.

----
MIT, Copyright (C) 2018-2025 by Pavel Kisliak, made in Belarus 🇧🇾
