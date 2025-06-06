/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <forward_list>
#include <optional>
#include "bitserializer/serialization_detail/object_traits.h"
#include "testing_tools/common_test_entities.h"


using namespace BitSerializer;

class TestSerializableClass
{
public:
	template <class TArchive>
	void Serialize(TArchive&) { }
};

class TestExtSerializableClass { };
template <class TArchive> void SerializeObject(TArchive&, TestExtSerializableClass&) { }

class TestExtSerializableArray
{
public:
	[[nodiscard]] constexpr int32_t GetSize() const { return 0; }  // NOLINT(readability-convert-member-functions-to-static)
};
template <class TArchive> void SerializeArray(TArchive&, TestExtSerializableArray&) { }

// External size() for test array serialization
auto size(const TestExtSerializableArray& value) {
	return value.GetSize();
}

class TestNotSerializableClass { };

class TestValidatorClass
{
public:
	template <class TValue>
	std::optional<std::string> operator() (const TValue&, const bool) const noexcept {
		return std::nullopt;
	}
};

//-----------------------------------------------------------------------------

TEST(SerializationObjectTraits, ShouldCheckThatClassHasSerializeMethod) {
	const bool testResult1 = has_serialize_method_v<TestSerializableClass>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = has_serialize_method_v<TestNotSerializableClass>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationObjectTraits, ShouldCheckThatClassHasExtSerializeMethod) {
	const bool testResult1 = has_global_serialize_object_v<TestExtSerializableClass>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = has_global_serialize_object_v<TestNotSerializableClass>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationObjectTraits, ShouldCheckThatArrayHasExtSerializeMethod) {
	const bool testResult1 = has_global_serialize_array_v<TestExtSerializableArray>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = has_global_serialize_array_v<TestNotSerializableClass>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationObjectTraits, ShouldCheckThatTypeIsEnumerable) {
	const bool testResult1 = is_enumerable_v<std::list<int>>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = is_enumerable_v<std::forward_list<int>>;
	EXPECT_TRUE(testResult2);

	const bool testResult3 = is_enumerable_v<TestNotSerializableClass>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationObjectTraits, ShouldCheckThatTypeIsEnumerableOfType) {
	const bool testResult1 = is_enumerable_of_v<std::list<int>, int>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = is_enumerable_of_v<std::forward_list<char>, char>;
	EXPECT_TRUE(testResult2);

	const bool testResult3 = is_enumerable_of_v<std::list<int>, char>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationObjectTraits, ShouldCheckThatTypeIsBinaryContainer) {
	const bool testResult1 = is_binary_container<std::list<char>>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = is_binary_container<std::vector<int8_t>>;
	EXPECT_TRUE(testResult2);
	const bool testResult3 = is_binary_container<std::forward_list<uint8_t>>;
	EXPECT_TRUE(testResult3);

	const bool testResult4 = is_binary_container<std::list<int>>;
	EXPECT_FALSE(testResult4);
}

TEST(SerializationObjectTraits, ShouldCheckThatContainerHasSizeMethod) {
	const bool testResult1 = has_size_v<std::list<int>>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = has_size_v<std::forward_list<int>>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationObjectTraits, ShouldCheckThatContainerHasGlobalSizeFn) {
	const bool testResult1 = has_global_size_v<std::vector<int>>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = has_global_size_v<TestExtSerializableArray>;
	EXPECT_TRUE(testResult2);
	const bool testResult3 = has_global_size_v<TestNotSerializableClass>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationObjectTraits, ShouldCheckThatContainerHasReserveMethod) {
	const bool testResult1 = has_reserve_v<std::vector<int>>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = has_reserve_v<std::list<int>>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationObjectTraits, ShouldGetContainerSizeForVector) {
	static constexpr size_t expectedSize = 10;
	const std::vector<int> testContainer(expectedSize);
	const auto actual = GetContainerSize(testContainer);
	EXPECT_EQ(expectedSize, actual);
}

TEST(SerializationObjectTraits, ShouldGetContainerSizeForForwardList) {
	static constexpr size_t expectedSize = 10;
	const std::forward_list<int> testContainer(expectedSize);
	const auto actual = GetContainerSize(testContainer);
	EXPECT_EQ(expectedSize, actual);
}

TEST(SerializationObjectTraits, ShouldCheckThatIsInputStream) {
	const bool testResult1 = is_input_stream_v<std::istringstream>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = is_input_stream_v<std::wistringstream>;
	EXPECT_TRUE(testResult2);

	const bool testResult3 = is_input_stream_v<std::ostringstream>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationObjectTraits, ShouldCheckThatIsOutputStream) {
	const bool testResult1 = is_output_stream_v<std::ostringstream>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = is_output_stream_v<std::wostringstream>;
	EXPECT_TRUE(testResult2);

	const bool testResult3 = is_output_stream_v<std::istringstream>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationObjectTraits, ShouldCheckThatIsValidator) {
	const bool testResult1 = is_validator_v<TestValidatorClass, int>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = is_validator_v<TestNotSerializableClass, int>;
	EXPECT_FALSE(testResult2);
}

//-----------------------------------------------------------------------------
// Tests of map fields counter
//-----------------------------------------------------------------------------
struct IntFieldsCounterFixture
{
	int x = 0, y = 0;

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << KeyValue("x", x);
		archive << KeyValue("y", y);
	}
};

