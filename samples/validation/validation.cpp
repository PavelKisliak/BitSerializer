#include <iostream>
#include <string>
#include <sstream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer_json_restcpp/json_restcpp_archive.h"

using namespace BitSerializer;

class TestSimpleClass
{
public:
	TestSimpleClass(): testInt{}, testDouble{} { }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeKeyValue("testInt", testInt, Required(), Range(0, 100));
		archive << MakeKeyValue("testDouble", testDouble, Required(), Range(-1.0, 1.0));
		archive << MakeKeyValue("testString", testString, MaxSize(8));
	};

private:
	int testInt;
	double testDouble;
	std::string testString;
};

int main()
{
	auto simpleObj = TestSimpleClass();
	auto json = _XPLATSTR("{ \"testInt\": 2000, \"testDouble\": 1.0, \"testString\" : \"Very looooooooong string!\" }");
	LoadObject<JsonArchive>(simpleObj, json);
	if (!Context.IsValid())
	{
		std::wcout << L"Validation errors: " << std::endl;
		const auto& validationErrors = Context.GetValidationErrors();
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