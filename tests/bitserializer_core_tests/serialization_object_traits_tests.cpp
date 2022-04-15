/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include <sstream>
#include <vector>
#include <array>
#include <list>
#include <forward_list>
#include <optional>
#include "bitserializer/serialization_detail/object_traits.h"

using namespace BitSerializer;

class TestSerializableClass
{
public:
	template <class TArchive>
	void Serialize(TArchive& archive) { }
};

class TestExtSerializableClass { };
template <class TArchive> void SerializeObject(TArchive& archive, TestExtSerializableClass& value) { }

class TestExtSerializableArray { };
template <class TArchive> void SerializeArray(TArchive& archive, TestExtSerializableArray& value) { }

class TestNotSerializableClass { };

class TestValidatorClass
{
public:
	template <class TValue>
	std::optional<std::string> operator() (const TValue& value, const bool isLoaded) const noexcept {
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

TEST(SerializationObjectTraits, ShouldCheckThatContainerHasSizeMethod) {
	const bool testResult1 = has_size_v<std::list<int>>;
	EXPECT_TRUE(testResult1);
	const bool testResult2 = has_size_v<std::forward_list<int>>;
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
