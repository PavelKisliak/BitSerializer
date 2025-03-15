/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/serialization_detail/archive_base.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests for ConvertByPolicy()
//-----------------------------------------------------------------------------
class NonConvertibleFixture { };

enum class UnregisteredEnum
{
	One, Two, Three
};

TEST(ConvertByPolicyTest, ConvertFundamentalTypes)
{
	uint8_t targetUint8 = 0;
	EXPECT_TRUE(Detail::ConvertByPolicy(true, targetUint8, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError));
	EXPECT_EQ(1, targetUint8);

	uint16_t targetInt16;
	constexpr int16_t sourceNumber = std::numeric_limits<int16_t>::max();
	EXPECT_TRUE(Detail::ConvertByPolicy(sourceNumber, targetInt16, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError));
	EXPECT_EQ(sourceNumber, targetInt16);

	float targetFloat = 0.f;
	constexpr auto sourceDouble = static_cast<double>(std::numeric_limits<float>::min());
	EXPECT_TRUE(Detail::ConvertByPolicy(sourceDouble, targetFloat, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError));
	EXPECT_EQ(sourceDouble, targetFloat);
}

TEST(ConvertByPolicyTest, ConvertToString)
{
	std::string targetStr;
	EXPECT_TRUE(Detail::ConvertByPolicy(true, targetStr, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError));
	EXPECT_EQ("true", targetStr);

	targetStr.clear();
	EXPECT_TRUE(Detail::ConvertByPolicy(10, targetStr, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError));
	EXPECT_EQ("10", targetStr);
}

TEST(ConvertByPolicyTest, ConvertFromString)
{
	bool targetBool = false;
	EXPECT_TRUE(Detail::ConvertByPolicy("true", targetBool, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError));
	EXPECT_EQ(true, targetBool);

	uint16_t targetInt16;
	EXPECT_TRUE(Detail::ConvertByPolicy("10925", targetInt16, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError));
	EXPECT_EQ(10925, targetInt16);
}

TEST(ConvertByPolicyTest, ThrowExceptionWhenOverflowType)
{
	int targetInteger = 0;
	EXPECT_THROW(Detail::ConvertByPolicy(std::numeric_limits<int64_t>::max(), targetInteger, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError), SerializationException);
	EXPECT_EQ(0, targetInteger);

	EXPECT_THROW(Detail::ConvertByPolicy(std::numeric_limits<int64_t>::min(), targetInteger, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError), SerializationException);
	EXPECT_EQ(0, targetInteger);
}

TEST(ConvertByPolicyTest, SkipOverflowValueWhenPolicyIsSkip)
{
	bool targetBoolean = false;
	EXPECT_FALSE(Detail::ConvertByPolicy(2, targetBoolean, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::Skip));
	EXPECT_EQ(false, targetBoolean);

	uint8_t targetUint8 = 0;
	EXPECT_FALSE(Detail::ConvertByPolicy(-1, targetUint8, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::Skip));
	EXPECT_EQ(0, targetUint8);

	float targetFloat = 0.f;
	constexpr auto sourceDouble = static_cast<double>(std::numeric_limits<float>::max()) * 1.00001;
	EXPECT_FALSE(Detail::ConvertByPolicy(sourceDouble, targetFloat, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::Skip));
	EXPECT_EQ(0.f, targetFloat);
}

TEST(ConvertByPolicyTest, ThrowExceptionWhenMismatchedType)
{
	int targetInteger = 0;
	EXPECT_THROW(Detail::ConvertByPolicy(NonConvertibleFixture(), targetInteger, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError), SerializationException);
	EXPECT_EQ(0, targetInteger);

	EXPECT_THROW(Detail::ConvertByPolicy("InvalidNumber", targetInteger, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError), SerializationException);
	EXPECT_EQ(0, targetInteger);
}

TEST(ConvertByPolicyTest, SkipMismatchedTypeWhenPolicyIsSkip)
{
	int targetInteger = 0;
	EXPECT_FALSE(Detail::ConvertByPolicy(NonConvertibleFixture(), targetInteger, MismatchedTypesPolicy::Skip, OverflowNumberPolicy::ThrowError));
	EXPECT_EQ(0, targetInteger);

	EXPECT_FALSE(Detail::ConvertByPolicy("InvalidNumber", targetInteger, MismatchedTypesPolicy::Skip, OverflowNumberPolicy::ThrowError));
	EXPECT_EQ(0, targetInteger);
}

TEST(ConvertByPolicyTest, ThrowExceptionWhenUnregisteredEnum)
{
	UnregisteredEnum targetEnum = UnregisteredEnum::One;
	EXPECT_THROW(Detail::ConvertByPolicy("Two", targetEnum, MismatchedTypesPolicy::ThrowError, OverflowNumberPolicy::ThrowError), SerializationException);
	EXPECT_EQ(UnregisteredEnum::One, targetEnum);
}
