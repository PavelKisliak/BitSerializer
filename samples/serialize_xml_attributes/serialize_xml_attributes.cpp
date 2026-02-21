#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/pugixml_archive.h"

using namespace BitSerializer;
using XmlArchive = BitSerializer::Xml::PugiXml::XmlArchive;

/**
 * Example demonstrates XML-specific attribute serialization using AttributeValue.
 * 
 * Note: AttributeValue causes compile-time error with non-XML archives.
 * For multi-format code, consider using PropertyValue instead (see multiformat_customization.cpp).
 */
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
		// XML-only: forces attribute serialization (compile error for JSON/YAML/CSV/MsgPack)
		archive << AttributeValue("Type", mType);
		archive << AttributeValue("Width", mWidth);
		archive << AttributeValue("Height", mHeight);

		// Alternative for multi-format code:
		// archive << PropertyValue("Type", mType);
		// archive << PropertyValue("Width", mWidth);
		// archive << PropertyValue("Height", mHeight);
	}

	std::string mType;
	int mWidth = 0;
	int mHeight = 0;
};

int main()	// NOLINT(bugprone-exception-escape)
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
