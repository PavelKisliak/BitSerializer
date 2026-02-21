### [BitSerializer](../README.md) / XML

Supported load/save XML from:

- std::string: UTF-8
- std::stream: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE (with/without BOM)

The BitSerializer uses as low level library [PugiXml](https://github.com/zeux/pugixml) - one of fastest libraries for parse **XML**.

### How to install
The recommended way is to use one of supported package managers, but you can do it manually just via CMake commands (in this case you should take care of the dependencies yourself).
#### VCPKG
Add BitSerializer to manifest file (`vcpkg.json`) with `pugixml-archive` feature:
```json
{
    "dependencies": [
        {
            "name": "bitserializer",
            "features": [ "pugixml-archive" ]
        }
    ]
}
```
The latest available version: [![Vcpkg Version](https://img.shields.io/vcpkg/v/bitserializer?color=blue)](https://vcpkg.link/ports/bitserializer)

If your project is based on VS solution you can just include next header files for start use:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/pugixml_archive.h"
```
If you are using CMake, you need to link the library:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::pugixml-archive)
```
#### Conan 2
Add the BitSerializer recipe to `conanfile.txt` in your project and enable `with_pugixml` option:
```
[requires]
bitserializer/x.xx

[options]
bitserializer/*:with_pugixml=True
```
Replace `x.xx` with the latest available version: [![Conan Center](https://img.shields.io/conan/v/bitserializer?color=blue)](https://conan.io/center/recipes/bitserializer)

The dependent library **PugiXml** will be automatically installed.
Usage the library will be related to selected Conan generator, if your choice is `CMakeDeps`, than linking will be classic:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::pugixml-archive)
```

### Implementation details

#### Root node naming
XML requires a named root node. When no explicit name is provided, BitSerializer uses default names:
 - "root" for objects
 - "array" for arrays

```cpp
class CPoint
{
public:
    CPoint(const int x, const int y) : X(x), Y(y) { }

    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        archive << KeyValue("x", X);
        archive << KeyValue("y", Y);
    }

    int X, Y;
};

int main()
{
    CPoint point(10, 20);

    // Serialize object with defined name of root node
    auto result = BitSerializer::SaveObject<XmlArchive>(KeyValue("Point", point));
    std::cout << "XML with defined root name: " << result << std::endl;

    // Serialize object without defined name of root node
    result = BitSerializer::SaveObject<XmlArchive>(point);
    std::cout << "XML without defined root name: " << result << std::endl;

    return 0;
}
```
This example outputs to the console:
```
XML with defined root name: <?xml version="1.0"?><Point><x>10</x><y>20</y></Point>
XML without defined root name: <?xml version="1.0"?><root><x>10</x><y>20</y></root>
```

#### Key type adaptation
By default, **PugiXml** uses 8-bit chars for keys (nodes and attributes). With the global definition `PUGIXML_WCHAR_MODE`, the key type becomes `wchar_t`. BitSerializer automatically adapts keys to the target archive's key type.

#### Serialization attributes
The XML nodes perfectly fit the common BitSerializer interface, but serialization of attributes is format-specific.
BitSerializer provides two helper classes for this purpose:

##### `AttributeValue` (XML-only)
Forces serialization as an XML attribute. **Causes compile-time error if used with non-XML archives** (JSON, YAML, CSV, MsgPack).

```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/pugixml_archive.h"

using namespace BitSerializer;
using XmlArchive = BitSerializer::Xml::PugiXml::XmlArchive;

class CRectangle
{
public:
    CRectangle() = default;

    CRectangle(const int Width, const int Height)
        : mType("Rectangle")
        , mWidth(Width)
        , mHeight(Height)
    { }

    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        archive << AttributeValue("Type", mType);
        archive << AttributeValue("Width", mWidth);
        archive << AttributeValue("Height", mHeight);
    }

    std::string mType;
    int mWidth = 0;
    int mHeight = 0;
};

int main()
{
    std::vector<CRectangle> Shapes = {
        { 5, 10 },
        { 20, 5 },
        { 50, 25 }
    };
    const auto result = BitSerializer::SaveObject<XmlArchive>(KeyValue("Shapes", Shapes));
    std::cout << result << std::endl;
    return 0;
}
```
[See full sample](../samples/serialize_xml_attributes/serialize_xml_attributes.cpp)

##### PropertyValue (Multi-format safe)
Automatically adapts serialization behavior based on archive type and value convertibility:

 - **XML:** Serializes as attribute if type is convertible to string, otherwise as element
 - **JSON/YAML/CSV/MsgPack:** Always serializes as key-value pair

```cpp
template <class TArchive>
void Serialize(TArchive& archive)
{
    // Scalar types → XML attribute, JSON key
    archive << PropertyValue("Width", mWidth);
    archive << PropertyValue("Height", mHeight);

    // Complex types → XML element, JSON key (automatic fallback)
    archive << PropertyValue("Profile", mProfile);
}
```

**When to use which:**
| Approach       | XML         | JSON             | Use case                               |
| -------------- | ------------| -----------------| -------------------------------------- |
| KeyValue       | Element     | Key              | Default choice, predictable structure  |
| AttributeValue | Attribute   | ❌ Compile error | XML-specific code, explicit attributes |
| PropertyValue  | Attribute¹  | Key              | Multi-format, code generation, OpenAPI |

¹ Falls back to element for non-string-convertible types

### Pretty format
**PugiXml** supports human-readable output with indentation. BitSerializer exposes this functionality through `SerializationOptions`:
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/pugixml_archive.h"

using XmlArchive = BitSerializer::Xml::PugiXml::XmlArchive;
using namespace BitSerializer;

class CPoint
{
public:
    CPoint(const int x, const int y) : X(x), Y(y) { }

    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        archive << AttributeValue("x", X);
        archive << AttributeValue("y", Y);
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
    BitSerializer::SaveObject<XmlArchive>(KeyValue("Points", points), result, serializationOptions);
    std::cout << result << std::endl;

    return 0;
}
```
This code outputs to the console:
```xml
<?xml version="1.0"?>
<Points>
  <object x="10" y="20" />
  <object x="30" y="40" />
</Points>
```