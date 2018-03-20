/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "pch.h"
#include "common_test_entities.h"
#include "bitserializer\serialization_detail\object_traits.h"

using namespace BitSerializer;

class TestSerializableClass
{
public:
	template <class TArchive>
	inline void Serialize(TArchive& archive) { };
};

class TestNotSerializableClass { };

TEST(SerializationTarits, ShouldCheckThatClassHasSerializeMethod) {
	bool testResult1 = is_serializable_class_v<TestSerializableClass>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_serializable_class_v<TestNotSerializableClass>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationTarits, ShouldCheckThatContainerHasResizeMethod) {
	bool testResult1 = is_resizeable_cont_v<std::vector<int>>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = is_resizeable_cont_v<std::array<int, 5>>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationTarits, ShouldCheckThatContainerHasSizeMethod) {
	bool testResult1 = has_size_v<std::list<int>>;
	EXPECT_TRUE(testResult1);
	bool testResult2 = has_size_v<std::forward_list<int>>;
	EXPECT_FALSE(testResult2);
}

TEST(SerializationTarits, ShouldGetContainerSizeForVector) {
	static constexpr size_t expectedSize = 10;
	std::vector<int> testContainer(expectedSize);
	auto actual = GetContainerSize(testContainer);
	EXPECT_EQ(expectedSize, actual);
}

TEST(SerializationTarits, ShouldGetContainerSizeForForwardList) {
	static constexpr size_t expectedSize = 10;
	std::forward_list<int> testContainer(expectedSize);
	auto actual = GetContainerSize(testContainer);
	EXPECT_EQ(expectedSize, actual);
}