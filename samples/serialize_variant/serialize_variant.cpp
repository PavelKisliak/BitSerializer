#include <iostream>

#include "bitserializer/bit_serializer.h"
#include "bitserializer/json_archive.h"
#include "bitserializer/types/std/variant.h"
#include "bitserializer/types/std/vector.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::JsonArchive;

class User
{
public:
	User() = default;
	User(std::string name, int age)
		: mName(std::move(name))
		, mAge(age)
	{ }

	bool operator==(const User& rhs) const noexcept {
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

// Required for the name-based (`VariantAsNamed`) and discriminated (`VariantAsDiscriminated`) representations.
// Every alternative used in such a variant must be registered with a unique type name.
BITSERIALIZER_REGISTER_TYPE(User, "User")
BITSERIALIZER_REGISTER_TYPE(std::vector<int>, "IntVector")
BITSERIALIZER_REGISTER_TYPE(int, "Int")
BITSERIALIZER_REGISTER_TYPE(std::string, "String")

class Product
{
public:
	Product() = default;
	Product(std::string name, uint64_t price)
		: mName(std::move(name))
		, mPrice(price)
	{ }

	bool operator==(const Product& rhs) const noexcept {
		return mName == rhs.mName && mPrice == rhs.mPrice;
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << KeyValue("name", mName);
		archive << KeyValue("price", mPrice);
	}

private:
	std::string mName;
	uint64_t mPrice = 0;
};

BITSERIALIZER_REGISTER_TYPE(Product, "Product")

int main()  // NOLINT(bugprone-exception-escape)
{
	using VariantType = std::variant<int, std::string, User, std::vector<int>>;

	// Default serialization (index-based). The active alternative is encoded as an integer index,
	// so no type registration is required.
	{
		VariantType testValue(User("Alice", 30));
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

	// Discriminated (internally-tagged) serialization via `VariantAsDiscriminated`.
	// The type name is embedded next to the active object's fields (the OpenAPI "discriminator"
	// style), instead of being wrapped around a nested value. This is what allows object
	// hierarchies to be represented in flat formats such as CSV.
	// Note: every alternative must be an object type to be flattened.
	using DiscriminatedVariantType = std::variant<User, Product>;

	{
		DiscriminatedVariantType testValue(User("Alice", 30));
		const auto jsonResult = BitSerializer::SaveObject<JsonArchive>(VariantAsDiscriminated(testValue));
		std::cout << "Discriminated (VariantAsDiscriminated) serialization:" << std::endl;
		std::cout << jsonResult << std::endl << std::endl;
	}

	// The discriminator field name can be customized.
	{
		DiscriminatedVariantType testValue(User("Alice", 30));
		const auto jsonResult = BitSerializer::SaveObject<JsonArchive>(VariantAsDiscriminated(testValue, "kind"));
		std::cout << "Discriminated with a custom discriminator key:" << std::endl;
		std::cout << jsonResult << std::endl << std::endl;
	}

	// Loading back a name-based variant.
	{
		VariantType actual;
		const std::string inputJson = R"({ "type": "User", "value": { "name": "Bob", "age": 25 } })";
		BitSerializer::LoadObject<JsonArchive>(VariantAsNamed(actual), inputJson);

		const auto expected = VariantType(User("Bob", 25));
		if (actual == expected) {
			std::cout << "Roundtrip (VariantAsNamed) succeeded." << std::endl;
		} else {
			std::cout << "Roundtrip (VariantAsNamed) FAILED." << std::endl;
		}
	}

	return 0;
}
