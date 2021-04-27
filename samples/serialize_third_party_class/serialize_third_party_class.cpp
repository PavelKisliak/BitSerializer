#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"

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
	using namespace BitSerializer;

	// Serialize public property
	archive << MakeAutoKeyValue("x", testThirdPartyClass.x);

	// Serialize private property
	if constexpr (TArchive::IsLoading()) {
		int y = 0;
		archive << MakeAutoKeyValue("y", y);
		testThirdPartyClass.SetY(y);
	}
	else {
		const int y = testThirdPartyClass.GetY();
		archive << MakeAutoKeyValue("y", y);
	}
}


using namespace BitSerializer::Json::RapidJson;

int main()
{
	auto testObj = TestThirdPartyClass(100, 200);
	const auto result = BitSerializer::SaveObject<JsonArchive>(testObj);
	std::cout << result << std::endl;
	return 0;
}
