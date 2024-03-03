/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/archive_stub.h"
#include "testing_tools/bin_archive_stub.h"

#include "bitserializer/types/std/chrono.h"
#include "bitserializer/types/std/array.h"


using namespace std::chrono;
using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for time_point
//-----------------------------------------------------------------------------
TEST(STD_Chrono, SerializeTimePoint)
{
	auto tpSec = BuildFixture<time_point<system_clock, seconds>>();
	TestSerializeType<ArchiveStub>(tpSec);

	auto tpMs = BuildFixture<time_point<system_clock, milliseconds>>();
	TestSerializeType<ArchiveStub>(tpMs);

	auto tpUs = BuildFixture<time_point<system_clock, microseconds>>();
	TestSerializeType<ArchiveStub>(tpUs);

	auto tpNs = BuildFixture<time_point<system_clock, nanoseconds>>();
	TestSerializeType<ArchiveStub>(tpNs);
}

TEST(STD_Chrono, SerializeTimePointMaxValues)
{
	using TimePointSec = time_point<system_clock, seconds>;
	TestSerializeType<BinArchiveStub>(TimePointSec::min());
	TestSerializeType<BinArchiveStub>(TimePointSec::max());

	using TimePointMs = time_point<system_clock, milliseconds>;
	TestSerializeType<BinArchiveStub>(TimePointMs::min());
	TestSerializeType<BinArchiveStub>(TimePointMs::max());

	using TimePointMu = time_point<system_clock, microseconds>;
	TestSerializeType<BinArchiveStub>(TimePointMu::min());
	TestSerializeType<BinArchiveStub>(TimePointMu::max());

	using TimePointNs = time_point<system_clock, nanoseconds>;
	TestSerializeType<BinArchiveStub>(TimePointNs::min());
	TestSerializeType<BinArchiveStub>(TimePointNs::max());
}

TEST(STD_Chrono, SerializeTimePointWithInt8AsRep)
{
	using hours_i8 = duration<int8_t, std::ratio<3600>>;
	using TimePointHoursI8Rep = time_point<system_clock, hours_i8>;

	TestSerializeType<ArchiveStub>(TimePointHoursI8Rep::max());
	TestSerializeType<ArchiveStub>(TimePointHoursI8Rep::min());
}

TEST(STD_Chrono, SerializeTimePointAsClassMember)
{
	TestClassWithSubType<time_point<system_clock, seconds>> testEntitySec;
	TestSerializeClass<ArchiveStub>(testEntitySec);

	TestClassWithSubType<time_point<system_clock, milliseconds>> testEntityMs;
	TestSerializeClass<ArchiveStub>(testEntityMs);

	TestClassWithSubType<time_point<system_clock, microseconds>> testEntityUs;
	TestSerializeClass<ArchiveStub>(testEntityUs);

	TestClassWithSubType<time_point<system_clock, nanoseconds>> testEntityNs;
	TestSerializeClass<ArchiveStub>(testEntityNs);
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
		TestClassWithSubType<system_clock::time_point> targetObj;
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
		TestClassWithSubType<time_point<system_clock, nanoseconds>> targetObj;
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
	const TestClassWithSubType expectedObj(system_clock::from_time_t(100));
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
	using TimePointNs = time_point<system_clock, nanoseconds>;
	const TestClassWithSubType<TimePointNs> expectedObj(system_clock::from_time_t(100));
	TestClassWithSubType<TimePointNs> targetObj = expectedObj;
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive, options);

	targetObj.Assert(expectedObj);
}

TEST(STD_Chrono, ShouldLoadTimepointWithRoundingSecondFractions)
{
	time_point<system_clock, nanoseconds> testTpNs1(1499999999ns);
	TestLoadingToDifferentType<ArchiveStub>(testTpNs1, time_point<system_clock, milliseconds>(1500ms));
	TestLoadingToDifferentType<ArchiveStub>(testTpNs1, time_point<system_clock, microseconds>(1500000us));
	TestLoadingToDifferentType<ArchiveStub>(testTpNs1, time_point<system_clock, seconds>(1s));

	time_point<system_clock, nanoseconds> testTpNs2(2494354999ns);
	TestLoadingToDifferentType<ArchiveStub>(testTpNs2, time_point<system_clock, milliseconds>(2494ms));
	TestLoadingToDifferentType<ArchiveStub>(testTpNs2, time_point<system_clock, microseconds>(2494355us));
	TestLoadingToDifferentType<ArchiveStub>(testTpNs2, time_point<system_clock, seconds>(2s));
}

