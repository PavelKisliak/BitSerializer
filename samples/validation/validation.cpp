#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"

using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

class UserModel
{
public:
	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		using namespace BitSerializer;

		archive << KeyValue("Id", mId, Required());
		archive << KeyValue("Age", mAge, Required(), Range(0, 150));
		archive << KeyValue("FirstName", mFirstName, Required(), MaxSize(16));
		archive << KeyValue("LastName", mLastName, Required(), MaxSize(16));
		// Custom validation with lambda
		archive << KeyValue("NickName", mNickName, [](const std::string& value, const bool isLoaded) -> std::optional<std::string>
		{
			// Loaded string should has text without spaces or should be NULL
			if (!isLoaded || value.find_first_of(' ') == std::string::npos)
				return std::nullopt;
			return "The field must not contain spaces";
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
	const char* json = R"({ "Id": 12420, "Age": 500, "FirstName": "John Smith-Cotatonovich", "NickName": "Smith 2000" })";
	try
	{
		BitSerializer::LoadObject<JsonArchive>(user, json);
	}
	catch (BitSerializer::ValidationException& ex)
	{
		const auto& validationErrors = ex.GetValidationErrors();
		std::cout << "Validation errors: " << std::endl;
		for (const auto& keyErrors : validationErrors)
		{
			std::cout << "Path: " << keyErrors.first << std::endl;
			for (const auto& err : keyErrors.second)
			{
				std::cout << "\t" << err << std::endl;
			}
		}
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what();
	}

	return EXIT_SUCCESS;
}