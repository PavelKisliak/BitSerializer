#include "bitserializer/bit_serializer.h"
#include "bitserializer_json_restcpp/json_restcpp_archive.h"

using namespace BitSerializer;
using namespace BitSerializer::Json::CppRest;

class TestSimpleClass
{
public:
	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeKeyValue(_XPLATSTR("TestBool"), mTestBool, Required());
		archive << MakeKeyValue(_XPLATSTR("TestInt"), mTestInt, Required(), Range(0, 100));
		archive << MakeKeyValue(_XPLATSTR("TestDouble"), mTestDouble, Required(), Range(-1.0, 1.0));
		archive << MakeKeyValue(_XPLATSTR("TestString"), mTestString, MaxSize(8));
	};

private:
	bool mTestBool;
	int mTestInt;
	double mTestDouble;
	std::string mTestString;
};

int main()
{
	auto simpleObj = TestSimpleClass();
	auto json = _XPLATSTR("{ \"TestInt\": 2000, \"TestDouble\": 1.0, \"TestString\" : \"Very looooooooong string!\" }");
	BitSerializer::LoadObject<JsonArchive>(simpleObj, json);
	if (!BitSerializer::Context.IsValid())
	{
		std::wcout << L"Validation errors: " << std::endl;
		const auto& validationErrors = BitSerializer::Context.GetValidationErrors();
		for (const auto& keyErrors : validationErrors)
		{
			std::wcout << L"Path: " << keyErrors.first << std::endl;
			for (const auto& err : keyErrors.second)
			{
				std::wcout << L"\t" << err << std::endl;
			}
		}
	}

	return EXIT_SUCCESS;
}