TEST(STD_Chrono, ThrowOverflowExceptionWhenTimepointCannotBeRounded)
{
	// Arrange
	time_point<system_clock, nanoseconds> testTpNs1(61499999999ns);
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(testTpNs1, outputArchive);

	// Act / Assert
	try
	{
		time_point<system_clock, minutes> actual;
		BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
		EXPECT_FALSE(true) << "Seconds should not be rounded to minutes";
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
	}
}

TEST(STD_Chrono, SkipErrorOfRoundingTimepointWhenPolicyIsSkip)
{
	// Arrange
	time_point<system_clock, nanoseconds> testTpNs1(61499999999ns);
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(testTpNs1, outputArchive);

	// Act
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	time_point<system_clock, minutes> actual(0min);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive, options);

	// Assert
	EXPECT_EQ(0, actual.time_since_epoch().count());
}

TEST(STD_Chrono, SerializeArrayOfTimePoints) {
	using TimePointMs = time_point<system_clock, milliseconds>;
	TestSerializeStlContainer<ArchiveStub, std::array<TimePointMs, 100>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for duration
//-----------------------------------------------------------------------------
TEST(STD_Chrono, SerializeDuration)
{
	auto durSec = BuildFixture<seconds>();
	TestSerializeType<ArchiveStub>(durSec);

	auto durMs = milliseconds(-6224282118935838835);
	TestSerializeType<ArchiveStub>(durMs);

	auto durUs = BuildFixture<microseconds>();
	TestSerializeType<ArchiveStub>(durUs);

	auto durNs = BuildFixture<nanoseconds>();
	TestSerializeType<ArchiveStub>(durNs);
}

TEST(STD_Chrono, SerializeDurationMaxValues)
{
	using hours_i8 = duration<int8_t, std::ratio<3600>>;
	TestSerializeType<ArchiveStub>(hours_i8::max());
	TestSerializeType<ArchiveStub>(hours_i8::min());

	using hours_u8 = duration<uint8_t, std::ratio<3600>>;
	TestSerializeType<ArchiveStub>(hours_u8::max());

	TestSerializeType<ArchiveStub>(seconds::max());
	TestSerializeType<ArchiveStub>(seconds::min());
}

TEST(STD_Chrono, SerializeDurationAsClassMember)
{
	TestClassWithSubType<seconds> testEntitySec;
	TestSerializeClass<ArchiveStub>(testEntitySec);

	TestClassWithSubType<milliseconds> testEntityMs;
	TestSerializeClass<ArchiveStub>(testEntityMs);

	TestClassWithSubType<microseconds> testEntityUs;
	TestSerializeClass<ArchiveStub>(testEntityUs);

	TestClassWithSubType<nanoseconds> testEntityNs;
	TestSerializeClass<ArchiveStub>(testEntityNs);
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
		TestClassWithSubType<seconds> targetObj;
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
		using seconds_u8 = duration<uint8_t, std::ratio<1>>;
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
	const TestClassWithSubType expectedObj(seconds(100));
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
	using seconds_i8 = duration<int8_t, std::ratio<1>>;
	const TestClassWithSubType expectedObj(seconds_i8(100));
	TestClassWithSubType targetObj = expectedObj;
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive, options);

	targetObj.Assert(expectedObj);
}

TEST(STD_Chrono, ShouldLoadDurationWithRoundingSecondFractions)
{
	nanoseconds testDurNs1(1499999999);
	TestLoadingToDifferentType<ArchiveStub>(testDurNs1, milliseconds(1500));
	TestLoadingToDifferentType<ArchiveStub>(testDurNs1, microseconds(1500000));
	TestLoadingToDifferentType<ArchiveStub>(testDurNs1, seconds(1));

	nanoseconds testDurNs2(2494354999);
	TestLoadingToDifferentType<ArchiveStub>(testDurNs2, milliseconds(2494));
	TestLoadingToDifferentType<ArchiveStub>(testDurNs2, microseconds(2494355));
	TestLoadingToDifferentType<ArchiveStub>(testDurNs2, seconds(2));
}

TEST(STD_Chrono, ThrowOverflowExceptionWhenDurationCannotBeRounded)
{
	// Arrange
	TestClassWithSubType<std::string> isoDuration("PT9M59S");
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(isoDuration, outputArchive);

	// Act / Assert
	try
	{
		TestClassWithSubType<minutes> actual;
		BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
		EXPECT_FALSE(true) << "Seconds should not be rounded to minutes";
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
	}
}

