#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

class TestThirdPartyClass
{
public:
	TestThirdPartyClass(int x, int y)
		: x(x), y(y)
	{ }

	int x;

	int GetY() const noexcept { return y; }
	void SetY(int y) noexcept { this->y = y; }

private:
	int y;
};

template<typename TArchive>
void SerializeObject(TArchive& archive, TestThirdPartyClass& testThirdPartyClass)
{
	// Serialize public property
	archive << KeyValue("x", testThirdPartyClass.x);

	// Serialize private property
	if constexpr (TArchive::IsLoading()) {
		int y = 0;
		archive << KeyValue("y", y);
		testThirdPartyClass.SetY(y);
	}
	else {
		const int y = testThirdPartyClass.GetY();
		archive << KeyValue("y", y);
	}
}


int main()
{
	auto testObj = TestThirdPartyClass(100, 200);
	const auto result = BitSerializer::SaveObject<JsonArchive>(testObj);
	std::cout << result << std::endl;
	return 0;
}
