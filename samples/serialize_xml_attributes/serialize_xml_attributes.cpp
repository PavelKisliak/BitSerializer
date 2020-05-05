#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer_pugixml/pugixml_archive.h"

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
		archive << MakeAutoAttributeValue("Type", mType);
		archive << MakeAutoAttributeValue("Width", mWidth);
		archive << MakeAutoAttributeValue("Height", mHeight);
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
	const auto result = BitSerializer::SaveObject<XmlArchive>(MakeAutoKeyValue("Shapes", Shapes));
	std::cout << result << std::endl;
	return 0;
}