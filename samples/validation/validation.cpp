// Define for suppress warning STL4015 : The std::iterator class template (used as a base class to provide typedefs) is deprecated in C++17.
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"

using namespace BitSerializer;
using namespace BitSerializer::Json::RapidJson;

class TestSimpleClass
{
public:
	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeKeyValue("TestBool", mTestBool, Required());
		archive << MakeKeyValue("TestInt", mTestInt, Required(), Range(0, 100));
		archive << MakeKeyValue("TestDouble", mTestDouble, Required(), Range(-1.0, 1.0));
		archive << MakeKeyValue("TestString", mTestString, MaxSize(8));
		// Sample with validation via lambda
		archive << MakeKeyValue("TestString2", mTestString, [](const std::string& val, const bool isLoaded) -> std::optional<std::wstring>
		{
			if (!isLoaded || val.find_first_of(' ') == std::string::npos)
				return std::nullopt;
			return L"The field must not contain spaces.";
		});
	}

private:
	bool mTestBool = false;
	int mTestInt = 0;
	double mTestDouble = 0.0;
	std::string mTestString;
	std::string mTestString2;
};

int main()
{
	auto simpleObj = TestSimpleClass();
	auto json = "{ \"TestInt\": 2000, \"TestDouble\": 1.0, \"TestString\" : \"Very looooooooong string!\", \"TestString2\" : \"1 23\" }";
	BitSerializer::LoadObject<JsonArchive>(simpleObj, json);
	if (!BitSerializer::Context.IsValid())
	{
		std::wcout << L"Validation errors: " << std::endl;
		const auto& validationErrors = BitSerializer::Context.GetValidationErrors();
		for (const auto& keyErrors : validationErrors)
		{
			std::cout << "Path: " << keyErrors.first << std::endl;
			for (const auto& err : keyErrors.second)
			{
				std::wcout << L"\t" << err << std::endl;
			}
		}
	}

	return EXIT_SUCCESS;
}