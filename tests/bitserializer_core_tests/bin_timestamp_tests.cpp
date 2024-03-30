/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <limits>
#include <gtest/gtest.h>
#include "bitserializer/convert.h"
#include "bitserializer/serialization_detail/bin_timestamp.h"

using namespace std::chrono;
using namespace BitSerializer;
using CBinTimestamp = BitSerializer::Detail::CBinTimestamp;

//-----------------------------------------------------------------------------
// Tests CBinTimestamp
//-----------------------------------------------------------------------------
TEST(BinTimestamp, ShouldConstruct)
{
	const CBinTimestamp timestamp(std::numeric_limits<int64_t>::max(), std::numeric_limits<int32_t>::max());
	EXPECT_EQ(std::numeric_limits<int64_t>::max(), timestamp.Seconds);
	EXPECT_EQ(std::numeric_limits<int32_t>::max(), timestamp.Nanoseconds);
}

TEST(BinTimestamp, ShouldComparableViaEqualOperator)
{
	const CBinTimestamp timestamp1(10, 20);
	const CBinTimestamp timestamp2(10, 20);
	const CBinTimestamp timestamp3(10, 21);
	EXPECT_TRUE(timestamp1 == timestamp2);
	EXPECT_FALSE(timestamp1 == timestamp3);
}

//-----------------------------------------------------------------------------
// Tests conversion CBinTimestamp from/to std::chrono::timepoint
//-----------------------------------------------------------------------------
TEST(BinTimestamp, ConvertFromChronoTimepoint)
{
	constexpr auto tpSec = time_point<system_clock, milliseconds>(100s);
	const auto timepoint1 = Convert::To<CBinTimestamp>(tpSec);
	EXPECT_EQ(100, timepoint1.Seconds);
	EXPECT_EQ(0, timepoint1.Nanoseconds);

	constexpr auto tpMs = time_point<system_clock, milliseconds>(1999ms);
	const auto timepoint2 = Convert::To<CBinTimestamp>(tpMs);
	EXPECT_EQ(1, timepoint2.Seconds);
	EXPECT_EQ(999000000, timepoint2.Nanoseconds);

	constexpr auto tpUs = time_point<system_clock, microseconds>(1999999us);
	const auto timepoint3 = Convert::To<CBinTimestamp>(tpUs);
	EXPECT_EQ(1, timepoint3.Seconds);
	EXPECT_EQ(999999000, timepoint3.Nanoseconds);

	constexpr auto tpNs = time_point<system_clock, nanoseconds>(1999999999ns);
	const auto timepoint4 = Convert::To<CBinTimestamp>(tpNs);
	EXPECT_EQ(1, timepoint4.Seconds);
	EXPECT_EQ(999999999, timepoint4.Nanoseconds);
}

TEST(BinTimestamp, ConvertFromChronoTimepointThrowExceptionWhenOverflow)
{
	using TimePointMinutes = time_point<system_clock, duration<int64_t, std::ratio<60>>>;
	// Maximum timepoint with minutes duration can't be represented in the CBinTimestamp
	EXPECT_THROW(Convert::To<CBinTimestamp>(TimePointMinutes::max()), std::out_of_range);
}

TEST(BinTimestamp, ConvertToChronoTimepoint)
{
	using TimePointSec = time_point<system_clock, seconds>;
	const auto tpSec = Convert::To<TimePointSec>(CBinTimestamp(100, 0));
	EXPECT_EQ(100, tpSec.time_since_epoch().count());

	using TimePointMs = time_point<system_clock, milliseconds>;
	const auto tpMs = Convert::To<TimePointMs>(CBinTimestamp(1, 999000000));
	EXPECT_EQ(1999, tpMs.time_since_epoch().count());

	using TimePointUs = time_point<system_clock, microseconds>;
	const auto tpUs = Convert::To<TimePointUs>(CBinTimestamp(1, 999999000));
	EXPECT_EQ(1999999, tpUs.time_since_epoch().count());

	using TimePointNs = time_point<system_clock, nanoseconds>;
	const auto tpNs = Convert::To<TimePointNs>(CBinTimestamp(1, 999999999));
	EXPECT_EQ(1999999999, tpNs.time_since_epoch().count());
}