struct ExtFieldsCounterFixture
{
	int x = 0, y = 0, z = 0;
};

struct FieldsCounterFixtureWithInheritance : IntFieldsCounterFixture
{
	int z = 0;

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::BaseObject<IntFieldsCounterFixture>(*this);
		archive << KeyValue("z", x);
	}
};

template<typename TArchive>
void SerializeObject(TArchive& archive, ExtFieldsCounterFixture& fixture)
{
	archive << KeyValue("x", fixture.x) << KeyValue("y", fixture.y) << KeyValue("z", fixture.z);
}

template <bool IsBinary = false>
struct ArchiveTest
{
	static constexpr ArchiveType archive_type = ArchiveType::Json;
	using key_type = std::string;
	static constexpr bool is_binary = IsBinary;		// Only binary archive's types require counting number of fields

	static constexpr SerializeMode GetMode() noexcept { return SerializeMode::Save; }
	static constexpr bool IsSaving() noexcept { return true; }
	static constexpr bool IsLoading() noexcept { return false; }
};

//-----------------------------------------------------------------------------

TEST(SerializationObjectTraits, ShouldCountObjectFieldsWithInternalFn) {
	constexpr ArchiveTest archive;
	TestPointClass val(10, 20);
	EXPECT_EQ(2U, FieldsCountVisitor(archive).Count(val));
}

TEST(SerializationObjectTraits, ShouldCountObjectFieldsWithGlobalFn) {
	constexpr ArchiveTest archive;
	ExtFieldsCounterFixture val;
	EXPECT_EQ(3U, FieldsCountVisitor(archive).Count(val));
}

TEST(SerializationObjectTraits, ShouldCountFieldsOfMap) {
	constexpr ArchiveTest<false> textArchive;
	constexpr ArchiveTest<true> binArchive;
	std::map<int, int> val { {1, 1}, { 2,2 }, { 3, 3 }, { 4, 4 } };

	EXPECT_EQ(0U, CountMapObjectFields(textArchive, val));
	EXPECT_EQ(4U, CountMapObjectFields(binArchive, val));
}

TEST(SerializationObjectTraits, ShouldCountObjectWithBaseSerializableClass) {
	constexpr ArchiveTest archive;
	FieldsCounterFixtureWithInheritance val;
	EXPECT_EQ(3U, FieldsCountVisitor(archive).Count(val));
}

//-----------------------------------------------------------------------------
// Tests of compatible_fixed_t
//-----------------------------------------------------------------------------
TEST(SerializationObjectTraits, ShouldMapCompatibleSignedTypes)
{
	using type1 = compatible_fixed_t<short>;
	constexpr bool result1 = std::is_convertible_v<type1&, int16_t&>;
	EXPECT_TRUE(result1);

	using type2 = compatible_fixed_t<int>;
	constexpr bool result2 = std::is_convertible_v<type2&, int32_t&>;
	EXPECT_TRUE(result2);

	if constexpr (sizeof(long) == sizeof(int32_t))
	{
		using type3 = compatible_fixed_t<long>;
		constexpr bool result3 = std::is_convertible_v<type3&, int32_t&>;
		EXPECT_TRUE(result3);
	}
	else if constexpr (sizeof(long) == sizeof(int64_t))
	{
		using type3 = compatible_fixed_t<long>;
		constexpr bool result3 = std::is_convertible_v<type3&, int64_t&>;
		EXPECT_TRUE(result3);
	}

	using type4 = compatible_fixed_t<long long>;
	constexpr bool result4 = std::is_convertible_v<type4&, int64_t&>;
	EXPECT_TRUE(result4);
}

TEST(SerializationObjectTraits, ShouldMapCompatibleUnsignedTypes)
{
	using type1 = compatible_fixed_t<unsigned short>;
	constexpr bool result1 = std::is_convertible_v<type1&, uint16_t&>;
	EXPECT_TRUE(result1);

	using type2 = compatible_fixed_t<unsigned int>;
	constexpr bool result2 = std::is_convertible_v<type2&, uint32_t&>;
	EXPECT_TRUE(result2);

	if constexpr (sizeof(unsigned long) == sizeof(uint32_t))
	{
		using type3 = compatible_fixed_t<unsigned long>;
		constexpr bool result3 = std::is_convertible_v<type3&, uint32_t&>;
		EXPECT_TRUE(result3);
	}
	else if constexpr (sizeof(unsigned long) == sizeof(uint64_t))
	{
		using type3 = compatible_fixed_t<unsigned long>;
		constexpr bool result3 = std::is_convertible_v<type3&, uint64_t&>;
		EXPECT_TRUE(result3);
	}

	using type4 = compatible_fixed_t<unsigned long long>;
	constexpr bool result4 = std::is_convertible_v<type4&, uint64_t&>;
	EXPECT_TRUE(result4);
}

TEST(SerializationObjectTraits, ShouldMapCompatibleCharTypes)
{
	if constexpr (std::is_signed_v<char>)
	{
		using type1 = compatible_fixed_t<char>;
		constexpr bool result1 = std::is_convertible_v<type1&, int8_t&>;
		EXPECT_TRUE(result1);
	}
	else
	{
		using type1 = compatible_fixed_t<char>;
		constexpr bool result1 = std::is_convertible_v<type1&, uint8_t&>;
		EXPECT_TRUE(result1);
	}
}
