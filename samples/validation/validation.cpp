#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer_json_restcpp/json_restcpp_archive.h"

using namespace BitSerializer;

class TestSimpleClass
{
public:
	TestSimpleClass(): testInt{}, testFloat{} { }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeKeyValue("TestInt", testInt, Required(), Range(0, 100));
		archive << MakeKeyValue("TestFloat", testFloat, Required(), Range(-1.0f, 1.0f));
		archive << MakeKeyValue("TestString", testString, MaxSize(8));
	};

private:
	int testInt;
	float testFloat;
	std::wstring testString;
};

int main()
{
	auto simpleObj = TestSimpleClass();
	LoadObject<JsonArchive>(simpleObj, L"{	\"TestInt\": 2000, \"TestString\" : \"Very looooooooong string!\"  }");
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