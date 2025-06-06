﻿/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/serialization_detail/key_value.h"
#include "bitserializer/serialization_detail/validators.h"


using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of KeyValue
//-----------------------------------------------------------------------------
TEST(KeyValue, ShouldStoreRefToKey)
{
	// Arrange
	const std::string key = "key1";
	int value = 10;

	// Act
	const KeyValue keyValue(key, value);

	// Assert
	EXPECT_TRUE(&keyValue.GetKey() == &key);
}

TEST(KeyValue, ShouldStoreKeyAsPtrToCString)
{
	// Arrange
	const char* key = "key1";
	int value = 10;

	// Act
	const KeyValue keyValue(key, value);

	// Assert
	EXPECT_TRUE(keyValue.GetKey() == key);
}

TEST(KeyValue, ShouldStoreKeyWhenPassedAsRValue)
{
	// Arrange
	int value = 10;

	// Act
	const KeyValue keyValue(std::string("key"), value);

	// Assert
	EXPECT_EQ("key", keyValue.GetKey());
}

TEST(KeyValue, ShouldStoreRefToValue)
{
	// Arrange
	int value = 10;

	// Act
	const KeyValue keyValue("key", value);

	// Assert
	EXPECT_TRUE(&keyValue.GetValue() == &value);
}

TEST(KeyValue, ShouldStoreValueWhenPassedAsRValue)
{
	// Act
	const KeyValue keyValue("key", std::string("value"));

	// Assert
	EXPECT_EQ("value", keyValue.GetValue());
}

TEST(KeyValue, ShouldStoreValidators)
{
	// Arrange
	int value = 10;

	// Act
	KeyValue keyValue("key", value, Validate::Required(), Validate::Range(0, 20));

	// Assert
	int knownArgs = 0, unknownArgs = 0;
	keyValue.VisitArgs([&knownArgs, &unknownArgs](auto& handler)
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
