#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"

class TestThirdPartyClass
{
public:
	TestThirdPartyClass(int x, int y)
		: x(x), y(y)
	{ }

	int x, y;
};

namespace BitSerializer
{
	namespace Detail
	{
		class TestThirdPartyClassSerializer
		{
		public:
			TestThirdPartyClassSerializer(TestThirdPartyClass& value)
				: value(value)
			{ }

			template <class TArchive>
			void Serialize(TArchive& archive)
			{
				archive << MakeAutoKeyValue(L"x", value.x);
				archive << MakeAutoKeyValue(L"y", value.y);
			}

			TestThirdPartyClass& value;
		};
	}	// namespace Detail

	template<typename TArchive, typename TKey>
	void Serialize(TArchive& archive, TKey&& key, TestThirdPartyClass& value)
	{
		auto serializer = Detail::TestThirdPartyClassSerializer(value);
		Serialize(archive, key, serializer);
	}
	template<typename TArchive>
	void Serialize(TArchive& archive, TestThirdPartyClass& value)
	{
		auto serializer = Detail::TestThirdPartyClassSerializer(value);
		Serialize(archive, serializer);
	}
}	// namespace BitSerializer


using namespace BitSerializer::Json::RapidJson;

int main()
{
	auto testObj = TestThirdPartyClass(100, 200);
	const auto result = BitSerializer::SaveObject<JsonArchive>(testObj);
	std::cout << result << std::endl;
	return 0;
}