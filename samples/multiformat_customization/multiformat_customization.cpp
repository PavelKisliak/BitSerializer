#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
#include "bitserializer/pugixml_archive.h"

using namespace BitSerializer;
using XmlArchive = BitSerializer::Xml::PugiXml::XmlArchive;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

/**
 * Option 1: Manual format detection (explicit control)
 *
 * Use when you need full control over serialization structure per format.
 * More verbose but explicit about format-specific behavior.
 */
class CPointExplicit
{
public:
	CPointExplicit(int x, int y) : x(x), y(y) {}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
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

/**
 * Option 2: Smart wrapper (recommended for most cases)
 *
 * PropertyValue automatically adapts based on archive type and value convertibility:
 * - XML + string-convertible type → Attribute
 * - XML + non-convertible type → Element (KeyValue)
 * - Other formats → Key-Value pair
 *
 * Ideal for code generation (OpenAPI) and multi-format APIs.
 */
class CPointSmart
{
public:
	CPointSmart(int x, int y) : x(x), y(y) {}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		// Automatic adaptation - no if constexpr needed
		archive << PropertyValue("x", x);
		archive << PropertyValue("y", y);
	}

	int x, y;
};

int main()  // NOLINT(bugprone-exception-escape)
{
	auto testObjExplicit = CPointExplicit(100, 200);
	auto testObjSmart = CPointSmart(100, 200);

	// JSON output (both approaches produce identical result)
	const auto jsonResultExplicit = BitSerializer::SaveObject<JsonArchive>(testObjExplicit);
	const auto jsonResultSmart = BitSerializer::SaveObject<JsonArchive>(testObjSmart);
	std::cout << "JSON (explicit): " << jsonResultExplicit << std::endl;
	std::cout << "JSON (smart):    " << jsonResultSmart << std::endl;

	// XML output (both approaches produce identical result)
	const auto xmlResultExplicit = BitSerializer::SaveObject<XmlArchive>(KeyValue("Point", testObjExplicit));
	const auto xmlResultSmart = BitSerializer::SaveObject<XmlArchive>(KeyValue("Point", testObjSmart));
	std::cout << "XML (explicit):  " << xmlResultExplicit << std::endl;
	std::cout << "XML (smart):     " << xmlResultSmart << std::endl;

	return 0;
}
