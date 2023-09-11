/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/serialization_detail/archive_base.h"

using namespace BitSerializer;


TEST(ArchiveBase_SafeNumberCast, ShouldConvertBooleanToUnsigned)
{
	uint8_t targetNumber = 0;
	EXPECT_TRUE(Detail::SafeNumberCast(true, targetNumber, OverflowNumberPolicy::Skip));
	EXPECT_EQ(1, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldConvertUnsignedToBoolean)
{
	bool targetBoolean = false;
	EXPECT_TRUE(Detail::SafeNumberCast(1, targetBoolean, OverflowNumberPolicy::Skip));
	EXPECT_EQ(true, targetBoolean);
}

TEST(ArchiveBase_SafeNumberCast, ShouldReturnFalseWhenOverflowBoolean)
{
	bool targetBoolean = false;
	EXPECT_FALSE(Detail::SafeNumberCast(2, targetBoolean, OverflowNumberPolicy::Skip));
	EXPECT_EQ(false, targetBoolean);
}

TEST(ArchiveBase_SafeNumberCast, ShouldThrowExceptionWhenOverflowBoolean)
{
	bool targetBoolean = false;
	EXPECT_THROW(Detail::SafeNumberCast(2, targetBoolean, OverflowNumberPolicy::ThrowError), SerializationException);
	EXPECT_EQ(false, targetBoolean);
}

TEST(ArchiveBase_SafeNumberCast, ShouldConvertMaxPositiveSignedToUnsigned)
{
	uint8_t targetNumber;
	constexpr int8_t sourceNumber = std::numeric_limits<int8_t>::max();
	EXPECT_TRUE(Detail::SafeNumberCast(sourceNumber, targetNumber, OverflowNumberPolicy::Skip));
	EXPECT_EQ(sourceNumber, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldReturnFalseWhenConvertNegativeNumberToUnsigned)
{
	uint8_t targetNumber = 0;
	EXPECT_FALSE(Detail::SafeNumberCast(-1, targetNumber, OverflowNumberPolicy::Skip));
	EXPECT_EQ(0, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldConvertMinSignedToSameType)
{
	int32_t targetNumber;
	constexpr int32_t sourceNumber = std::numeric_limits<int32_t>::min();
	EXPECT_TRUE(Detail::SafeNumberCast(sourceNumber, targetNumber, OverflowNumberPolicy::Skip));
	EXPECT_EQ(sourceNumber, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldConvertMaxUnsignedToSameType)
{
	uint32_t targetNumber;
	constexpr uint32_t sourceNumber = std::numeric_limits<uint32_t>::max();
	EXPECT_TRUE(Detail::SafeNumberCast(sourceNumber, targetNumber, OverflowNumberPolicy::Skip));
	EXPECT_EQ(sourceNumber, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldThrowExceptionWhenOverflowSigned)
{
	int64_t targetNumber = 0;
	constexpr uint64_t sourceNumber = static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1;
	EXPECT_THROW(Detail::SafeNumberCast(sourceNumber, targetNumber, OverflowNumberPolicy::ThrowError), SerializationException);
	EXPECT_EQ(0, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldThrowExceptionWhenConvertNegativeNumberToUnsigned)
{
	uint8_t targetNumber = 0;
	EXPECT_THROW(Detail::SafeNumberCast(-1, targetNumber, OverflowNumberPolicy::ThrowError), SerializationException);
	EXPECT_EQ(0, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldLosslesslyConvertMaxFloatFromDouble)
{
	float targetNumber = 0.f;
	constexpr auto sourceNumber = static_cast<double>(std::numeric_limits<float>::max());
	EXPECT_TRUE(Detail::SafeNumberCast(sourceNumber, targetNumber, OverflowNumberPolicy::Skip));
	EXPECT_EQ(sourceNumber, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldConvertZeroFromDouble)
{
	float targetNumber = -1.f;
	constexpr double sourceNumber = 0.0;
	EXPECT_TRUE(Detail::SafeNumberCast(sourceNumber, targetNumber, OverflowNumberPolicy::Skip));
	EXPECT_EQ(sourceNumber, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldLosslesslyConvertMinFloatFromDouble)
{
	float targetNumber = 0.f;
	constexpr auto sourceNumber = static_cast<double>(std::numeric_limits<float>::min());
	EXPECT_TRUE(Detail::SafeNumberCast(sourceNumber, targetNumber, OverflowNumberPolicy::Skip));
	EXPECT_EQ(sourceNumber, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldLosslesslyConvertLowestFloatFromDouble)
{
	float targetNumber = 0.f;
	constexpr auto sourceNumber = static_cast<double>(std::numeric_limits<float>::lowest());
	EXPECT_TRUE(Detail::SafeNumberCast(sourceNumber, targetNumber, OverflowNumberPolicy::Skip));
	EXPECT_EQ(sourceNumber, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldReturnFalseWhenOverflowPositiveFloat)
{
	float targetNumber = 0.f;
	constexpr auto sourceNumber = static_cast<double>(std::numeric_limits<float>::max()) * 1.00001;
	EXPECT_FALSE(Detail::SafeNumberCast(sourceNumber, targetNumber, OverflowNumberPolicy::Skip));
	EXPECT_EQ(0.f, targetNumber);
}

TEST(ArchiveBase_SafeNumberCast, ShouldReturnFalseWhenOverflowNegativeFloat)
{
	float targetNumber = 0.f;
	constexpr auto sourceNumber = static_cast<double>(std::numeric_limits<float>::lowest()) * 1.00001;
	EXPECT_FALSE(Detail::SafeNumberCast(sourceNumber, targetNumber, OverflowNumberPolicy::Skip));
	EXPECT_EQ(0.f, targetNumber);
}