TEST(BinTimestamp, ConvertToChronoTimepointThrowExceptionWhenOverflow)
{
	CBinTimestamp timestamp(std::numeric_limits<int64_t>::max());
	using TimePointNs = time_point<system_clock, nanoseconds>;
	// Max timestamp can't be converted to TimePoint with nanoseconds as duration
	EXPECT_THROW(Convert::To<TimePointNs>(timestamp), std::out_of_range);
}

TEST(BinTimestamp, ConvertToChronoTimepointWithAllowedRounding)
{
	using TimePointSec = time_point<system_clock, seconds>;
	const auto tpSec = Convert::To<TimePointSec>(CBinTimestamp(100, 555555555));
	EXPECT_EQ(101, tpSec.time_since_epoch().count());

	using TimePointMs = time_point<system_clock, milliseconds>;
	const auto tpMs = Convert::To<TimePointMs>(CBinTimestamp(1, 555444444));
	EXPECT_EQ(1555, tpMs.time_since_epoch().count());

	using TimePointUs = time_point<system_clock, microseconds>;
	const auto tpUs = Convert::To<TimePointUs>(CBinTimestamp(1, 555555555));
	EXPECT_EQ(1555556, tpUs.time_since_epoch().count());
}

//-----------------------------------------------------------------------------
// Tests conversion CBinTimestamp from/to std::chrono::duration
//-----------------------------------------------------------------------------

TEST(BinTimestamp, ShouldConvertFromChronoDuration)
{
	const auto timepoint1 = Convert::To<CBinTimestamp>(100s);
	EXPECT_EQ(100, timepoint1.Seconds);
	EXPECT_EQ(0, timepoint1.Nanoseconds);

	const auto timepoint2 = Convert::To<CBinTimestamp>(1999ms);
	EXPECT_EQ(1, timepoint2.Seconds);
	EXPECT_EQ(999000000, timepoint2.Nanoseconds);

	const auto timepoint3 = Convert::To<CBinTimestamp>(1999999us);
	EXPECT_EQ(1, timepoint3.Seconds);
	EXPECT_EQ(999999000, timepoint3.Nanoseconds);

	const auto timepoint4 = Convert::To<CBinTimestamp>(1999999999ns);
	EXPECT_EQ(1, timepoint4.Seconds);
	EXPECT_EQ(999999999, timepoint4.Nanoseconds);
}

TEST(BinTimestamp, ConvertFromChronoDurationThrowExceptionWhenOverflow)
{
	CBinTimestamp timestamp(std::numeric_limits<int64_t>::max());
	// Max timestamp can't be converted to duration with nanoseconds as duration
	EXPECT_THROW(Convert::To<nanoseconds>(timestamp), std::out_of_range);
}

TEST(BinTimestamp, ConvertToChronoDuration)
{
	const auto sec = Convert::To<seconds>(CBinTimestamp(100, 0));
	EXPECT_EQ(100, sec.count());

	const auto ms = Convert::To<milliseconds>(CBinTimestamp(1, 999000000));
	EXPECT_EQ(1999, ms.count());

	const auto us = Convert::To<microseconds>(CBinTimestamp(1, 999999000));
	EXPECT_EQ(1999999, us.count());

	const auto ns = Convert::To<nanoseconds>(CBinTimestamp(1, 999999999));
	EXPECT_EQ(1999999999, ns.count());
}

TEST(BinTimestamp, ConvertToChronoDurationThrowExceptionWhenOverflow)
{
	using Minutes = duration<int64_t, std::ratio<60>>;
	// Maximum minutes can't be represented in the CBinTimestamp
	EXPECT_THROW(Convert::To<CBinTimestamp>(Minutes::max()), std::out_of_range);
}

TEST(BinTimestamp, ConvertToChronoDurationWithAllowedRounding)
{
	const auto sec = Convert::To<seconds>(CBinTimestamp(100, 555555555));
	EXPECT_EQ(101, sec.count());

	const auto ms = Convert::To<milliseconds>(CBinTimestamp(1, 555444444));
	EXPECT_EQ(1555, ms.count());

	const auto us = Convert::To<microseconds>(CBinTimestamp(1, 555555555));
	EXPECT_EQ(1555556, us.count());
}
