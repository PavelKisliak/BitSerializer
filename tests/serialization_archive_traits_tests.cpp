/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "pch.h"
#include "common_test_entities.h"
#include "bitserializer\serialization_detail\archive_traits.h"

using namespace BitSerializer;

/// <summary>
/// Test archive, which implements loading mode and serialization types WITHOUT keys
/// </summary>
class TestArchive_LoadMode : ArchiveScope<SerializeMode::Load>
{
public:
	void SerializeValue(bool& value) { }
	void SerializeValue(int& value) { }

	template <typename TSym, typename TAllocator>
	void SerializeString(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value) {}

	std::unique_ptr<TestArchive_LoadMode> OpenScopeForSerializeObject() { return nullptr; }
	std::unique_ptr<TestArchive_LoadMode> OpenScopeForSerializeArray(size_t arraySize) { return nullptr; }
};

/// <summary>
/// Test archive, which implements save mode and serialization types WITH keys
/// </summary>
class TestArchive_SaveMode : ArchiveScope<SerializeMode::Save>
{
public:
	using key_type = std::string;

	key_type GetKeyByIndex(size_t index) { return key_type(); }

	void SerializeValue(const key_type& key, bool& value) { }
	void SerializeValue(const key_type& key, int& value) { }

	template <typename TSym, typename TAllocator>
	void SerializeString(const key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value) {}

	std::unique_ptr<TestArchive_LoadMode> OpenScopeForSerializeObject(const key_type& key) { return nullptr; }
	std::unique_ptr<TestArchive_LoadMode> OpenScopeForSerializeArray(const key_type& key, size_t arraySize) { return nullptr; }
};

class TestWrongArchive
{
	using key_type = std::string;
};

TEST(SerializationArchiveTraits, ShouldCheckThatClassInheritedFromArchiveScope) {
	bool testResult1 = is_archive_scope_v<TestArchive_LoadMode>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_archive_scope_v<TestArchive_SaveMode>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = is_archive_scope_v<TestWrongArchive>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeValue) {
	bool testResult1 = can_serialize_value_v<TestArchive_LoadMode, bool>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_value_v<TestArchive_LoadMode, int>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_value_v<TestWrongArchive, int>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeValueWithKey) {
	bool testResult1 = can_serialize_value_with_key_v<TestArchive_SaveMode, bool>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_value_with_key_v<TestArchive_SaveMode, int>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_value_with_key_v<TestWrongArchive, int>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeString) {
	bool testResult1 = can_serialize_string_v<TestArchive_LoadMode, std::string>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_string_v<TestArchive_LoadMode, std::wstring>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_string_v<TestWrongArchive, std::string>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeStringWithKey) {
	bool testResult1 = can_serialize_string_with_key_v<TestArchive_SaveMode, std::string>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_string_with_key_v<TestArchive_SaveMode, std::wstring>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_string_with_key_v<TestWrongArchive, std::string>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeObject) {
	bool testResult1 = can_serialize_object_v<TestArchive_LoadMode>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_object_v<TestArchive_LoadMode>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_object_v<TestWrongArchive>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeObjectWithKey) {
	bool testResult1 = can_serialize_object_with_key_v<TestArchive_SaveMode>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_object_with_key_v<TestArchive_SaveMode>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_object_with_key_v<TestWrongArchive>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveIsObjectScope) {
	bool testResult1 = is_object_scope_v<TestArchive_SaveMode>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_object_scope_v<TestWrongArchive>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeArray) {
	bool testResult1 = can_serialize_array_v<TestArchive_LoadMode>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_array_v<TestArchive_LoadMode>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_array_v<TestWrongArchive>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeArrayWithKey) {
	bool testResult1 = can_serialize_array_with_key_v<TestArchive_SaveMode>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_array_with_key_v<TestArchive_SaveMode>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_array_with_key_v<TestWrongArchive>;
	EXPECT_FALSE(testResult3);
}