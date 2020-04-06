### [BitSerializer](../README.md) / XML

Supported load/save XML from:

- std::string: UTF-8
- std::stream: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE (with/without BOM)

The BitSerializer uses as low level library [PugiXml](https://github.com/zeux/pugixml) - one of fastest libraries for parse **XML**.

#### How to install (VCPKG)
```shell
vcpkg install bitserializer-pugixml:x64-windows
```
Specify required triplet for your platform instead of "x64-windows".
After installation packages just include next files:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer_pugixml/pugixml_archive.h"
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
		archive << MakeKeyValue("x", X);
		archive << MakeKeyValue("y", Y);
	}

	int X, Y;
};

int main()
{
	CPoint point(10, 20);

	// Serialize object with defined name of root node
	auto result = BitSerializer::SaveObject<XmlArchive>(MakeKeyValue("Point", point));
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

By default, **PugiXml** uses 8-bit chars as keys (for nodes and attributes), but with global definition **PUGIXML_WCHAR_MODE** the key type will be **wchar_t** (and BitSerializer also will need the same key type too). You can use universal adapters such **MakeAutoKeyValue()** and **MakeAutoAttributeValue()** for do not care about key types but with possible performance penalty for conversion.
```cpp
	void Serialize(TArchive& archive)
	{
		archive << MakeAutoAttributeValue("Foo", foo);
		archive << MakeAutoKeyValue("bar", bar);
	}
```

#### Serialization attributes
The XML nodes perfectly fits to common BitSerialazer interface, but serialization attributes is a bit specific, for support them, BitSerializer has one more helper function - **MakeAttributeValue()**.
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer_pugixml/pugixml_archive.h"

using namespace BitSerializer::Xml::PugiXml;
using namespace BitSerializer;

class CRectangle
{
public:
	CRectangle(const int Width, const int Height)
		: mType("Rectangle")
		, mWidth(Width)
		, mHeight(Height)
	{ }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeAutoAttributeValue("Type", mType);
		archive << MakeAutoAttributeValue("Width", mWidth);
		archive << MakeAutoAttributeValue("Height", mHeight);
	}

	std::string mType;
	int mWidth;
	int mHeight;
};

int main()
{
	std::vector<CRectangle> Shapes = {
		{ 5, 10 },
		{ 20, 5 },
		{ 50, 25 }
	};
	const auto result = BitSerializer::SaveObject<XmlArchive>(MakeAutoKeyValue("Shapes", Shapes));
	std::cout << result << std::endl;
	return 0;
}
```
[See full sample](samples/serialize_xml_attributes/serialize_xml_attributes.cpp)

### Pretty format
As base library (PugiXml) has the functionality for output to human readable format, the BitSerializer also allows to do this:
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer_pugixml/pugixml_archive.h"

using namespace BitSerializer::Xml::PugiXml;
using namespace BitSerializer;

class CPoint
{
public:
	CPoint(const int x, const int y) : X(x), Y(y) { }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeAttributeValue("x", X);
		archive << MakeAttributeValue("y", Y);
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
	BitSerializer::SaveObject<XmlArchive>(MakeKeyValue("Points", points), result, serializationOptions);
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