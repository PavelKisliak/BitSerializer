/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/archive_stub.h"

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

TEST(STD_Chrono, SerializeTimePointMaxValues) {
	using hours_i8 = std::chrono::duration<int8_t, std::ratio<3600>>;
	using TimePointHoursI8Rep = std::chrono::time_point<std::chrono::system_clock, hours_i8>;

	TestSerializeType<ArchiveStub>(TimePointHoursI8Rep::max());
	TestSerializeType<ArchiveStub>(TimePointHoursI8Rep::min());

	TestSerializeType<ArchiveStub>(std::chrono::system_clock::time_point::max());
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
	const TestClassWithSubType expectedObj(std::chrono::system_clock::from_time_t(100));
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
	const TestClassWithSubType<TimePointNs> expectedObj(std::chrono::system_clock::from_time_t(100));
	TestClassWithSubType<TimePointNs> targetObj = expectedObj;
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive, options);

	targetObj.Assert(expectedObj);
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::chrono::duration
//-----------------------------------------------------------------------------
TEST(STD_Chrono, SerializeDuration) {
	auto dur = BuildFixture<std::chrono::seconds>();
	TestSerializeType<ArchiveStub>(dur);
}

TEST(STD_Chrono, SerializeDurationMaxValues) {
	using hours_i8 = std::chrono::duration<int8_t, std::ratio<3600>>;
	TestSerializeType<ArchiveStub>(hours_i8::max());
	TestSerializeType<ArchiveStub>(hours_i8::min());

	using hours_u8 = std::chrono::duration<uint8_t, std::ratio<3600>>;
	TestSerializeType<ArchiveStub>(hours_u8::max());

	TestSerializeType<ArchiveStub>(std::chrono::seconds::max());
	TestSerializeType<ArchiveStub>(std::chrono::seconds::min());
}

TEST(STD_Chrono, SerializeDurationAsClassMember) {
	TestClassWithSubType<std::chrono::seconds> testEntity;
	TestSerializeClass<ArchiveStub>(testEntity);
}

TEST(STD_Chrono, SerializeArrayOfDurations) {
	TestSerializeStlContainer<ArchiveStub, std::array<std::chrono::seconds, 100>>();
}

TEST(STD_Chrono, ThrowMismatchedTypesExceptionWhenLoadInvalidIsoDuration)
{
	// Save as string
	TestClassWithSubType<std::string> invalidDuration("P?MT10S");
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(invalidDuration, outputArchive);

	try
	{
		// Load as time_point
		TestClassWithSubType<std::chrono::seconds> targetObj;
		BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(STD_Chrono, ThrowOverflowTypeExceptionWhenLoadIsoDuration)
{
	// Save as string
	TestClassWithSubType<std::string> isoDuration("PT500S");
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(isoDuration, outputArchive);

	try
	{
		// Load as duration (to type which can store 0...255 seconds)
		using seconds_u8 = std::chrono::duration<uint8_t, std::ratio<1>>;
		TestClassWithSubType targetObj(seconds_u8(0));
		BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(STD_Chrono, SkipInvalidIsoDurationWhenPolicyIsSkip)
{
	// Save as string
	TestClassWithSubType<std::string> invalidDuration("PMT10S");
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(invalidDuration, outputArchive);

	// Load as time_point
	const TestClassWithSubType expectedObj(std::chrono::seconds(100));
	TestClassWithSubType targetObj = expectedObj;
	SerializationOptions options;
	options.mismatchedTypesPolicy = MismatchedTypesPolicy::Skip;
	BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive, options);

	targetObj.Assert(expectedObj);
}

TEST(STD_Chrono, SkipTooBigDurationWhenPolicyIsSkip)
{
	// Save as string
	TestClassWithSubType<std::string> isoDuration("PT256S");
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(isoDuration, outputArchive);

	// Load as duration (to type which can store -128...127 seconds)
	using seconds_i8 = std::chrono::duration<int8_t, std::ratio<1>>;
	const TestClassWithSubType expectedObj(seconds_i8(100));
	TestClassWithSubType targetObj = expectedObj;
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive, options);

	targetObj.Assert(expectedObj);
}
