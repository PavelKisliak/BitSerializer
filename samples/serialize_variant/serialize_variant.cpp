#include <iostream>

#include "bitserializer/bit_serializer.h"
#include "bitserializer/json_archive.h"
#include "bitserializer/types/std/variant.h"
#include "bitserializer/types/std/vector.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::JsonArchive;

class CUser
{
public:
	CUser() = default;
	CUser(std::string name, int age)
		: mName(std::move(name))
		, mAge(age)
	{ }

	bool operator==(const CUser& rhs) const noexcept {
		return mName == rhs.mName && mAge == rhs.mAge;
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << KeyValue("name", mName);
		archive << KeyValue("age", mAge);
	}

private:
	std::string mName;
	int mAge = 0;
};

// Required for `VariantAsNamed` only.
// Every alternative used in a name-based variant must be registered with a unique type name.
BITSERIALIZER_REGISTER_TYPE(CUser, "User")
BITSERIALIZER_REGISTER_TYPE(std::vector<int>, "IntVector")
BITSERIALIZER_REGISTER_TYPE(int, "Int")
BITSERIALIZER_REGISTER_TYPE(std::string, "String")

int main()  // NOLINT(bugprone-exception-escape)
{
	using VariantType = std::variant<int, std::string, CUser, std::vector<int>>;

	// Default serialization (index-based). The active alternative is encoded as an/ integer index,
	// so no type registration is required.
	{
		VariantType testValue(CUser("Alice", 30));
		const auto jsonResult = BitSerializer::SaveObject<JsonArchive>(testValue);
		std::cout << "Default (index-based) serialization:" << std::endl;
		std::cout << jsonResult << std::endl << std::endl;
	}

	// Name-based serialization via `VariantAsNamed`. The active alternative is encoded by its registered
	// type name, which is more stable than an index (useful for versioning and human-readable formats).
	{
		VariantType testValue(std::vector<int>{ 1, 2, 3 });
		const auto jsonResult = BitSerializer::SaveObject<JsonArchive>(VariantAsNamed(testValue));
		std::cout << "Name-based (VariantAsNamed) serialization:" << std::endl;
		std::cout << jsonResult << std::endl << std::endl;
	}

	// Loading back a name-based variant.
	{
		VariantType actual;
		const std::string inputJson = R"({ "type": "User", "value": { "name": "Bob", "age": 25 } })";
		BitSerializer::LoadObject<JsonArchive>(VariantAsNamed(actual), inputJson);

		const auto expected = VariantType(CUser("Bob", 25));
		if (actual == expected) {
			std::cout << "Roundtrip (VariantAsNamed) succeeded." << std::endl;
		} else {
			std::cout << "Roundtrip (VariantAsNamed) FAILED." << std::endl;
		}
	}

	return 0;
}
