/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "pch.h"
#include "bitserializer\serialization_detail\object_traits.h"

using namespace BitSerializer;

class TestSerializableClass
{
public:
	template <class TArchive>
	inline void Serialize(TArchive& archive) { };
};

class TestNotSerializableClass { };

TEST(SerializationObjectTraits, ShouldCheckThatClassHasSerializeMethod) {
	bool testResult1 = is_serializable_class_v<TestSerializableClass>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_serializable_class_v<TestNotSerializableClass>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationObjectTraits, ShouldCheckThatContainerHasResizeMethod) {
	bool testResult1 = is_resizeable_cont_v<std::vector<int>>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_resizeable_cont_v<std::array<int, 5>>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationObjectTraits, ShouldCheckThatContainerHasSizeMethod) {
	bool testResult1 = has_size_v<std::list<int>>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = has_size_v<std::forward_list<int>>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationObjectTraits, ShouldGetContainerSizeForVector) {
	static constexpr size_t expectedSize = 10;
	std::vector<int> testContainer(expectedSize);
	auto actual = GetContainerSize(testContainer);
	EXPECT_EQ(expectedSize, actual);
}

TEST(SerializationObjectTraits, ShouldGetContainerSizeForForwardList) {
	static constexpr size_t expectedSize = 10;
	std::forward_list<int> testContainer(expectedSize);
	auto actual = GetContainerSize(testContainer);
	EXPECT_EQ(expectedSize, actual);
}

TEST(SerializationObjectTraits, ShouldCheckThatIsInputStream) {
	bool testResult1 = is_input_stream_v<std::istringstream>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_input_stream_v<std::wistringstream>;
	EXPECT_TRUE(testResult2);

	bool testResult3 = is_input_stream_v<std::ostringstream>;
	EXPECT_FALSE(testResult3);
}

TEST(SerializationObjectTraits, ShouldCheckThatIsOutputStream) {
	bool testResult1 = is_output_stream_v<std::ostringstream>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_output_stream_v<std::wostringstream>;
	EXPECT_TRUE(testResult2);

	bool testResult3 = is_output_stream_v<std::istringstream>;
	EXPECT_FALSE(testResult3);
}