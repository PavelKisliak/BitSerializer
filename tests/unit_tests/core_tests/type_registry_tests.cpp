/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "gtest/gtest.h"
#include "bitserializer/serialization_detail/type_registry.h"
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;
using namespace BitSerializer::Detail;

//-----------------------------------------------------------------------------
// Test types for variant alternatives
//-----------------------------------------------------------------------------
struct User
{
	int id;
	std::string name;
};
struct Admin
{
	int id;
	std::vector<std::string> permissions;
};
struct Guest
{
	std::string sessionId;
};

// Register types for variant alternatives
BITSERIALIZER_REGISTER_TYPE(User, "User")
BITSERIALIZER_REGISTER_TYPE(Admin, "Admin")
BITSERIALIZER_REGISTER_TYPE(Guest, "Guest")

// Registry for variant alternatives
using VariantRegistry = TypeRegistry<User, Admin, Guest>;

//-----------------------------------------------------------------------------
TEST(TypeRegistry, VariantAlternativesRegistered)
{
	EXPECT_TRUE(VariantRegistry::Contains("User"));
	EXPECT_TRUE(VariantRegistry::Contains("Admin"));
	EXPECT_TRUE(VariantRegistry::Contains("Guest"));
	EXPECT_FALSE(VariantRegistry::Contains("Cat"));
}

TEST(TypeRegistry, FindByNameReturnsCorrectEntry)
{
	const auto* userEntry = VariantRegistry::Find("User");
	ASSERT_NE(userEntry, nullptr);
	EXPECT_EQ(userEntry->Name, "User");

	const auto* adminEntry = VariantRegistry::Find("Admin");
	ASSERT_NE(adminEntry, nullptr);
	EXPECT_EQ(adminEntry->Name, "Admin");

	const auto* missingEntry = VariantRegistry::Find("NonExistent");
	EXPECT_EQ(missingEntry, nullptr);
}

TEST(TypeRegistry, FindByNameCaseSensitive)
{
	const auto* entry = VariantRegistry::Find("user"); // lowercase
	EXPECT_EQ(entry, nullptr);
}

TEST(TypeRegistry, VariantRegistryEntriesCount)
{
	EXPECT_EQ(VariantRegistry::Size(), 3u);
}

TEST(TypeRegistry, SerializableTypeTraitsDefault)
{
	// Unregistered type has empty name and deleted factory
	struct UnregisteredType {};
	static_assert(SerializableTypeTraits<UnregisteredType>::Name.empty());
}

TEST(TypeRegistry, FindReturnsCachedEntry)
{
	// Find() should return the same cached entry for repeated calls
	const auto* entry1 = VariantRegistry::Find("User");
	const auto* entry2 = VariantRegistry::Find("User");
	EXPECT_EQ(entry1, entry2); // Same cached entry returned
}

TEST(TypeRegistry, RegistryEntriesAreConstexpr)
{
	// Registry entries are constexpr (name and factory)
	static_assert(VariantRegistry::Size() == 3u, "Registry size is constexpr");

	// Runtime checks for constexpr functionality
	const auto* entry = VariantRegistry::Find("Guest");
	ASSERT_NE(entry, nullptr);
	EXPECT_EQ(entry->Name, "Guest");
	EXPECT_EQ(entry->Index, 2u);
}

TEST(TypeRegistry, FindByIndexReturnsCorrectEntry)
{
	const auto* entry0 = VariantRegistry::FindByIndex(0);
	ASSERT_NE(entry0, nullptr);
	EXPECT_EQ(entry0->Name, "User");
	EXPECT_EQ(entry0->Index, 0u);

	const auto* entry2 = VariantRegistry::FindByIndex(2);
	ASSERT_NE(entry2, nullptr);
	EXPECT_EQ(entry2->Name, "Guest");
	EXPECT_EQ(entry2->Index, 2u);

	const auto* entryOutOfRange = VariantRegistry::FindByIndex(10);
	EXPECT_EQ(entryOutOfRange, nullptr);
}

TEST(TypeRegistry, EmplaceByIndex)
{
	using TestVariant = std::variant<User, Admin, Guest>;
	TestVariant variant;

	// Emplace by index 0 (User)
	VariantRegistry::EmplaceByIndex(variant, 0);
	ASSERT_TRUE(std::holds_alternative<User>(variant));

	// Emplace by index 1 (Admin)
	VariantRegistry::EmplaceByIndex(variant, 1);
	ASSERT_TRUE(std::holds_alternative<Admin>(variant));

	// Emplace by index 2 (Guest)
	VariantRegistry::EmplaceByIndex(variant, 2);
	ASSERT_TRUE(std::holds_alternative<Guest>(variant));

	// Out of range should not crash (variant unchanged)
	VariantRegistry::EmplaceByIndex(variant, 10);
	ASSERT_TRUE(std::holds_alternative<Guest>(variant));
}

TEST(TypeRegistry, HeaderRegisteredTypeIsFound)
{
	// `TestPointClass` is registered in the shared "common_test_entities.h" header, which is included by many translation units.
	// Verifying it here confirms that name-based registration works across translation-unit boundaries.
	using HeaderRegistry = TypeRegistry<TestPointClass>;
	static_assert(SerializableTypeTraits<TestPointClass>::Name == "TestPointClass", "TestPointClass must be registered in common_test_entities.h");

	EXPECT_TRUE(HeaderRegistry::Contains("TestPointClass"));
	const auto* entry = HeaderRegistry::Find("TestPointClass");
	ASSERT_NE(entry, nullptr);
	EXPECT_EQ(entry->Name, "TestPointClass");
	EXPECT_EQ(entry->Index, 0u);
}
