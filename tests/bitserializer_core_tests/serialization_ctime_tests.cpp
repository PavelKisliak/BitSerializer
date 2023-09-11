/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/archive_stub.h"

#include "bitserializer/types/std/ctime.h"


using namespace BitSerializer;

//-----------------------------------------------------------------------------

struct TestCTime
{
	TestCTime() = default;
	TestCTime(time_t time) : Time(time) { }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << AutoKeyValue("Time", CTimeRef(Time));
	}

	time_t Time = 0;
};

//-----------------------------------------------------------------------------
// Tests of serialization for std::time_t
//-----------------------------------------------------------------------------
TEST(STD_Ctime, SerializeTimeType)
{
	// Arrange
	time_t expected = 2335219200, actual = 0;
	ArchiveStub::preferred_output_format outputArchive;

	// Act
	BitSerializer::SaveObject<ArchiveStub>(CTimeRef(expected), outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(CTimeRef(actual), outputArchive);

	// Assert
	EXPECT_EQ(expected, actual);
}

TEST(STD_Ctime, ThrowMismatchedTypesExceptionWhenLoadInvalidIsoDate)
{
	// Save as string
	std::string invalidDatetime = "1970/01/01T00:00:00Z";
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(invalidDatetime, outputArchive);

	try
	{
		// Load as `time_t`
		time_t actual = 0;
		BitSerializer::LoadObject<ArchiveStub>(CTimeRef(actual), outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(STD_Ctime, SkipInvalidIsoDateWhenPolicyIsSkip)
{
	// Save as string
	std::string invalidDatetime = "1970/01/01T00:00:00Z";
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(invalidDatetime, outputArchive);

	// Load as `time_t`
	time_t actual = 0;
	SerializationOptions options;
	options.mismatchedTypesPolicy = MismatchedTypesPolicy::Skip;
	BitSerializer::LoadObject<ArchiveStub>(CTimeRef(actual), outputArchive, options);

	EXPECT_EQ(0, actual);
}

TEST(STD_Ctime, SerializeTimeTypeAsClassMember)
{
	// Arrange
	TestCTime expected(2335219200), actual;
	ArchiveStub::preferred_output_format outputArchive;

	// Act
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);

	// Assert
	EXPECT_EQ(expected.Time, actual.Time);
}
