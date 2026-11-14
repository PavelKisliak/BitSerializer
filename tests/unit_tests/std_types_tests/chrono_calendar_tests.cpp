/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/archive_stub.h"

#include "bitserializer/types/std/chrono.h"

using namespace std::chrono;
using namespace BitSerializer;

#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
//-----------------------------------------------------------------------------
// Tests of serialization for C++20 calendar types
//-----------------------------------------------------------------------------
TEST(STD_ChronoCalendar, SerializeYearMonthDay)
{
	const year_month_day expected{year{2024}, month{2}, day{29}};
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);

	year_month_day actual{year{1970}, month{1}, day{1}};
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	EXPECT_TRUE(expected == actual);
}

TEST(STD_ChronoCalendar, SerializeYearMonthDayBoundaryYears)
{
	for (const auto& expected : { year_month_day{year{-32767}, month{1}, day{1}}, year_month_day{year{32767}, month{12}, day{31}} })
	{
		ArchiveStub::preferred_output_type outputArchive{};
		BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);

		year_month_day actual{year{2000}, month{1}, day{1}};
		BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
		EXPECT_TRUE(expected == actual);
	}
}

TEST(STD_ChronoCalendar, ConvertYearMonthDayToString)
{
	EXPECT_EQ("2024-02-29", Convert::ToString(year_month_day{year{2024}, month{2}, day{29}}));
	EXPECT_EQ("+10000-01-01", Convert::ToString(year_month_day{year{10000}, month{1}, day{1}}));
	EXPECT_EQ("-0001-12-31", Convert::ToString(year_month_day{year{-1}, month{12}, day{31}}));
}

TEST(STD_ChronoCalendar, ThrowWhenYearMonthDayIsInvalid)
{
	EXPECT_THROW(Convert::To<year_month_day>("2024-02-30"), std::invalid_argument);
	EXPECT_THROW(Convert::To<year_month_day>("2024-13-01"), std::invalid_argument);
	EXPECT_THROW(Convert::To<year_month_day>("2024-01"), std::invalid_argument);
	EXPECT_THROW(Convert::ToString(year_month_day{year{2023}, month{2}, day{29}}), std::invalid_argument);
}

TEST(STD_ChronoCalendar, SerializeHhMmSs)
{
	using TestDuration = milliseconds;
	const hh_mm_ss<TestDuration> expected{hours{12} + minutes{34} + seconds{56} + milliseconds{789}};
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);

	hh_mm_ss<TestDuration> actual{TestDuration{0}};
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	EXPECT_EQ(expected.to_duration().count(), actual.to_duration().count());
}

TEST(STD_ChronoCalendar, SerializeNegativeHhMmSs)
{
	using TestDuration = milliseconds;
	const hh_mm_ss<TestDuration> expected{TestDuration{-45296789}};
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);

	hh_mm_ss<TestDuration> actual{TestDuration{0}};
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	EXPECT_EQ(expected.to_duration().count(), actual.to_duration().count());
	EXPECT_EQ(expected.is_negative(), actual.is_negative());
}

TEST(STD_ChronoCalendar, ConvertHhMmSsToString)
{
	using TestDuration = milliseconds;
	EXPECT_EQ("12:34:56.789", Convert::ToString(hh_mm_ss<TestDuration>{hours{12} + minutes{34} + seconds{56} + milliseconds{789}}));
	EXPECT_EQ("12:34:56.78", Convert::ToString(hh_mm_ss<TestDuration>{hours{12} + minutes{34} + seconds{56} + milliseconds{780}}));
	EXPECT_EQ("12:34:56", Convert::ToString(hh_mm_ss<TestDuration>{hours{12} + minutes{34} + seconds{56}}));
	EXPECT_EQ("-12:34:56.789", Convert::ToString(hh_mm_ss<TestDuration>{TestDuration{-45296789}}));
	EXPECT_EQ("00:00:00", Convert::ToString(hh_mm_ss<seconds>{seconds{0}}));
	EXPECT_EQ("36:00:00", Convert::ToString(hh_mm_ss<seconds>{hours{36}}));
}

TEST(STD_ChronoCalendar, ThrowWhenHhMmSsIsInvalid)
{
	using TestDuration = milliseconds;
	EXPECT_THROW(Convert::To<hh_mm_ss<TestDuration>>("12:60:00"), std::invalid_argument);
	EXPECT_THROW(Convert::To<hh_mm_ss<TestDuration>>("12-34-56"), std::invalid_argument);
	EXPECT_THROW(Convert::To<hh_mm_ss<seconds>>("12:34:56.789"), std::invalid_argument);
	EXPECT_THROW(Convert::To<hh_mm_ss<hours>>("12:34:56"), std::out_of_range);
}

TEST(STD_ChronoCalendar, SerializeToBinaryArchive)
{
	const year_month_day expectedDate{year{2024}, month{2}, day{29}};
	BinArchiveStub::preferred_output_type dateArchive{};
	BitSerializer::SaveObject<BinArchiveStub>(expectedDate, dateArchive);

	year_month_day actualDate{year{1970}, month{1}, day{1}};
	BitSerializer::LoadObject<BinArchiveStub>(actualDate, dateArchive);
	EXPECT_TRUE(expectedDate == actualDate);

	using TestDuration = milliseconds;
	const hh_mm_ss<TestDuration> expectedTime{hours{12} + minutes{34} + seconds{56} + milliseconds{789}};
	BinArchiveStub::preferred_output_type timeArchive{};
	BitSerializer::SaveObject<BinArchiveStub>(expectedTime, timeArchive);

	hh_mm_ss<TestDuration> actualTime{TestDuration{0}};
	BitSerializer::LoadObject<BinArchiveStub>(actualTime, timeArchive);
	EXPECT_EQ(expectedTime.to_duration().count(), actualTime.to_duration().count());
}
#endif
