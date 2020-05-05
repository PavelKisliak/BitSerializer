#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"

using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

class UserModel
{
public:
	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		using namespace BitSerializer;

		archive << MakeKeyValue("Id", mId, Required());
		archive << MakeKeyValue("Age", mAge, Required(), Range(0, 150));
		archive << MakeKeyValue("FirstName", mFirstName, Required(), MaxSize(16));
		archive << MakeKeyValue("LastName", mLastName, Required(), MaxSize(16));
		// Custom validation with lambda
		archive << MakeKeyValue("NickName", mNickName, [](const std::string& value, const bool isLoaded) -> std::optional<std::wstring>
		{
			if (!isLoaded || value.find_first_of(' ') == std::string::npos)
				return std::nullopt;
			return L"The field must not contain spaces";
		});
	}

private:
	uint64_t mId = 0;
	uint16_t mAge = 0;
	std::string mFirstName;
	std::string mLastName;
	std::string mNickName;
};

int main()
{
	UserModel user;
	const auto json = R"({ "Id": 12420, "Age": 500, "FirstName": "John Smith-Cotatonovich", "NickName": "Smith 2000" })";
	BitSerializer::LoadObject<JsonArchive>(user, json);
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