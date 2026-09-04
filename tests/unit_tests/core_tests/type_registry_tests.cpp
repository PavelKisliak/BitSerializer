/*******************************************************************************
 * Copyright (C) 2018-2026 by Pavel Kisliak                                     *
 * This file is part of BitSerializer library, licensed under the MIT license.  *
 *******************************************************************************/
#include "gtest/gtest.h"
#include "bitserializer/serialization_detail/type_registry.h"
#include <memory>

using namespace BitSerializer;
using namespace BitSerializer::Detail;

// Test types for variant alternatives
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

// Test types for polymorphic hierarchy
struct Animal
{
	virtual ~Animal() = default;
	[[nodiscard]] virtual std::string Sound() const = 0;
};

struct Cat : Animal
{
	[[nodiscard]] std::string Sound() const override {
		return "Meow";
	}
};
struct Dog : Animal
{
	[[nodiscard]] std::string Sound() const override {
		return "Woof";
	}
};

// Register types for variant alternatives
BITSERIALIZER_REGISTER_TYPE(User, "User");
BITSERIALIZER_REGISTER_TYPE(Admin, "Admin");
BITSERIALIZER_REGISTER_TYPE(Guest, "Guest");

// Register types for polymorphic hierarchy
BITSERIALIZER_REGISTER_BASE_TYPE(Animal);
BITSERIALIZER_REGISTER_TYPE(Cat, "Cat");
BITSERIALIZER_REGISTER_TYPE(Dog, "Dog");

// Registry for variant alternatives
using VariantRegistry = TypeRegistry<User, Admin, Guest>;

// Registry for polymorphic hierarchy
using HierarchyRegistry = TypeRegistry<Cat, Dog>;

TEST(TypeRegistry, VariantAlternativesRegistered)
{
	EXPECT_TRUE(VariantRegistry::Contains("User"));
	EXPECT_TRUE(VariantRegistry::Contains("Admin"));
	EXPECT_TRUE(VariantRegistry::Contains("Guest"));
	EXPECT_FALSE(VariantRegistry::Contains("Cat"));
}

TEST(TypeRegistry, HierarchyTypesRegistered)
{
	EXPECT_TRUE(HierarchyRegistry::Contains("Cat"));
	EXPECT_TRUE(HierarchyRegistry::Contains("Dog"));
	EXPECT_FALSE(HierarchyRegistry::Contains("Animal")); // Base not in registry
	EXPECT_FALSE(HierarchyRegistry::Contains("User"));
}

TEST(TypeRegistry, FindByNameReturnsCorrectEntry)
{
	const auto* userEntry = VariantRegistry::Find("User");
	ASSERT_NE(userEntry, nullptr);
	EXPECT_EQ(userEntry->Name, "User");
	EXPECT_NE(userEntry->Factory, nullptr);
	EXPECT_NE(userEntry->Emplace, nullptr);

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

TEST(TypeRegistry, FactoryCreatesCorrectType)
{
	const auto* userEntry = VariantRegistry::Find("User");
	ASSERT_NE(userEntry, nullptr);
	ASSERT_NE(userEntry->Factory, nullptr);

	auto* user = static_cast<User*>(userEntry->Factory());
	EXPECT_NE(user, nullptr);
	user->id = 42;
	user->name = "Test";
	delete user;
}

TEST(TypeRegistry, EmplaceCreatesCorrectType)
{
	const auto* userEntry = VariantRegistry::Find("User");
	ASSERT_NE(userEntry, nullptr);
	ASSERT_NE(userEntry->Emplace, nullptr);

	alignas(User) char storage[sizeof(User)];
	userEntry->Emplace(storage);
	auto* user = reinterpret_cast<User*>(storage);
	EXPECT_NE(user, nullptr);
	user->id = 42;
	user->name = "Test";
	user->~User(); // Manual destruction for placement new
}

TEST(TypeRegistry, BaseTypeHasNoFactory)
{
	// Animal is registered as base type (no factory)
	static_assert(SerializableTypeTraits<Animal>::Name.empty());
	static_assert(!HasCreateFactory<Animal>::value);
	static_assert(!HasEmplaceFactory<Animal>::value);
}

TEST(TypeRegistry, VariantRegistryEntriesCount)
{
	EXPECT_EQ(VariantRegistry::Size(), 3u);
}

TEST(TypeRegistry, HierarchyRegistryEntriesCount)
{
	EXPECT_EQ(HierarchyRegistry::Size(), 2u);
}

TEST(TypeRegistry, SerializableTypeTraitsDefault)
{
	// Unregistered type has empty name and deleted factory
	struct UnregisteredType {};
	static_assert(SerializableTypeTraits<UnregisteredType>::Name.empty());
	static_assert(!HasCreateFactory<UnregisteredType>::value);
	static_assert(!HasEmplaceFactory<UnregisteredType>::value);
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
	EXPECT_NE(entry->Factory, nullptr);
	EXPECT_NE(entry->Emplace, nullptr);
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