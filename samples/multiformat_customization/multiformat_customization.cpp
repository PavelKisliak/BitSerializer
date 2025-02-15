#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
#include "bitserializer/pugixml_archive.h"

using namespace BitSerializer;
using XmlArchive = BitSerializer::Xml::PugiXml::XmlArchive;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

class CPoint
{
public:
	CPoint(int x, int y)
		: x(x), y(y)
	{ }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		// Serialize as attributes when archive type is XML
		if constexpr (TArchive::archive_type == ArchiveType::Xml)
		{
			archive << AttributeValue("x", x);
			archive << AttributeValue("y", y);
		}
		else
		{
			archive << KeyValue("x", x);
			archive << KeyValue("y", y);
		}
	}

	int x, y;
};

int main()	// NOLINT(bugprone-exception-escape)
{
	auto testObj = CPoint(100, 200);

	const auto jsonResult = BitSerializer::SaveObject<JsonArchive>(testObj);
	std::cout << "JSON: " << jsonResult << std::endl;

	// Used explicitly defined root node name "Point" (to avoid auto-generated name "root")
	const auto xmlResult = BitSerializer::SaveObject<XmlArchive>(KeyValue("Point", testObj));
	std::cout << "XML: " << xmlResult << std::endl;
	return 0;
}