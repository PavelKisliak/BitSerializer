/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "tests/test_helpers/common_test_methods.h"
#include "tests/test_helpers/archive_stub.h"

#include "bitserializer/types/std/chrono.h"
#include "bitserializer/types/std/array.h"


using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for std::chrono::time_point
//-----------------------------------------------------------------------------
TEST(STD_Chrono, SerializeTimePoint) {
	auto tp = BuildFixture<std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>>();
	TestSerializeType<ArchiveStub>(tp);
}

TEST(STD_Chrono, SerializeTimePointAsClassMember) {
	TestClassWithSubType<std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>> testEntity;
	TestSerializeClass<ArchiveStub>(testEntity);
}

TEST(STD_Chrono, SerializeArrayOfTimePoints) {
	using TimePointMs = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;
	TestSerializeStlContainer<ArchiveStub, std::array<TimePointMs, 100>>();
}

TEST(STD_Chrono, ThrowMismatchedTypesExceptionWhenLoadInvalidIsoDate)
{
	// Save as string
	TestClassWithSubType<std::string> invalidDatetime("1970/01/01T00:00:00Z");
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(invalidDatetime, outputArchive);

	try
	{
		// Load as time_point
		TestClassWithSubType<std::chrono::system_clock::time_point> targetObj;
		BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(STD_Chrono, ThrowOverflowTypeExceptionWhenLoadTooBigDate)
{
	// Save as string
	TestClassWithSubType<std::string> sourceObj("9999-12-31T23:59:59Z");
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(sourceObj, outputArchive);

	try
	{
		// Load as time_point
		TestClassWithSubType<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> targetObj;
		BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(STD_Chrono, SkipInvalidIsoDateWhenPolicyIsSkip)
{
	// Save as string
	TestClassWithSubType<std::string> invalidDatetime("1970/01/01T00:00:00Z");
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(invalidDatetime, outputArchive);

	// Load as time_point
	const TestClassWithSubType expectedObj(std::chrono::system_clock::from_time_t(0));
	TestClassWithSubType targetObj = expectedObj;
	SerializationOptions options;
	options.mismatchedTypesPolicy = MismatchedTypesPolicy::Skip;
	BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive, options);

	targetObj.Assert(expectedObj);
}

TEST(STD_Chrono, SkipTooBigDateWhenPolicyIsSkip)
{
	// Save as string
	TestClassWithSubType<std::string> sourceObj("9999-12-31T23:59:59Z");
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(sourceObj, outputArchive);

	// Load as time_point
	using TimePointNs = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
	const TestClassWithSubType<TimePointNs> expectedObj(std::chrono::system_clock::from_time_t(0));
	TestClassWithSubType<TimePointNs> targetObj = expectedObj;
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive, options);

	targetObj.Assert(expectedObj);
}
