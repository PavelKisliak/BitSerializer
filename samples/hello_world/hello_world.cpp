#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
#include "bitserializer/csv_archive.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/types/std/chrono.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;
using CsvArchive = BitSerializer::Csv::CsvArchive;

struct CUser
{
	// Mandatory fields
	uint64_t Id = 0;
	std::u16string Name;
	std::chrono::system_clock::time_point Birthday;
	std::string Email;
	// Optional fields (maybe absent or `null` in the source JSON)
	std::string PhoneNumber;
	std::u32string NickName;
	std::string Language;

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << KeyValue("Id", Id, Required());
		// Using the `Required()` validator with a custom error message (can be ID of localization string)
		archive << KeyValue("Birthday", Birthday, Required("Birthday is required"));
		archive << KeyValue("Name", Name, Required(), Validate::MaxSize(32));
		archive << KeyValue("Email", Email, Required(), Refine::TrimWhitespace(), Validate::Email());
		// Optional field (should be empty or contain a valid phone number)
		archive << KeyValue("PhoneNumber", PhoneNumber, Refine::TrimWhitespace(), Validate::PhoneNumber());
		archive << KeyValue("NickName", NickName);
		// Use fallback value "en" if missing data
		archive << KeyValue("Language", Language, Refine::ToLowerCase(), Fallback("en"));
	}
};

int main()	// NOLINT(bugprone-exception-escape)
{
	const char* sourceJson = R"([
{ "Id": 1, "Birthday": "1998-05-15T00:00:00Z", "Name": "John Doe", "Email": "john.doe@example.com", "PhoneNumber": "+(123) 4567890", "NickName": "JD" },
{ "Id": 2, "Birthday": "1993-08-20T00:00:00Z", "Name": "Alice Smith", "Email": "alice.smith@example.com", "PhoneNumber": "+(098) 765-43-21", "NickName": "Ali" },
{ "Id": 3, "Birthday": "2001-03-10T00:00:00Z", "Name": "Ivan Petrov", "Email": "ivan.petrov@example.com", "PhoneNumber": null, "Language": "RU" }
])";

	// Load list of users from JSON
	std::vector<CUser> users;
	BitSerializer::LoadObject<JsonArchive>(users, sourceJson);

	// Save to CSV
	std::string csv;
	BitSerializer::SaveObject<CsvArchive>(users, csv);

	std::cout << csv << std::endl;
	return EXIT_SUCCESS;
}
