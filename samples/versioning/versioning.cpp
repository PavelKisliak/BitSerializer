#include <iostream>
#include <iomanip>

#include "bitserializer/bit_serializer.h"
#include "bitserializer/msgpack_archive.h"
#include "bitserializer/types/std/vector.h"

using namespace BitSerializer;
using MsgPackArchive = MsgPack::MsgPackArchive;

// Old version of test object (no needs to keep old models, just as example)
struct TestUserV1
{
	std::string name;			// Deprecated, need to split to first and last name
	uint8_t age{};
	uint32_t lastOrderId{};		// Deprecated, need to remove

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << KeyValue("name", name, Required());
		archive << KeyValue("age", age);
		archive << KeyValue("lastOrderId", lastOrderId);
	}
};

// Actual model
struct TestUser
{
	// Introduce version field
	static constexpr int16_t CurrentVersion = 1;

	std::string firstName;
	std::string lastName;
	uint8_t age{};
	std::string country;

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		// Load 'version' field if exists
		int16_t version = TArchive::IsSaving() ? CurrentVersion : 0;
		archive << KeyValue("version", version);

		if constexpr (TArchive::IsLoading())
		{
			if (version == 0)
			{
				// Import name from old format
				std::string name;
				archive << KeyValue("name", name, Required());
				const auto spacePos = name.find(' ');
				firstName = name.substr(0, spacePos);
				lastName = spacePos != std::string::npos ? name.substr(spacePos + 1) : "";
			}
			else
			{
				archive << KeyValue("firstName", firstName, Required());
				archive << KeyValue("lastName", lastName, Required());
			}
		}
		archive << KeyValue("age", age);
		archive << KeyValue("country", country);
	}
};

int main()
{
	// Save old version
	std::vector<TestUserV1> oldUsers {
		{ "John Smith", 35, 1254 },
		{ "Emily Roberts", 27, 4546 },
		{ "James Murphy", 32, 10653 }
	};
	const auto archive = BitSerializer::SaveObject<MsgPackArchive>(oldUsers);

	// Loading with import to new version
	std::vector<TestUser> newUsers;
	BitSerializer::LoadObject<MsgPackArchive>(newUsers, archive);

	// Print
	static constexpr int columnWidth = 12;
	std::cout << std::left << std::setw(columnWidth) << " First name" << " |"
		<< std::setw(columnWidth) << " Last Name" << " |"
		<< std::setw(columnWidth) << " Age" << " |"
		<< std::setw(columnWidth) << " Country" << std::endl;

	std::string splitLine(60, '-');
	std::cout << splitLine << std::endl;

	for (const auto& user : newUsers)
	{
		std::cout << " "
			<< std::setw(columnWidth) << user.firstName << "| "
			<< std::setw(columnWidth) << user.lastName << "| "
			<< std::setw(columnWidth) << std::to_string(user.age) << "| "
			<< std::setw(columnWidth) << (user.country.empty() ? "Unknown" : user.country)
			<< std::endl;
	}

	return 0;
}
