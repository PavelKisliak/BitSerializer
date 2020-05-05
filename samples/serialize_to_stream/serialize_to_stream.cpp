#include <iostream>
#include <sstream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

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