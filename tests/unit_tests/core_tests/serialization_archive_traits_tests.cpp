/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/serialization_detail/archive_traits.h"
#include "bitserializer/serialization_detail/archive_key.h"

using namespace BitSerializer;

// NOLINTBEGIN(readability-convert-member-functions-to-static)

/**
 * @brief Test stub of archive that implements loading mode and serialization types WITHOUT keys.
 */
class TestArchive_LoadMode : ArchiveScope<SerializeMode::Load>
{
public:
	TestArchive_LoadMode(const std::string&, SerializationContext& context) 
		: ArchiveScope<SerializeMode::Load>(context)
	{ }
	TestArchive_LoadMode(std::istream&, SerializationContext& context)
		: ArchiveScope<SerializeMode::Load>(context)
	{ }
	TestArchive_LoadMode(ScopeUnopened, SerializationContext& context)
		: ArchiveScope<SerializeMode::Load>(context, ScopeUnopened{})
	{ }

	bool SerializeValue(bool&) { return true; }
	bool SerializeValue(int&) { return true; }
	bool SerializeValue(std::nullptr_t&) { return true; }

	template <typename TSym, typename TAllocator>
	void SerializeString(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>&) {}

	TestArchive_LoadMode OpenObjectScope(size_t) { return {ScopeUnopened{}, GetContext()}; }
	TestArchive_LoadMode OpenArrayScope(size_t) { return {ScopeUnopened{}, GetContext()}; }
	TestArchive_LoadMode OpenBinaryScope(size_t) { return {ScopeUnopened{}, GetContext()}; }
	TestArchive_LoadMode OpenAttributeScope() { return {ScopeUnopened{}, GetContext()}; }

	[[nodiscard]] size_t GetEstimatedSize() const { return 0; }
};

/**
 * @brief Test stub of archive that implements save mode and serialization types WITH keys.
 */
class TestArchive_SaveMode : ArchiveScope<SerializeMode::Save>
{
public:
	using key_type = std::string;
	using supported_key_types = TSupportedKeyTypes<key_type, std::string_view>;

	class key_const_iterator
	{
		key_type mTest;

	public:
		const key_type& operator*() const {
			return mTest;
		}
	};

	TestArchive_SaveMode(std::string&, SerializationContext& context)
		: ArchiveScope<SerializeMode::Save>(context) { }
	TestArchive_SaveMode(std::ostream&, SerializationContext& context)
		: ArchiveScope<SerializeMode::Save>(context)
	{ }

	bool SerializeValue(const key_type&, bool&) { return true; }
	bool SerializeValue(const key_type&, int&) { return true; }
	bool SerializeValue(const key_type&, std::nullptr_t&) { return true; }

	template <typename TSym, typename TAllocator>
	bool SerializeString(const key_type&, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>&) {return true;}

	TestArchive_LoadMode OpenObjectScope(const key_type&, size_t) { return {ScopeUnopened{}, GetContext()}; }
	TestArchive_LoadMode OpenArrayScope(const key_type&, size_t) { return {ScopeUnopened{}, GetContext()}; }
	TestArchive_LoadMode OpenBinaryScope(const key_type&, size_t) { return {ScopeUnopened{}, GetContext()}; }
	TestArchive_LoadMode OpenAttributeScope(const key_type&) { return {ScopeUnopened{}, GetContext()}; }
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
	bool testResult3 = can_serialize_value_v<TestArchive_LoadMode, std::nullptr_t>;
	EXPECT_TRUE(testResult3);
	bool testResult4 = can_serialize_value_v<TestWrongArchive, int>;
	EXPECT_FALSE(testResult4);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeValueWithKey) {
	bool testResult1 = can_serialize_value_with_key_v<TestArchive_SaveMode, bool, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_value_with_key_v<TestArchive_SaveMode, int, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_value_with_key_v<TestArchive_SaveMode, std::nullptr_t, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult3);
	bool testResult4 = can_serialize_value_with_key_v<TestWrongArchive, int, TestArchive_SaveMode::key_type>;
	EXPECT_FALSE(testResult4);
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

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeBinArray) {
	bool testResult1 = can_serialize_binary_v<TestArchive_LoadMode>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_binary_v<TestArchive_LoadMode>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_binary_v<TestWrongArchive>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeBinArrayWithKey) {
	bool testResult1 = can_serialize_binary_with_key_v<TestArchive_SaveMode, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_binary_with_key_v<TestArchive_SaveMode, TestArchive_SaveMode::key_type>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_binary_with_key_v<TestWrongArchive, TestWrongArchive::key_type>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatArchiveCanSerializeAttribute) {
	bool testResult1 = can_serialize_attribute_v<TestArchive_LoadMode>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = can_serialize_attribute_v<TestArchive_LoadMode>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = can_serialize_attribute_v<TestWrongArchive>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationArchiveTraits, ShouldCheckThatStringTypeConvertibleToAnyOfTuple) {
	bool testResult1 = Detail::is_convertible_to_any_of_tuple_v<std::wstring, std::tuple<std::string, std::wstring>>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = Detail::is_convertible_to_any_of_tuple_v<const wchar_t*, std::tuple<std::string, std::wstring>>;
	EXPECT_TRUE(testResult2);
	bool testResult3 = Detail::is_convertible_to_any_of_tuple_v<const char[], std::tuple<std::string_view>>;
	EXPECT_TRUE(testResult3);

	bool testResult4 = Detail::is_convertible_to_any_of_tuple_v<std::string, std::tuple<std::wstring>>;
	EXPECT_FALSE(testResult4);
	bool testResult5 = Detail::is_convertible_to_any_of_tuple_v<std::string, std::tuple<>>;
	EXPECT_FALSE(testResult5);
}

