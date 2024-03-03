/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/archive_stub.h"
#include "testing_tools/bin_archive_stub.h"

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
	ArchiveStub::preferred_output_format testArchive;

	// Act
	BitSerializer::SaveObject<ArchiveStub>(CTimeRef(expected), testArchive);
	BitSerializer::LoadObject<ArchiveStub>(CTimeRef(actual), testArchive);

	// Assert
	EXPECT_EQ(expected, actual);
}

TEST(STD_Ctime, ThrowMismatchedTypesExceptionWhenLoadInvalidIsoDate)
{
	// Save as string
	std::string invalidDatetime = "1970/01/01T00:00:00Z";
	ArchiveStub::preferred_output_format testArchive;
	BitSerializer::SaveObject<ArchiveStub>(invalidDatetime, testArchive);

	try
	{
		// Load as `time_t`
		time_t actual = 0;
		BitSerializer::LoadObject<ArchiveStub>(CTimeRef(actual), testArchive);
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
	ArchiveStub::preferred_output_format testArchive;
	BitSerializer::SaveObject<ArchiveStub>(invalidDatetime, testArchive);

	// Load as `time_t`
	time_t actual = 0;
	SerializationOptions options;
	options.mismatchedTypesPolicy = MismatchedTypesPolicy::Skip;
	BitSerializer::LoadObject<ArchiveStub>(CTimeRef(actual), testArchive, options);

	EXPECT_EQ(0, actual);
}

TEST(STD_Ctime, SerializeTimeTypeAsClassMember)
{
	// Arrange
	TestCTime expected(2335219200), actual;
	ArchiveStub::preferred_output_format testArchive;

	// Act
	BitSerializer::SaveObject<ArchiveStub>(expected, testArchive);
	BitSerializer::LoadObject<ArchiveStub>(actual, testArchive);

	// Assert
	EXPECT_EQ(expected.Time, actual.Time);
}

TEST(STD_Ctime, SkipInvalidIsoDateWhenPolicyIsSkipFromObject)
{
	// Arrange
	ArchiveStub::preferred_output_format testArchive;
	auto& binObjRef = testArchive.emplace<Detail::TestIoDataObject>();
	Detail::TestIoData strIoData;
	strIoData.emplace<std::wstring>(L"Invalid date");
	binObjRef.emplace(std::wstring(L"Time"), std::move(strIoData));

	// Act / Assert
	try
	{
		TestCTime testEntity;
		BitSerializer::LoadObject<ArchiveStub>(testEntity, testArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(STD_CtimeAsBin, SkipInvalidIsoDateInObjectWhenPolicyIsSkip)
{
	// Arrange
	ArchiveStub::preferred_output_format testArchive;
	auto& binObjRef = testArchive.emplace<Detail::TestIoDataObject>();
	Detail::TestIoData strIoData;
	strIoData.emplace<std::wstring>(L"Invalid date");
	binObjRef.emplace(std::wstring(L"Time"), std::move(strIoData));
	TestCTime testEntity;
	auto expected = testEntity.Time;

	// Act
	SerializationOptions options;
	options.mismatchedTypesPolicy = MismatchedTypesPolicy::Skip;
	BitSerializer::LoadObject<ArchiveStub>(testEntity, testArchive, options);

	// Assert
	EXPECT_EQ(expected, testEntity.Time);
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::time_t (to binary archive)
//-----------------------------------------------------------------------------
TEST(STD_CtimeAsBin, SerializeTimeType)
{
	// Arrange
	time_t expected = 2335219200, actual = 0;
	BinArchiveStub::preferred_output_format binArchive;

	// Act
	BitSerializer::SaveObject<BinArchiveStub>(CTimeRef(expected), binArchive);
	BitSerializer::LoadObject<BinArchiveStub>(CTimeRef(actual), binArchive);

	// Assert
	EXPECT_EQ(expected, actual);
}

TEST(STD_CtimeAsBin, ShouldIgnoreNanosecondsPart)
{
	// Arrange
	Detail::CBinTimestamp timestamp(59, 999999999);
	BinArchiveStub::preferred_output_format binArchive;
	binArchive.emplace<Detail::CBinTimestamp>(timestamp);

	time_t actual = 0;
	BitSerializer::LoadObject<BinArchiveStub>(CTimeRef(actual), binArchive);

	// Assert
	EXPECT_EQ(timestamp.Seconds, actual);
}

TEST(STD_CtimeAsBin, SerializeTimeTypeAsClassMember)
{
	// Arrange
	TestCTime expected(2335219200), actual;
	BinArchiveStub::preferred_output_format outputArchive;

	// Act
	BitSerializer::SaveObject<BinArchiveStub>(expected, outputArchive);
	BitSerializer::LoadObject<BinArchiveStub>(actual, outputArchive);

	// Assert
	EXPECT_EQ(expected.Time, actual.Time);
}

TEST(STD_CtimeAsBin, ShouldIgnoreNanosecondsWhenLoadFromObject)
{
	// Arrange
	BinArchiveStub::preferred_output_format binArchive;
	Detail::CBinTimestamp timestamp(59, 999999999);
	auto& binObjRef = binArchive.emplace<Detail::BinTestIoDataObject>();
	Detail::BinTestIoData timestampIoData;
	timestampIoData.emplace<Detail::CBinTimestamp>(timestamp);
	binObjRef.emplace(std::string("Time"), std::move(timestampIoData));

	// Act
	TestCTime testEntity;
	BitSerializer::LoadObject<BinArchiveStub>(testEntity, binArchive);

	// Assert
	EXPECT_EQ(timestamp.Seconds, testEntity.Time);
}
