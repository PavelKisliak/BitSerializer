/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/serialization_detail/archive_traits.h"

using namespace BitSerializer;

/// <summary>
/// Test archive, which implements loading mode and serialization types WITHOUT keys
/// </summary>
class TestArchive_LoadMode : ArchiveScope<SerializeMode::Load>
{
public:
	TestArchive_LoadMode(const std::string& inputData) { }
	TestArchive_LoadMode(std::istream& inputData) { }

	void SerializeValue(bool& value) { }
	void SerializeValue(int& value) { }

	template <typename TSym, typename TAllocator>
	void SerializeString(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value) {}

	std::unique_ptr<TestArchive_LoadMode> OpenObjectScope() { return nullptr; }
	std::unique_ptr<TestArchive_LoadMode> OpenArrayScope(size_t arraySize) { return nullptr; }
};

/// <summary>
/// Test archive, which implements save mode and serialization types WITH keys
/// </summary>
class TestArchive_SaveMode : ArchiveScope<SerializeMode::Save>
{
public:
	using key_type = std::string;

	class key_const_iterator
	{
		key_type mTest;

	public:
		const key_type& operator*() const {
			return mTest;
		}
	};

	TestArchive_SaveMode(std::string& outputData) { }
	TestArchive_SaveMode(std::ostream& inputData) { }

	key_const_iterator cbegin() const {
		return key_const_iterator();
	}

	key_const_iterator cend() const {
		return key_const_iterator();
	}

	bool SerializeValue(const key_type& key, bool& value) { return true; }
	bool SerializeValue(const key_type& key, int& value) { return true; }

	template <typename TSym, typename TAllocator>
	bool SerializeString(const key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value) {return true;}

	std::unique_ptr<TestArchive_LoadMode> OpenObjectScope(const key_type& key) { return nullptr; }
	std::unique_ptr<TestArchive_LoadMode> OpenArrayScope(const key_type& key, size_t arraySize) { return nullptr; }
};

class TestWrongArchive
{
public:
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

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveSupportInputDataType) {
	bool testResult1 = is_archive_support_input_data_type_v<TestArchive_LoadMode, std::string>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_archive_support_input_data_type_v<TestArchive_LoadMode, std::istream>;
	EXPECT_TRUE(testResult2);

	bool testResult3 = is_archive_support_input_data_type_v<TestWrongArchive, std::string>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveSupportOutputDataType) {
	bool testResult1 = is_archive_support_output_data_type_v<TestArchive_SaveMode, std::string>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_archive_support_output_data_type_v<TestArchive_SaveMode, std::ostream>;
	EXPECT_TRUE(testResult2);

	bool testResult3 = is_archive_support_output_data_type_v<TestWrongArchive, std::string>;
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
	bool testResult1 = can_serialize_value_with_key_v<TestArchive_SaveMode, bool, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_value_with_key_v<TestArchive_SaveMode, int, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_value_with_key_v<TestWrongArchive, int, TestArchive_SaveMode::key_type>;
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
	bool testResult1 = can_serialize_object_with_key_v<TestArchive_SaveMode, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_object_with_key_v<TestArchive_SaveMode, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_object_with_key_v<TestWrongArchive, TestArchive_SaveMode::key_type>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveIsObjectScope) {
	bool testResult1 = is_object_scope_v<TestArchive_SaveMode, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_object_scope_v<TestWrongArchive, TestWrongArchive::key_type>;
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
	bool testResult1 = can_serialize_array_with_key_v<TestArchive_SaveMode, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_array_with_key_v<TestArchive_SaveMode, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_array_with_key_v<TestWrongArchive, TestWrongArchive::key_type>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatTypeConvertibleToOneFromTuple) {
	bool testResult1 = is_type_convertible_to_one_from_tuple_v<std::wstring, std::tuple<std::string, std::wstring>>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_type_convertible_to_one_from_tuple_v<wchar_t*, std::tuple<std::string, std::wstring>>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = is_type_convertible_to_one_from_tuple_v<std::string, std::tuple<std::wstring>>;
	EXPECT_FALSE(testResult3);
	bool testResult4 = is_type_convertible_to_one_from_tuple_v<std::string, std::tuple<>>;
	EXPECT_FALSE(testResult4);
}