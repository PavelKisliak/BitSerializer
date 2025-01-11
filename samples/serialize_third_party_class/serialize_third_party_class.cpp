#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

namespace MyApp
{
	class TestThirdPartyClass
	{
	public:
		TestThirdPartyClass(int x, int y) noexcept
			: x(x), y(y)
		{ }

		// Example of public property
		int x;

		// Example of property that is only accessible via a getter/setter
		int GetY() const noexcept { return y; }
		void SetY(int y) noexcept { this->y = y; }

	private:
		int y;
	};

	// Serializes TestThirdPartyClass.
	template<typename TArchive>
	void SerializeObject(TArchive& archive, TestThirdPartyClass& testThirdPartyClass)
	{
		// Serialize public property
		archive << KeyValue("x", testThirdPartyClass.x);

		// Serialize private property
		if constexpr (TArchive::IsLoading())
		{
			int y = 0;
			archive << KeyValue("y", y);
			testThirdPartyClass.SetY(y);
		}
		else
		{
			const int y = testThirdPartyClass.GetY();
			archive << KeyValue("y", y);
		}
	}
}

int main()
{
	auto testObj = MyApp::TestThirdPartyClass(100, 200);
	const auto result = BitSerializer::SaveObject<JsonArchive>(testObj);
	std::cout << result << std::endl;
	return 0;
}