TEST(STD_Chrono, SkipErrorOfRoundingDurationWhenPolicyIsSkip)
{
	// Arrange
	TestClassWithSubType<std::string> isoDuration("PT9M59S");
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(isoDuration, outputArchive);

	// Act
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	minutes actual(0);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive, options);

	// Assert
	EXPECT_EQ(0, actual.count());
}

TEST(STD_Chrono, SerializeArrayOfDurations) {
	TestSerializeStlContainer<ArchiveStub, std::array<seconds, 100>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for time_point (to binary archive)
//-----------------------------------------------------------------------------
TEST(STD_ChronoAsBin, SerializeTimePoint)
{
	auto tpSec = time_point<system_clock, milliseconds>(100s);
	TestSerializeType<BinArchiveStub>(tpSec);

	auto tpMs = time_point<system_clock, milliseconds>(1999ms);
	TestSerializeType<BinArchiveStub>(tpMs);

	auto tpUs = time_point<system_clock, microseconds>(1999999us);
	TestSerializeType<BinArchiveStub>(tpUs);

	auto tpNs = time_point<system_clock, nanoseconds>(1999999999ns);
	TestSerializeType<BinArchiveStub>(tpNs);
}

TEST(STD_ChronoAsBin, SerializeTimePointMaxValues)
{
	using TimePointSec = time_point<system_clock, seconds>;
	TestSerializeType<BinArchiveStub>(TimePointSec::min());
	TestSerializeType<BinArchiveStub>(TimePointSec::max());

	using TimePointMs = time_point<system_clock, milliseconds>;
	TestSerializeType<BinArchiveStub>(TimePointMs::min());
	TestSerializeType<BinArchiveStub>(TimePointMs::max());

	using TimePointMu = time_point<system_clock, microseconds>;
	TestSerializeType<BinArchiveStub>(TimePointMu::min());
	TestSerializeType<BinArchiveStub>(TimePointMu::max());

	using TimePointNs = time_point<system_clock, nanoseconds>;
	TestSerializeType<BinArchiveStub>(TimePointNs::min());
	TestSerializeType<BinArchiveStub>(TimePointNs::max());
}

TEST(STD_ChronoAsBin, ThrowOverflowExceptionWhenSaveTooBigTimepoint)
{
	try
	{
		using TimePointMinutes = time_point<system_clock, duration<int64_t, std::ratio<60>>>;
		// Maximum minutes can't be serialized as binary
		auto tp = TimePointMinutes::max();
		BitSerializer::SaveObject<BinArchiveStub>(tp);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(STD_ChronoAsBin, ThrowOverflowExceptionWhenLoadTooBigTimestamp)
{
	// Arrange
	BinArchiveStub::preferred_output_format outputArchive;
	Detail::CBinTimestamp timestamp(std::numeric_limits<int64_t>::max());
	outputArchive.emplace<Detail::CBinTimestamp>(timestamp);

	// Act / Assert
	try
	{
		using TimePoint = time_point<system_clock, nanoseconds>;
		// Max timestamp can't be deserialized to TimePoint with nanoseconds as duration
		TimePoint tp;
		BitSerializer::LoadObject<BinArchiveStub>(tp, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(STD_ChronoAsBin, SkipTooBigTimestampWhenPolicyIsSkip)
{
	// Arrange
	BinArchiveStub::preferred_output_format outputArchive;
	Detail::CBinTimestamp timestamp(std::numeric_limits<int64_t>::max());
	outputArchive.emplace<Detail::CBinTimestamp>(timestamp);

	// Load as time_point
	using TimePoint = time_point<system_clock, nanoseconds>;
	TimePoint tp{};
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	BitSerializer::LoadObject<BinArchiveStub>(tp, outputArchive, options);
	EXPECT_EQ(TimePoint{}, tp);
}

TEST(STD_ChronoAsBin, SerializeTimePointAsClassMember) {
	TestClassWithSubType<time_point<system_clock, nanoseconds>> testEntity;
	TestSerializeClass<BinArchiveStub>(testEntity);
}

TEST(STD_ChronoAsBin, ThrowOverflowExceptionWhenLoadTooBigTimestampFromObject)
{
	// Arrange
	using TimePoint = time_point<system_clock, nanoseconds>;
	using TestObject = TestClassWithSubType<TimePoint>;

	BinArchiveStub::preferred_output_format outputArchive;
	Detail::CBinTimestamp timestamp(std::numeric_limits<int64_t>::max());
	auto& binObjRef = outputArchive.emplace<Detail::BinTestIoDataObject>();
	Detail::BinTestIoData timestampIoData;
	timestampIoData.emplace<Detail::CBinTimestamp>(timestamp);
	binObjRef.emplace(std::string(TestObject::KeyName), std::move(timestampIoData));

	// Act / Assert
	try
	{
		// Max timestamp can't be deserialized to TimePoint with nanoseconds as duration
		TestObject testEntity;
		BitSerializer::LoadObject<BinArchiveStub>(testEntity, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(STD_ChronoAsBin, SkipTooBigTimestampInObjectWhenPolicyIsSkip)
{
	// Arrange
	using TimePoint = time_point<system_clock, nanoseconds>;
	using TestObject = TestClassWithSubType<TimePoint>;

	BinArchiveStub::preferred_output_format outputArchive;
	Detail::CBinTimestamp timestamp(std::numeric_limits<int64_t>::max());
	auto& binObjRef = outputArchive.emplace<Detail::BinTestIoDataObject>();
	Detail::BinTestIoData timestampIoData;
	timestampIoData.emplace<Detail::CBinTimestamp>(timestamp);
	binObjRef.emplace(std::string(TestObject::KeyName), std::move(timestampIoData));

	TestObject testObj;
	const auto expected = testObj.GetValue();

	// Act
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	BitSerializer::LoadObject<BinArchiveStub>(testObj, outputArchive, options);

	// Assert
	EXPECT_EQ(expected, testObj.GetValue());
}

TEST(STD_ChronoAsBin, ShouldLoadTimepointWithRoundingSecondFractions)
{
	time_point<system_clock, nanoseconds> testTpNs1(1499999999ns);
	TestLoadingToDifferentType<BinArchiveStub>(testTpNs1, time_point<system_clock, milliseconds>(1500ms));
	TestLoadingToDifferentType<BinArchiveStub>(testTpNs1, time_point<system_clock, microseconds>(1500000us));
	TestLoadingToDifferentType<BinArchiveStub>(testTpNs1, time_point<system_clock, seconds>(1s));

	time_point<system_clock, nanoseconds> testTpNs2(2494354999ns);
	TestLoadingToDifferentType<BinArchiveStub>(testTpNs2, time_point<system_clock, milliseconds>(2494ms));
	TestLoadingToDifferentType<BinArchiveStub>(testTpNs2, time_point<system_clock, microseconds>(2494355us));
	TestLoadingToDifferentType<BinArchiveStub>(testTpNs2, time_point<system_clock, seconds>(2s));
}

TEST(STD_ChronoAsBin, ThrowOverflowExceptionWhenTimepointCannotBeRounded)
{
	// Arrange
	Detail::CBinTimestamp timestamp(119, 999999999);
	BinArchiveStub::preferred_output_format binArchive;
	binArchive.emplace<Detail::CBinTimestamp>(timestamp);

	// Act / Assert
	try
	{
		time_point<system_clock, minutes> actual;
		BitSerializer::LoadObject<BinArchiveStub>(actual, binArchive);
		EXPECT_FALSE(true) << "Seconds should not be rounded to minutes";
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
	}
}

TEST(STD_ChronoAsBin, SkipErrorOfRoundingTimepointWhenPolicyIsSkip)
{
	// Arrange
	Detail::CBinTimestamp timestamp(119, 999999999);
	BinArchiveStub::preferred_output_format binArchive;
	binArchive.emplace<Detail::CBinTimestamp>(timestamp);

	// Act
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	time_point<system_clock, minutes> actual(0min);
	BitSerializer::LoadObject<BinArchiveStub>(actual, binArchive, options);

	// Assert
	EXPECT_EQ(0, actual.time_since_epoch().count());
}

TEST(STD_ChronoAsBin, SerializeArrayOfTimePoints)
{
	using TimePointMs = time_point<system_clock, nanoseconds>;
	TestSerializeStlContainer<BinArchiveStub, std::array<TimePointMs, 100>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for duration (to binary archive)
//-----------------------------------------------------------------------------
TEST(STD_ChronoAsBin, SerializeDuration)
{
	seconds durSec(-575);
	TestSerializeType<BinArchiveStub>(durSec);

	milliseconds durMs(1001);
	TestSerializeType<BinArchiveStub>(durMs);

	microseconds durUs(1999999);
	TestSerializeType<BinArchiveStub>(durUs);

	nanoseconds durNs(1000000001);
	TestSerializeType<BinArchiveStub>(durNs);
}

TEST(STD_ChronoAsBin, SerializeDurationMaxValues)
{
	TestSerializeType<BinArchiveStub>(nanoseconds::min());
	TestSerializeType<BinArchiveStub>(nanoseconds::max());

	TestSerializeType<BinArchiveStub>(microseconds::min());
	TestSerializeType<BinArchiveStub>(microseconds::min());

	TestSerializeType<BinArchiveStub>(milliseconds::min());
	TestSerializeType<BinArchiveStub>(milliseconds::min());

	TestSerializeType<BinArchiveStub>(seconds::min());
	TestSerializeType<BinArchiveStub>(seconds::max());

	TestSerializeType<BinArchiveStub>(duration_cast<hours>(seconds::min()));
	TestSerializeType<BinArchiveStub>(duration_cast<hours>(seconds::max()));
}

TEST(STD_ChronoAsBin, SerializeDurationAsClassMember)
{
	TestClassWithSubType<seconds> testEntitySec;
	TestSerializeClass<BinArchiveStub>(testEntitySec);

	TestClassWithSubType<milliseconds> testEntityMs;
	TestSerializeClass<BinArchiveStub>(testEntityMs);

	TestClassWithSubType<microseconds> testEntityUs;
	TestSerializeClass<BinArchiveStub>(testEntityUs);

	TestClassWithSubType<nanoseconds> testEntityNs;
	TestSerializeClass<BinArchiveStub>(testEntityNs);
}

TEST(STD_ChronoAsBin, ThrowOverflowExceptionWhenLoadTooBigDuration)
{
	// Arrange
	Detail::CBinTimestamp timestamp(256);
	BinArchiveStub::preferred_output_format binArchive;
	binArchive.emplace<Detail::CBinTimestamp>(timestamp);

	// Act / Assert
	try
	{
		duration<uint8_t> actual(0);
		BitSerializer::LoadObject<BinArchiveStub>(actual, binArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true) << "Exception was not thrown on a time overflow";
}

TEST(STD_ChronoAsBin, SkipTooBigDurationWhenPolicyIsSkip)
{
	// Arrange
	Detail::CBinTimestamp timestamp(119, 999999999);
	BinArchiveStub::preferred_output_format binArchive;
	binArchive.emplace<Detail::CBinTimestamp>(timestamp);

	// Act
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	minutes actual(0);
	BitSerializer::LoadObject<BinArchiveStub>(actual, binArchive, options);

	// Assert
	EXPECT_EQ(0, actual.count());
}

TEST(STD_ChronoAsBin, ShouldLoadDurationWithRoundingSecondFractions)
{
	nanoseconds testDurNs1(1499999999);
	TestLoadingToDifferentType<BinArchiveStub>(testDurNs1, milliseconds(1500));
	TestLoadingToDifferentType<BinArchiveStub>(testDurNs1, microseconds(1500000));
	TestLoadingToDifferentType<BinArchiveStub>(testDurNs1, seconds(1));

	nanoseconds testDurNs2(2494354999);
	TestLoadingToDifferentType<BinArchiveStub>(testDurNs2, milliseconds(2494));
	TestLoadingToDifferentType<BinArchiveStub>(testDurNs2, microseconds(2494355));
	TestLoadingToDifferentType<BinArchiveStub>(testDurNs2, seconds(2));
}

TEST(STD_ChronoAsBin, ThrowOverflowExceptionWhenDurationCannotBeRounded)
{
	// Arrange
	Detail::CBinTimestamp timestamp(119, 999999999);
	BinArchiveStub::preferred_output_format binArchive;
	binArchive.emplace<Detail::CBinTimestamp>(timestamp);

	// Act / Assert
	try
	{
		minutes actual;
		BitSerializer::LoadObject<BinArchiveStub>(actual, binArchive);
		EXPECT_FALSE(true) << "Seconds should not be rounded to minutes";
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
	}
}

TEST(STD_ChronoAsBin, SkipErrorOfRoundingDurationWhenPolicyIsSkip)
{
	// Arrange
	Detail::CBinTimestamp timestamp(119, 999999999);
	BinArchiveStub::preferred_output_format binArchive;
	binArchive.emplace<Detail::CBinTimestamp>(timestamp);

	// Act
	SerializationOptions options;
	options.overflowNumberPolicy = OverflowNumberPolicy::Skip;
	minutes actual(0);
	BitSerializer::LoadObject<BinArchiveStub>(actual, binArchive, options);

	// Assert
	EXPECT_EQ(0, actual.count());
}

TEST(STD_ChronoAsBin, SerializeArrayOfDurations) {
	TestSerializeStlContainer<BinArchiveStub, std::array<seconds, 100>>();
}