TEST(SerializationArchiveTraits, ShouldCheckThatIntegralTypeConvertibleToAnyOfTuple) {
	bool testResult1 = Detail::is_convertible_to_any_of_tuple_v<int16_t, std::tuple<float, int64_t>>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = Detail::is_convertible_to_any_of_tuple_v<uint8_t, std::tuple<std::string, uint64_t>>;
	EXPECT_TRUE(testResult2);

	bool testResult3 = Detail::is_convertible_to_any_of_tuple_v<bool, std::tuple<uint8_t>>;
	EXPECT_FALSE(testResult3);
	bool testResult4 = Detail::is_convertible_to_any_of_tuple_v<const bool, std::tuple<std::string, std::string_view, int64_t, uint64_t, float, double>>;
	EXPECT_FALSE(testResult4);
	bool testResult5 = Detail::is_convertible_to_any_of_tuple_v<float, std::tuple<uint64_t>>;
	EXPECT_FALSE(testResult5);
}

TEST(SerializationArchiveTraits, ShouldCheckThatFloatingTypeConvertibleToAnyOfTuple) {
	bool testResult1 = Detail::is_convertible_to_any_of_tuple_v<float, std::tuple<int64_t, float>>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = Detail::is_convertible_to_any_of_tuple_v<double, std::tuple<uint64_t, double>>;
	EXPECT_TRUE(testResult2);

	bool testResult3 = Detail::is_convertible_to_any_of_tuple_v<float, std::tuple<uint64_t>>;
	EXPECT_FALSE(testResult3);
	bool testResult4 = Detail::is_convertible_to_any_of_tuple_v<double, std::tuple<uint64_t>>;
	EXPECT_FALSE(testResult4);
}

TEST(SerializationArchiveTraits, ShouldPassThroughKeyWhenArchiveSupportsIt) {
	using Archive = TestArchive_SaveMode;

	const std::string_view nameView = "name";
	const std::string nameStr = "name";
	static_assert(std::is_same_v<decltype(Detail::ToArchiveKey<Archive>(nameView)), const std::string_view&>);
	static_assert(std::is_same_v<decltype(Detail::ToArchiveKey<Archive>(nameStr)), const std::string&>);

	const auto key = Detail::ToArchiveKey<Archive>(nameView);
	static_assert(std::is_same_v<std::decay_t<decltype(key)>, std::string_view>);
	EXPECT_EQ(key, "name");
}

TEST(SerializationArchiveTraits, ShouldConvertKeyWhenArchiveDoesNotSupportIt) {
	using Archive = TestArchive_SaveMode;

	const int number = 42;
	static_assert(std::is_same_v<decltype(Detail::ToArchiveKey<Archive>(number)), std::string>);

	const auto key = Detail::ToArchiveKey<Archive>(number);
	static_assert(std::is_same_v<decltype(key), const std::string>);
	EXPECT_EQ(key, "42");
}

// NOLINTEND(readability-convert-member-functions-to-static)
