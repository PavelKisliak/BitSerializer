/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/serialization_detail/attr_key_value.h"
#include "bitserializer/serialization_detail/validators.h"


using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of AttributeValue
//-----------------------------------------------------------------------------
TEST(AttributeValue, ShouldStoreRefToKey)
{
	// Arrange
	const std::string key = "key1";
	int value = 10;

	// Act
	const AttributeValue attrValue(key, value);

	// Assert
	EXPECT_TRUE(&attrValue.GetKey() == &key);
}

TEST(AttributeValue, ShouldStoreKeyAsPtrToCString)
{
	// Arrange
	const char* key = "key1";
	int value = 10;

	// Act
	const AttributeValue attrValue(key, value);

	// Assert
	EXPECT_TRUE(attrValue.GetKey() == key);
}

TEST(AttributeValue, ShouldStoreKeyWhenPassedAsRValue)
{
	// Arrange
	int value = 10;

	// Act
	const AttributeValue attrValue(std::string("key"), value);

	// Assert
	EXPECT_EQ("key", attrValue.GetKey());
}

TEST(AttributeValue, ShouldStoreRefToValue)
{
	// Arrange
	int value = 10;

	// Act
	const AttributeValue attrValue("key", value);

	// Assert
	EXPECT_TRUE(&attrValue.GetValue() == &value);
}

TEST(AttributeValue, ShouldStoreValueWhenPassedAsRValue)
{
	// Act
	const AttributeValue attrValue("key", std::string("value"));

	// Assert
	EXPECT_EQ("value", attrValue.GetValue());
}

TEST(AttributeValue, ShouldVisitExtraParameters)
{
	// Arrange
	int value = 10;

	// Act
	AttributeValue attrValue("key", value, Validate::Required(), Validate::Range(0, 20));

	// Assert
	int knownArgs = 0, unknownArgs = 0;
	attrValue.VisitArgs([&knownArgs, &unknownArgs](auto& handler)
	{
		using Type = std::decay_t<decltype(handler)>;
		if constexpr (std::is_same_v<Type, Validate::Required> || std::is_same_v<Type, Validate::Range<int>>) {
			++knownArgs;
		}
		else {
			++unknownArgs;
		}
	});
	EXPECT_EQ(2, knownArgs);
	EXPECT_EQ(0, unknownArgs);
}
