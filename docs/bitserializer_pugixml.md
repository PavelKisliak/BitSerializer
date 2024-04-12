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
#### Conan
Add the BitSerializer recipe to `conanfile.txt` in your project and enable `with_pugixml` option:
```
[requires]
bitserializer/0.50

[options]
bitserializer:with_pugixml=True
```
The dependent library **PugiXml** will be automatically installed.
Usage the library will be related to selected Conan generator, if your choice is `cmake_find_package_multi`, than linking will be classic:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::pugixml-archive)
```

### Implementation detail
XML format requires root named node, but BitSerializer allows to serialize objects with and without keys.
When name for root node was not provided, BitSerializer uses default name "**root**" for objects and "**array**" for arrays.
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

By default, **PugiXml** uses 8-bit chars as keys (for nodes and attributes), but with global definition **PUGIXML_WCHAR_MODE** the key type will be **wchar_t**. In any case, BitSerializer adapts the key to the target archive.

#### Serialization attributes
The XML nodes perfectly fits to common BitSerialazer interface, but serialization attributes is a bit specific, for support them, BitSerializer has one more helper class - `AttributeValue`.
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

### Pretty format
As base library (PugiXml) has the functionality for output to human readable format, the BitSerializer also allows to do this:
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