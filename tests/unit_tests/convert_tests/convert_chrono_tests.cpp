/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <cstdint>
#include <gtest/gtest.h>
#include "bitserializer/convert.h"

using namespace BitSerializer;
using namespace std::chrono;

//-----------------------------------------------------------------------------
// Test data
//-----------------------------------------------------------------------------
using TimePointSec = time_point<system_clock, seconds>;
using TimePointMs = time_point<system_clock, milliseconds>;
using TimePointUs = time_point<system_clock, microseconds>;
using TimePointNs = time_point<system_clock, nanoseconds>;

// Year before introduction of the Gregorian calendar are not automatically allowed by the standard
constexpr auto tp0000_01_01T00_00_00 = TimePointSec(seconds(-62167219200));
// First year that allowed by the ISO 8601 standard
constexpr auto tp1583_01_01T00_00_00 = TimePointSec(seconds(-12212553600));
// Min timepoint for nanoseconds as duration
constexpr auto tp1677_09_21T00_12_44 = std::chrono::time_point_cast<std::chrono::seconds>(TimePointNs::min());
constexpr auto tp1872_01_01T00_00_00 = TimePointSec(seconds(-3092601600));
constexpr auto tp1968_12_31T23_59_59 = TimePointSec(seconds(-31536001));
// Unix time EPOCH
constexpr auto tp1970_01_01T00_00_00 = TimePointSec(seconds(0));
constexpr auto tp2044_01_01T00_00_00 = TimePointSec(seconds(2335219200));
// Max timepoint for nanoseconds as duration
constexpr auto tp2262_04_11T23_47_16 = std::chrono::time_point_cast<std::chrono::seconds>(TimePointNs::max());
constexpr auto tp9999_12_31T23_59_59 = TimePointSec(seconds(253402300799));
constexpr auto tp10000_01_01T00_00_00 = TimePointSec(seconds(253402300800));

//-----------------------------------------------------------------------------
// Test conversion from std::chrono::system_clock::time_point to std::string
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertTimePointToUtcString) {
	EXPECT_EQ("0000-01-01T00:00:00Z", Convert::ToString(tp0000_01_01T00_00_00));
	EXPECT_EQ("1583-01-01T00:00:00Z", Convert::ToString(tp1583_01_01T00_00_00));
	EXPECT_EQ("1677-09-21T00:12:44Z", Convert::ToString(tp1677_09_21T00_12_44));
	EXPECT_EQ("1872-01-01T00:00:00Z", Convert::ToString(tp1872_01_01T00_00_00));
	EXPECT_EQ("1968-12-31T23:59:59Z", Convert::ToString(tp1968_12_31T23_59_59));
	EXPECT_EQ("1970-01-01T00:00:00Z", Convert::ToString(tp1970_01_01T00_00_00));
	EXPECT_EQ(u"2044-01-01T00:00:00Z", Convert::To<std::u16string>(tp2044_01_01T00_00_00));
	EXPECT_EQ(U"2262-04-11T23:47:16Z", Convert::To<std::u32string>(tp2262_04_11T23_47_16));
	EXPECT_EQ(L"9999-12-31T23:59:59Z", Convert::To<std::wstring>(tp9999_12_31T23_59_59));
	EXPECT_EQ(L"+10000-01-01T00:00:00Z", Convert::To<std::wstring>(tp10000_01_01T00_00_00));
}

TEST(ConvertChrono, ConvertTimePointWithMsToUtcString) {
	EXPECT_EQ("1677-09-21T00:12:44.999Z", Convert::ToString(tp1677_09_21T00_12_44 + 999ms));
	EXPECT_EQ("1872-01-01T00:00:00.001Z", Convert::ToString(tp1872_01_01T00_00_00 + 1ms));
	EXPECT_EQ("1968-12-31T23:59:59.567Z", Convert::ToString(tp1968_12_31T23_59_59 + 567ms));
	EXPECT_EQ("1970-01-01T00:00:00.025Z", Convert::ToString(tp1970_01_01T00_00_00 + 25ms));
	EXPECT_EQ(u"2044-01-01T00:00:00.001Z", Convert::To<std::u16string>(tp2044_01_01T00_00_00 + 1ms));
	EXPECT_EQ(U"2262-04-11T23:47:16.999Z", Convert::To<std::u32string>(tp2262_04_11T23_47_16 + 999ms));
	EXPECT_EQ(L"9999-12-31T23:59:59.999Z", Convert::To<std::wstring>(tp9999_12_31T23_59_59 + 999ms));
}

TEST(ConvertChrono, ConvertTimePointWithUsToUtcString) {
	EXPECT_EQ("1677-09-21T00:12:44.999999Z", Convert::ToString(tp1677_09_21T00_12_44 + 999999us));
	EXPECT_EQ("1872-01-01T00:00:00.000001Z", Convert::ToString(tp1872_01_01T00_00_00 + 1us));
	EXPECT_EQ("1968-12-31T23:59:59.123456Z", Convert::ToString(tp1968_12_31T23_59_59 + 123456us));
	EXPECT_EQ("1970-01-01T00:00:00.005625Z", Convert::ToString(tp1970_01_01T00_00_00 + 5625us));
}

TEST(ConvertChrono, ConvertTimePointWithNsToUtcString) {
	EXPECT_EQ("1677-09-21T00:12:44.999999999Z", Convert::ToString(tp1677_09_21T00_12_44 + 999999999ns));
	EXPECT_EQ("1872-01-01T00:00:00.000000001Z", Convert::ToString(tp1872_01_01T00_00_00 + 1ns));
	EXPECT_EQ("1968-12-31T23:59:59.123456789Z", Convert::ToString(tp1968_12_31T23_59_59 + 123456789ns));
	EXPECT_EQ("1970-01-01T00:00:00.000001125Z", Convert::ToString(tp1970_01_01T00_00_00 + 1125ns));
}

TEST(ConvertChrono, ConvertTimePointToUtcStringMaxValues) {
	using days_i32 = duration<int32_t, std::ratio<86400>>;
	using TimePointDaysI32Rep = time_point<system_clock, days_i32>;
	EXPECT_EQ("+5881580-07-11T00:00:00Z", Convert::ToString(TimePointDaysI32Rep::max()));
	EXPECT_EQ("-5877641-06-23T00:00:00Z", Convert::ToString(TimePointDaysI32Rep::min()));
}

//-----------------------------------------------------------------------------
// Test conversion from std::string_view to std::chrono::system_clock::time_point
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertUtcStringToTimePoint) {
	EXPECT_EQ(tp0000_01_01T00_00_00, Convert::To<TimePointMs>("0000-01-01T00:00:00Z"));
	EXPECT_EQ(tp1583_01_01T00_00_00, Convert::To<TimePointMs>("1583-01-01T00:00:00Z"));
	EXPECT_EQ(tp1677_09_21T00_12_44, Convert::To<TimePointMs>("1677-09-21T00:12:44Z"));
	EXPECT_EQ(tp1872_01_01T00_00_00, Convert::To<TimePointMs>("1872-01-01T00:00:00Z"));
	EXPECT_EQ(tp1968_12_31T23_59_59, Convert::To<TimePointMs>("1968-12-31T23:59:59Z"));
	EXPECT_EQ(tp1970_01_01T00_00_00, Convert::To<TimePointMs>("1970-01-01T00:00:00Z"));
	EXPECT_EQ(tp2044_01_01T00_00_00, Convert::To<TimePointMs>(U"2044-01-01T00:00:00Z"));
	EXPECT_EQ(tp2262_04_11T23_47_16, Convert::To<TimePointMs>(L"2262-04-11T23:47:16Z"));
	EXPECT_EQ(tp9999_12_31T23_59_59, Convert::To<TimePointMs>(L"9999-12-31T23:59:59Z"));
	EXPECT_EQ(tp10000_01_01T00_00_00, Convert::To<TimePointMs>(L"+10000-01-01T00:00:00Z"));
}

TEST(ConvertChrono, ConvertUtcStringWithMsToTimePoint) {
	// With comma as decimal separator
	EXPECT_EQ(tp1677_09_21T00_12_44, Convert::To<TimePointMs>("1677-09-21T00:12:44,000Z"));
	EXPECT_EQ(tp1677_09_21T00_12_44 + 999ms, Convert::To<TimePointMs>("1677-09-21T00:12:44,999Z"));
	// With dot as decimal separator
	EXPECT_EQ(tp1872_01_01T00_00_00 + 1ms, Convert::To<TimePointMs>("1872-01-01T00:00:00.001Z"));
	EXPECT_EQ(tp1968_12_31T23_59_59 + 567ms, Convert::To<TimePointMs>("1968-12-31T23:59:59.567Z"));
	EXPECT_EQ(tp1970_01_01T00_00_00 + 500ms, Convert::To<TimePointMs>("1970-01-01T00:00:00.5Z"));
	EXPECT_EQ(tp2044_01_01T00_00_00 + 10ms, Convert::To<TimePointMs>(U"2044-01-01T00:00:00.01Z"));
	EXPECT_EQ(tp2262_04_11T23_47_16 + 999ms, Convert::To<TimePointMs>(L"2262-04-11T23:47:16.999Z"));
	EXPECT_EQ(tp9999_12_31T23_59_59 + 999ms, Convert::To<TimePointMs>(L"9999-12-31T23:59:59.999Z"));
}

TEST(ConvertChrono, ConvertUtcStringWithUsToTimePoint) {
	EXPECT_EQ(tp1677_09_21T00_12_44 + 999999us,		Convert::To<TimePointUs>("1677-09-21T00:12:44.999999Z"));
	EXPECT_EQ(tp1872_01_01T00_00_00 + 1us,			Convert::To<TimePointUs>("1872-01-01T00:00:00.000001Z"));
	EXPECT_EQ(tp1968_12_31T23_59_59 + 123456us,		Convert::To<TimePointUs>("1968-12-31T23:59:59.123456Z"));
	EXPECT_EQ(tp1970_01_01T00_00_00 + 5625us,		Convert::To<TimePointUs>("1970-01-01T00:00:00.005625Z"));
}

TEST(ConvertChrono, ConvertUtcStringWithNsToTimePoint) {
	EXPECT_EQ(tp1872_01_01T00_00_00 + 999999999ns,	Convert::To<TimePointNs>("1872-01-01T00:00:00.999999999Z"));
	EXPECT_EQ(tp1872_01_01T00_00_00 + 1ns,			Convert::To<TimePointNs>("1872-01-01T00:00:00.000000001Z"));
	EXPECT_EQ(tp1970_01_01T00_00_00 + 123456789ns,	Convert::To<TimePointNs>("1970-01-01T00:00:00.123456789Z"));
	EXPECT_EQ(tp2262_04_11T23_47_16 + 1125ns,		Convert::To<TimePointNs>("2262-04-11T23:47:16.000001125Z"));
}

TEST(ConvertChrono, ConvertUtcStringToTimePointWithRoundingFromMilli) {
	EXPECT_EQ(tp2044_01_01T00_00_00,		Convert::To<TimePointSec>(L"2044-01-01T00:00:00.450Z"));
	EXPECT_EQ(tp2044_01_01T00_00_00 + 1s,	Convert::To<TimePointSec>(L"2044-01-01T00:00:00.501Z"));
	EXPECT_EQ(tp2044_01_01T00_00_00 + 1s,	Convert::To<TimePointSec>(L"2044-01-01T00:00:00.6Z"));
}

TEST(ConvertChrono, ConvertUtcStringToTimePointWithRoundingFromMicro) {
	EXPECT_EQ(tp2044_01_01T00_00_00 + 300ms, Convert::To<TimePointMs>(L"2044-01-01T00:00:00.300499Z"));
	EXPECT_EQ(tp2044_01_01T00_00_00 + 301ms, Convert::To<TimePointMs>(L"2044-01-01T00:00:00.300501Z"));
}

TEST(ConvertChrono, ConvertUtcStringToTimePointWithRoundingFromNano) {
	EXPECT_EQ(tp2044_01_01T00_00_00 + 750255us, Convert::To<TimePointUs>(L"2044-01-01T00:00:00.750255405Z"));
	EXPECT_EQ(tp2044_01_01T00_00_00 + 750256us, Convert::To<TimePointUs>(L"2044-01-01T00:00:00.750255505Z"));
}

TEST(ConvertChrono, ConvertUtcStringToTimePointMaxValues) {
	using hours_i8 = duration<int8_t, std::ratio<3600>>;
	using TimePointHoursI8Rep = time_point<system_clock, hours_i8>;
	EXPECT_EQ(TimePointHoursI8Rep::max(), Convert::To<TimePointHoursI8Rep>("1970-01-06T07:00:00Z"));
	EXPECT_EQ(TimePointHoursI8Rep::min(), Convert::To<TimePointHoursI8Rep>("1969-12-26T16:00:00Z"));

	using days_i32 = duration<int32_t, std::ratio<86400>>;
	using TimePointDaysI32Rep = time_point<system_clock, days_i32>;
	EXPECT_EQ(TimePointDaysI32Rep::max(), Convert::To<TimePointDaysI32Rep>("+5881580-07-11T00:00:00Z"));
	EXPECT_EQ(TimePointDaysI32Rep::min(), Convert::To<TimePointDaysI32Rep>("-5877641-06-23T00:00:00Z"));

	using days_u32 = duration<uint32_t, std::ratio<86400>>;
	using TimePointDaysU32Rep = time_point<system_clock, days_u32>;
	EXPECT_EQ(TimePointDaysU32Rep::max(), Convert::To<TimePointDaysU32Rep>("+11761191-01-20T00:00:00Z"));

	using TimePointNano = time_point<system_clock, nanoseconds>;
	EXPECT_EQ(TimePointNano::max(), Convert::To<TimePointNano>("2262-04-11T23:47:16.854775807Z"));
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenInvalidDelimiters) {
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01 00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970/01/01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00.00.00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:00:00:000Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00-00-00"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenExtraMinus) {
	EXPECT_THROW(Convert::To<TimePointMs>("1970--01-01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01--01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T-00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:-00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:00:-00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:00:00.-000Z"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenInvalidDate) {
	EXPECT_THROW(Convert::To<TimePointMs>("1970-13-01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-00-01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-32T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-00T00:00:00Z"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenInvalidTime) {
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T25:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:60:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:00:60Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:00:00.1234567891Z"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenEmpty) {
	EXPECT_THROW(Convert::To<TimePointNs>(""), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenMissedTimePart) {
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01TZ"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T10:20Z"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenOverflow) {
	EXPECT_THROW(Convert::To<TimePointNs>("0000-09-21T00:12:43Z"), std::out_of_range);
	EXPECT_THROW(Convert::To<TimePointNs>("1677-09-21T00:12:43Z"), std::out_of_range);
	EXPECT_THROW(Convert::To<TimePointNs>("2262-04-11T23:47:17Z"), std::out_of_range);
	EXPECT_THROW(Convert::To<TimePointNs>("2262-04-11T23:47:16.855Z"), std::out_of_range);

	using days_i32 = duration<int32_t, std::ratio<86400>>;
	using TimePointDaysI32Rep = time_point<system_clock, days_i32>;
	EXPECT_THROW(Convert::To<TimePointDaysI32Rep>("+5881580-07-12T00:00:00Z"), std::out_of_range);
	EXPECT_THROW(Convert::To<TimePointDaysI32Rep>("-5877641-06-22T00:00:00Z"), std::out_of_range);

	EXPECT_THROW(Convert::To<TimePointSec>("+292277026596-12-04T15:30:08Z"), std::out_of_range);
	EXPECT_THROW(Convert::To<TimePointSec>("-292277022657-01-27T08:29:51Z"), std::out_of_range);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenNotEnoughPrecision) {
	using days_i64 = duration<int64_t, std::ratio<86400>>;
	using TimePointDays = time_point<system_clock, days_i64>;
	EXPECT_THROW(Convert::To<TimePointDays>("2023-09-01T01:00:00Z"), std::out_of_range);

	using TimePointHours = time_point<system_clock, hours>;
	EXPECT_THROW(Convert::To<TimePointHours>("2023-09-01T00:01:00Z"), std::out_of_range);

	using TimePointMinutes = time_point<system_clock, minutes>;
	EXPECT_THROW(Convert::To<TimePointMinutes>("2023-09-01T00:00:01Z"), std::out_of_range);
}

//-----------------------------------------------------------------------------
// Test conversion from std::chrono::duration to std::string
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertDurationToString) {
	EXPECT_EQ("P1DT1H1M1S", Convert::ToString(25h + 1min + 1s));
	EXPECT_EQ(u"P25DT55M41S", Convert::To<std::u16string>(24h * 25 + 55min + 41s));
	EXPECT_EQ(U"PT10H20S", Convert::To<std::u32string>(10h + 20s));
	EXPECT_EQ(L"P2DT44S", Convert::To<std::wstring>(48h + 44s));
}

TEST(ConvertChrono, ConvertDurationToStringWhenOnlySinglePart) {
	EXPECT_EQ("P1D", Convert::ToString(24h));
	EXPECT_EQ("P1325D", Convert::ToString(24h * 1325));
	EXPECT_EQ("PT1H", Convert::ToString(1h));
	EXPECT_EQ("PT1M", Convert::ToString(1min));
	EXPECT_EQ("PT1S", Convert::ToString(1s));
}

TEST(ConvertChrono, ConvertDurationToStringWhenZeroSeconds) {
	EXPECT_EQ("PT0S", Convert::ToString(0s));
}

TEST(ConvertChrono, ConvertDurationToStringWithMilliFractions) {
	EXPECT_EQ("PT0.5S", Convert::ToString(500ms));
	EXPECT_EQ("PT0.123S", Convert::ToString(123ms));
	EXPECT_EQ("PT1H1.999S", Convert::ToString(1h + 1s + 999ms));
}

TEST(ConvertChrono, ConvertDurationToStringWithMicroFractions) {
	EXPECT_EQ("PT0.5S", Convert::ToString(500000us));
	EXPECT_EQ("PT0.123456S", Convert::ToString(123456us));
	EXPECT_EQ("PT1H0.999999S", Convert::ToString(1h + 999999us));
}

TEST(ConvertChrono, ConvertDurationToStringWithNanoFractions) {
	EXPECT_EQ("PT0.5S", Convert::ToString(500000000ns));
	EXPECT_EQ("PT0.123456789S", Convert::ToString(123456789ns));
	EXPECT_EQ("PT1H0.999999999S", Convert::ToString(1h + 999999999ns));
}

TEST(ConvertChrono, ConvertDurationToStringWhenNegative) {
	EXPECT_EQ("-PT1S", Convert::ToString(-1s));
	EXPECT_EQ("-P10DT25M", Convert::ToString(-24h * 10 - 25min));
	EXPECT_EQ("-P120DT3H3M3S", Convert::ToString(-24h * 120 - 3h - 3min - 3s));
	EXPECT_EQ("-P3450DT15H59M59.125S", Convert::ToString(-24h * 3450 - 15h - 59min - 59s - 125ms));
}

TEST(ConvertChrono, ConvertDurationToStringMaxValues) {
	using seconds_i8 = duration<int8_t, std::ratio<1>>;
	EXPECT_EQ("PT2M7S", Convert::ToString(seconds_i8(std::numeric_limits<int8_t>::max())));
	EXPECT_EQ("-PT2M8S", Convert::ToString(seconds_i8(std::numeric_limits<int8_t>::min())));

	using seconds_u8 = duration<uint8_t, std::ratio<1>>;
	EXPECT_EQ("PT4M15S", Convert::ToString(seconds_u8(std::numeric_limits<uint8_t>::max())));

	using minutes_i16 = duration<int16_t, std::ratio<60>>;
	EXPECT_EQ("P22DT18H7M", Convert::ToString(minutes_i16(std::numeric_limits<int16_t>::max())));
	EXPECT_EQ("-P22DT18H8M", Convert::ToString(minutes_i16(std::numeric_limits<int16_t>::min())));

	using days_i64 = duration<int64_t, std::ratio<86400>>;
	EXPECT_EQ("P9223372036854775807D", Convert::ToString(days_i64(std::numeric_limits<int64_t>::max())));
	EXPECT_EQ("-P9223372036854775808D", Convert::ToString(days_i64(std::numeric_limits<int64_t>::min())));

	using days_u64 = duration<uint64_t, std::ratio<86400>>;
	EXPECT_EQ("P18446744073709551615D", Convert::ToString(days_u64(std::numeric_limits<uint64_t>::max())));
}

//-----------------------------------------------------------------------------
// Test conversion from std::string_view to std::chrono::duration
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertStringToDuration) {
	EXPECT_EQ(5000ms, Convert::To<std::chrono::milliseconds>("PT5S"));
	EXPECT_EQ(6000000us, Convert::To<std::chrono::microseconds>("PT6S"));
	EXPECT_EQ(7000000000ns, Convert::To<std::chrono::nanoseconds>("PT7S"));

	EXPECT_EQ(25h + 1min + 1s, Convert::To<std::chrono::seconds>("P1DT1H1M1S"));
	EXPECT_EQ(24h * 25 + 55min + 41s, Convert::To<std::chrono::seconds>(u"P25DT55M41S"));
	EXPECT_EQ(10h + 20s, Convert::To<std::chrono::seconds>(U"PT10H20S"));
	EXPECT_EQ(48h + 44s, Convert::To<std::chrono::seconds>(L"P2DT44S"));
}

TEST(ConvertChrono, ConvertStringToDurationWithFollowingSpace) {
	EXPECT_EQ(35min + 25s, Convert::To<std::chrono::seconds>("PT35M25S Hello"));
	EXPECT_EQ(23h + 59min + 59s, Convert::To<std::chrono::seconds>(L"PT23H59M59S\nHello"));
}

TEST(ConvertChrono, ConvertStringToDurationWhenOnlySinglePart) {
	EXPECT_EQ(24h * 7, Convert::To<std::chrono::seconds>("P1W"));
	EXPECT_EQ(24h, Convert::To<std::chrono::seconds>("P1D"));
	EXPECT_EQ(1h, Convert::To<std::chrono::seconds>("PT1H"));
	EXPECT_EQ(1min, Convert::To<std::chrono::seconds>("PT1M"));
	EXPECT_EQ(1s, Convert::To<std::chrono::seconds>("PT1S"));
}

TEST(ConvertChrono, ConvertStringToDurationWhenZero) {
	EXPECT_EQ(0ms, Convert::To<std::chrono::milliseconds>("PT0.0S"));
	EXPECT_EQ(0s, Convert::To<std::chrono::seconds>("PT0S"));
	EXPECT_EQ(0s, Convert::To<std::chrono::seconds>("PT0M"));
	EXPECT_EQ(0s, Convert::To<std::chrono::seconds>("PT0H"));
	EXPECT_EQ(0s, Convert::To<std::chrono::seconds>("P0D"));
	EXPECT_EQ(0s, Convert::To<std::chrono::seconds>("P0W"));
}

TEST(ConvertChrono, ConvertStringToDurationWhithSecondsFractions) {
	// With comma as decimal separator
	EXPECT_EQ(500ms, Convert::To<std::chrono::milliseconds>("PT0,5S"));
	EXPECT_EQ(5s + 999ms, Convert::To<std::chrono::milliseconds>("PT5,999S"));
	// With dot as decimal separator
	EXPECT_EQ(123456us, Convert::To<std::chrono::microseconds>("PT0.123456S"));
	EXPECT_EQ(123456789ns, Convert::To<std::chrono::nanoseconds>("PT0.123456789S"));
}

TEST(ConvertChrono, ConvertStringToDurationWithRoundingFromMilli) {
	EXPECT_EQ(0s, Convert::To<std::chrono::seconds>("PT0.499S"));
	EXPECT_EQ(1s, Convert::To<std::chrono::seconds>("PT0.501S"));
}

TEST(ConvertChrono, ConvertStringToDurationWithRoundingFromMicro) {
	EXPECT_EQ(300ms, Convert::To<std::chrono::milliseconds>("PT0.300499S"));
	EXPECT_EQ(301ms, Convert::To<std::chrono::milliseconds>("PT0.300501S"));
}

TEST(ConvertChrono, ConvertStringToDurationWithRoundingFromNano) {
	EXPECT_EQ(750255us, Convert::To<std::chrono::microseconds>("PT0.750255405S"));
	EXPECT_EQ(750256us, Convert::To<std::chrono::microseconds>("PT0.750255505S"));
}

TEST(ConvertChrono, ConvertStringToDurationWhenNegative) {
	EXPECT_EQ(-1s, Convert::To<std::chrono::seconds>("-PT1S"));
	EXPECT_EQ(-5000ms, Convert::To<std::chrono::seconds>("-PT5S"));
	EXPECT_EQ(-24h - 25min, Convert::To<std::chrono::seconds>("-P1DT25M"));
	EXPECT_EQ(-24h * 120 - 3h - 3min - 3s, Convert::To<std::chrono::seconds>("-P120DT3H3M3S"));
}

TEST(ConvertChrono, ConvertStringToDurationWithPLusPrefix) {
	EXPECT_EQ(10s, Convert::To<std::chrono::milliseconds>("+PT10S"));
	EXPECT_EQ(100min + 15s, Convert::To<std::chrono::seconds>("+PT100M15S"));
}

TEST(ConvertChrono, ConvertStringToDurationMaxValues) {
	using seconds_i8 = duration<int8_t, std::ratio<1>>;
	EXPECT_TRUE(seconds_i8(std::numeric_limits<int8_t>::max()) == Convert::To<seconds_i8>("PT127S"));
	EXPECT_TRUE(seconds_i8(std::numeric_limits<int8_t>::min()) == Convert::To<seconds_i8>("-PT128S"));

	using seconds_u8 = duration<uint8_t, std::ratio<1>>;
	EXPECT_TRUE(seconds_u8(std::numeric_limits<uint8_t>::max()) == Convert::To<seconds_u8>("PT255S"));

	using minutes_i16 = duration<int16_t, std::ratio<60>>;
	EXPECT_TRUE(minutes_i16(std::numeric_limits<int16_t>::max()) == Convert::To<minutes_i16>("PT32767M"));
	EXPECT_TRUE(minutes_i16(std::numeric_limits<int16_t>::min()) == Convert::To<minutes_i16>("-PT32768M"));

	using days_i64 = duration<int64_t, std::ratio<86400>>;
	EXPECT_TRUE(days_i64(std::numeric_limits<int64_t>::max()) == Convert::To<days_i64>("P9223372036854775807D"));
	EXPECT_TRUE(days_i64(std::numeric_limits<int64_t>::min()) == Convert::To<days_i64>("-P9223372036854775808D"));

	using days_u64 = duration<uint64_t, std::ratio<86400>>;
	EXPECT_TRUE(days_u64(std::numeric_limits<uint64_t>::max()) == Convert::To<days_u64>("P18446744073709551615D"));
}

TEST(ConvertChrono, ConvertStringToDurationWithLessPrecision) {
	EXPECT_EQ(std::chrono::hours(10), Convert::To<std::chrono::hours>("PT600M"));
	EXPECT_EQ(std::chrono::hours(10), Convert::To<std::chrono::hours>("PT540M3600S"));
	EXPECT_EQ(std::chrono::hours(-10), Convert::To<std::chrono::hours>("-PT600M"));
	EXPECT_EQ(std::chrono::hours(-10), Convert::To<std::chrono::hours>("-PT540M3600S"));

	using days_i64 = duration<int64_t, std::ratio<86400>>;
	EXPECT_EQ(days_i64(10), Convert::To<days_i64>("PT240H"));
	EXPECT_EQ(days_i64(10), Convert::To<days_i64>("P5DT120H"));
	EXPECT_EQ(days_i64(-10), Convert::To<days_i64>("-PT240H"));
	EXPECT_EQ(days_i64(-10), Convert::To<days_i64>("-P5DT120H"));
}

TEST(ConvertChrono, ConvertStringToDurationShouldThrowExceptionWhenLossOfPrecision) {
	using days_i64 = duration<int64_t, std::ratio<86400>>;
	EXPECT_THROW(Convert::To<days_i64>("PT1H"), std::out_of_range);
	EXPECT_THROW(Convert::To<days_i64>("PT23H"), std::out_of_range);
	EXPECT_THROW(Convert::To<days_i64>("PT25H"), std::out_of_range);
	EXPECT_THROW(Convert::To<days_i64>("-PT15H"), std::out_of_range);
}

TEST(ConvertChrono, ConvertStringToDurationShouldThrowExceptionWhenMissedT) {
	EXPECT_THROW(Convert::To<std::chrono::seconds>("P0S"), std::invalid_argument);
	EXPECT_THROW(Convert::To<std::chrono::seconds>("P10H20M30S"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertStringToDurationShouldThrowExceptionWhenInvalidFormat) {
	EXPECT_THROW(Convert::To<std::chrono::seconds>("T0S"), std::invalid_argument);
	EXPECT_THROW(Convert::To<std::chrono::seconds>("P"), std::invalid_argument);
	EXPECT_THROW(Convert::To<std::chrono::seconds>("-P"), std::invalid_argument);
	EXPECT_THROW(Convert::To<std::chrono::seconds>("-PT"), std::invalid_argument);
	EXPECT_THROW(Convert::To<std::chrono::seconds>("PT-1S"), std::invalid_argument);
	EXPECT_THROW(Convert::To<std::chrono::seconds>("PTM1S"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertStringToDurationShouldThrowExceptionWhenContainsYearOrMonth) {
	EXPECT_THROW(Convert::To<std::chrono::seconds>("P5Y"), std::invalid_argument);
	EXPECT_THROW(Convert::To<std::chrono::seconds>("P5YT20D"), std::invalid_argument);
	EXPECT_THROW(Convert::To<std::chrono::seconds>("P10MT20M"), std::invalid_argument);
	EXPECT_THROW(Convert::To<std::chrono::seconds>("2003-02-15T00:00:00Z/P2M"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertStringToDurationShouldThrowExceptionWhenContainsBaseUtc) {
	// Accordingly to ISO standard, duration may preseed UTC time for correctly calculate number of days in month, but this is not supported now
	EXPECT_THROW(Convert::To<std::chrono::seconds>("2003-02-15T00:00:00Z/P2M"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertStringToDurationShouldThrowExceptionWhenNonSecondsFractions) {
	// Accordingly to ISO standard, smallest value may have a decimal fraction, but BitSerializer supports only seconds fractions
	EXPECT_THROW(Convert::To<std::chrono::seconds>("P0.5D"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertStringToDurationShouldThrowExceptionWhenEmpty) {
	EXPECT_THROW(Convert::To<std::chrono::seconds>(""), std::invalid_argument);
}

TEST(ConvertChrono, ConvertStringToDurationShouldThrowExceptionWhenOverflow) {
	using ms_i16 = duration<int16_t, std::milli>;
	EXPECT_THROW(Convert::To<ms_i16>("PT33S"), std::out_of_range);
	EXPECT_THROW(Convert::To<ms_i16>("-PT33S"), std::out_of_range);
	EXPECT_THROW(Convert::To<ms_i16>("PT32.768S"), std::out_of_range);
	EXPECT_THROW(Convert::To<ms_i16>("-PT32.769S"), std::out_of_range);

	using minutes_i8 = duration<int8_t, std::ratio<60>>;
	EXPECT_THROW(Convert::To<minutes_i8>("PT128M"), std::out_of_range);
	EXPECT_THROW(Convert::To<minutes_i8>("-PT129M"), std::out_of_range);

	using minutes_u8 = duration<uint8_t, std::ratio<60>>;
	EXPECT_THROW(Convert::To<minutes_u8>("PT256M"), std::out_of_range);
	EXPECT_THROW(Convert::To<minutes_u8>("-PT10M"), std::out_of_range);

	using days_i64 = duration<int64_t, std::ratio<86400>>;
	EXPECT_THROW(Convert::To<days_i64>("P9223372036854775808D"), std::out_of_range);
	EXPECT_THROW(Convert::To<days_i64>("-P9223372036854775809D"), std::out_of_range);

	using days_u64 = duration<uint64_t, std::ratio<86400>>;
	EXPECT_THROW(Convert::To<days_u64>("P18446744073709551616D"), std::out_of_range);
}

//-----------------------------------------------------------------------------
// Test conversion from time_t to std::string
//-----------------------------------------------------------------------------

#if INTPTR_MAX == INT64_MAX
constexpr time_t ctime0000_01_01T00_00_00 = -62167219200;
constexpr time_t ctime1583_01_01T00_00_00 = -12212553600;
#endif
constexpr time_t ctime1901_12_13T20_45_52 = -2147483648;	// Min for int32_t
constexpr time_t ctime1969_08_01T00_00_00 = -13219200;
constexpr time_t ctime1969_12_31T00_59_00 = -82860;
constexpr time_t ctime1969_12_31T06_53_00 = -61620;
constexpr time_t ctime1969_12_31T23_59_59 = -1;
constexpr time_t ctime1970_01_01T00_00_00 = 0;
constexpr time_t ctime1970_01_01T00_00_01 = 1;
constexpr time_t ctime2038_01_19T03_14_07 = 2147483647;		// Max for int32_t
#if INTPTR_MAX == INT64_MAX
constexpr time_t ctime9999_12_31T23_59_59 = 253402300799;
#endif

//-----------------------------------------------------------------------------

TEST(ConvertChrono, ConvertCTimeSinceEpochToUtcString) {
	EXPECT_EQ("1970-01-01T00:00:00Z", Convert::To<std::string>(UnixTime(ctime1970_01_01T00_00_00)));
	EXPECT_EQ(u"1970-01-01T00:00:01Z", Convert::To<std::u16string>(UnixTime(ctime1970_01_01T00_00_01)));
	EXPECT_EQ(U"2038-01-19T03:14:07Z", Convert::To<std::u32string>(UnixTime(ctime2038_01_19T03_14_07)));
#if INTPTR_MAX == INT64_MAX
	EXPECT_EQ(L"9999-12-31T23:59:59Z", Convert::To<std::wstring>(UnixTime(ctime9999_12_31T23_59_59)));
#endif
}

TEST(ConvertChrono, ConvertCTimeBeforeEpochToUtcString) {
	EXPECT_EQ("1969-12-31T23:59:59Z", Convert::ToString(UnixTime(ctime1969_12_31T23_59_59)));
	EXPECT_EQ("1969-12-31T06:53:00Z", Convert::ToString(UnixTime(ctime1969_12_31T06_53_00)));
	EXPECT_EQ("1969-12-31T00:59:00Z", Convert::ToString(UnixTime(ctime1969_12_31T00_59_00)));
	EXPECT_EQ("1969-08-01T00:00:00Z", Convert::ToString(UnixTime(ctime1969_08_01T00_00_00)));
	EXPECT_EQ("1901-12-13T20:45:52Z", Convert::ToString(UnixTime(ctime1901_12_13T20_45_52)));
#if INTPTR_MAX == INT64_MAX
	EXPECT_EQ("1583-01-01T00:00:00Z", Convert::ToString(UnixTime(ctime1583_01_01T00_00_00)));
	EXPECT_EQ("0000-01-01T00:00:00Z", Convert::ToString(UnixTime(ctime0000_01_01T00_00_00)));
#endif
}

//-----------------------------------------------------------------------------
// Test conversion from std::string to time_t
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertUtcSinceEpochStringToCTime) {
	EXPECT_EQ(ctime1970_01_01T00_00_00, Convert::To<UnixTime>("1970-01-01T00:00:00Z"));
	EXPECT_EQ(ctime1970_01_01T00_00_01, Convert::To<UnixTime>(u"1970-01-01T00:00:01Z"));
	EXPECT_EQ(ctime2038_01_19T03_14_07, Convert::To<UnixTime>(U"2038-01-19T03:14:07Z"));
#if INTPTR_MAX == INT64_MAX
	EXPECT_EQ(ctime9999_12_31T23_59_59, Convert::To<UnixTime>(L"9999-12-31T23:59:59Z"));
#endif
}

TEST(ConvertChrono, ConvertUtcBeforeEpochStringToCTime) {
	EXPECT_EQ(ctime1969_12_31T23_59_59, Convert::To<UnixTime>("1969-12-31T23:59:59Z"));
	EXPECT_EQ(ctime1969_12_31T06_53_00, Convert::To<UnixTime>("1969-12-31T06:53:00Z"));
	EXPECT_EQ(ctime1969_12_31T00_59_00, Convert::To<UnixTime>("1969-12-31T00:59:00Z"));
	EXPECT_EQ(ctime1969_08_01T00_00_00, Convert::To<UnixTime>(u"1969-08-01T00:00:00Z"));
#if INTPTR_MAX == INT64_MAX
	EXPECT_EQ(ctime1583_01_01T00_00_00, Convert::To<UnixTime>(U"1583-01-01T00:00:00Z"));
	EXPECT_EQ(ctime0000_01_01T00_00_00, Convert::To<UnixTime>(L"0000-01-01T00:00:00Z"));
#endif
}

TEST(ConvertChrono, ConvertCTimeShouldThrowExceptionWhenEmpty) {
	EXPECT_THROW(Convert::To<UnixTime>(""), std::invalid_argument);
}

TEST(ConvertChrono, ConvertCTimeShouldThrowExceptionWhenOverflow) {
	EXPECT_THROW(Convert::To<UnixTime>("+292277026596-12-04T15:30:08Z"), std::out_of_range);
	EXPECT_THROW(Convert::To<UnixTime>("-292277022657-01-27T08:29:51Z"), std::out_of_range);
}

//-----------------------------------------------------------------------------
// Test conversion from `tm` structure to std::string
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertTmToUtcString)
{
	std::tm tm{};
	tm.tm_year = 2023 - 1900;
	tm.tm_mon = 9 - 1;
	tm.tm_mday = 5;
	tm.tm_hour = 22;
	tm.tm_min = 59;
	tm.tm_sec = 35;
	EXPECT_EQ("2023-09-05T22:59:35Z", Convert::ToString(tm));
}

//-----------------------------------------------------------------------------
// Test conversion from std::string to `tm` structure
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertUtcStringToTm)
{
	const auto tm = Convert::To<std::tm>("2023-09-05T22:59:35Z");
	EXPECT_EQ(2023 - 1900, tm.tm_year);
	EXPECT_EQ(9 - 1, tm.tm_mon);
	EXPECT_EQ(5, tm.tm_mday);
	EXPECT_EQ(22, tm.tm_hour);
	EXPECT_EQ(59, tm.tm_min);
	EXPECT_EQ(35, tm.tm_sec);
}

TEST(ConvertChrono, ConvertUtcStringToTmShouldThrowExceptionWhenInvalidDate) {
	EXPECT_THROW(Convert::To<tm>("2015-00-04T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<tm>("2015-00-00T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<tm>("2015-31-04T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<tm>("2015-12-32T00:00:00Z"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringToTmShouldThrowExceptionWhenInvalidTime) {
	EXPECT_THROW(Convert::To<tm>("2015-12-04T24:30:07Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<tm>("2015-12-04T15:60:07Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<tm>("2015-12-04T15:30:60Z"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringToTmShouldThrowExceptionWhenOverflow) {
	EXPECT_THROW(Convert::To<tm>("+2147485548-12-04T15:30:07Z"), std::out_of_range);
	EXPECT_THROW(Convert::To<tm>("-2147481749-12-04T15:30:07Z"), std::out_of_range);
}

TEST(ConvertChrono, ConvertUtcStringToTmShouldAcceptBoundaryYears) {
	const auto maxTm = Convert::To<tm>("+2147485547-12-04T15:30:07Z");
	EXPECT_EQ((std::numeric_limits<int>::max)(), maxTm.tm_year);

	const auto minTm = Convert::To<tm>("-2147481748-12-04T15:30:07Z");
	EXPECT_EQ((std::numeric_limits<int>::min)(), minTm.tm_year);
}

#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
//-----------------------------------------------------------------------------
// Test conversion for C++20 calendar types
//-----------------------------------------------------------------------------
static_assert(Convert::IsConvertible<year_month_day, std::string>());
static_assert(Convert::IsConvertible<std::string, year_month_day>());
static_assert(Convert::IsConvertible<hh_mm_ss<milliseconds>, std::string>());
static_assert(Convert::IsConvertible<std::string, hh_mm_ss<milliseconds>>());

TEST(ConvertChrono, ConvertYearMonthDayToString)
{
	const auto date = year_month_day{year{2024}, month{2}, day{29}};
	EXPECT_EQ("2024-02-29", Convert::ToString(date));
	EXPECT_EQ(L"2024-02-29", Convert::To<std::wstring>(date));
	EXPECT_EQ(u"2024-02-29", Convert::To<std::u16string>(date));
	EXPECT_EQ(U"2024-02-29", Convert::To<std::u32string>(date));
	EXPECT_EQ("1970-01-01", Convert::ToString(year_month_day{year{1970}, month{1}, day{1}}));
	EXPECT_EQ("+10000-01-01", Convert::ToString(year_month_day{year{10000}, month{1}, day{1}}));
	EXPECT_EQ("-0001-12-31", Convert::ToString(year_month_day{year{-1}, month{12}, day{31}}));
}

TEST(ConvertChrono, ConvertStringToYearMonthDay)
{
	const auto expected = year_month_day{year{2024}, month{2}, day{29}};
	EXPECT_TRUE(expected == Convert::To<year_month_day>("2024-02-29"));
	EXPECT_TRUE(expected == Convert::To<year_month_day>(L"2024-02-29"));
	EXPECT_TRUE(expected == Convert::To<year_month_day>(u"2024-02-29"));
	EXPECT_TRUE(expected == Convert::To<year_month_day>(U"2024-02-29"));
	EXPECT_TRUE(expected == Convert::To<year_month_day>(u8"2024-02-29"));

	const auto minYear = year_month_day{year{-1}, month{12}, day{31}};
	EXPECT_TRUE(minYear == Convert::To<year_month_day>("-0001-12-31"));

	const auto extendedYear = year_month_day{year{10000}, month{1}, day{1}};
	EXPECT_TRUE(extendedYear == Convert::To<year_month_day>("+10000-01-01"));
}

TEST(ConvertChrono, ConvertStringToYearMonthDayShouldThrowExceptionWhenInvalid)
{
	EXPECT_THROW(Convert::To<year_month_day>("2024-02-30"), std::invalid_argument);
	EXPECT_THROW(Convert::To<year_month_day>("2024-13-01"), std::invalid_argument);
	EXPECT_THROW(Convert::To<year_month_day>("2024-00-01"), std::invalid_argument);
	EXPECT_THROW(Convert::To<year_month_day>("2024-01-00"), std::invalid_argument);
	EXPECT_THROW(Convert::To<year_month_day>("2024-01-32"), std::invalid_argument);
	EXPECT_THROW(Convert::To<year_month_day>("2024-01"), std::invalid_argument);
	EXPECT_THROW(Convert::To<year_month_day>("2024-01-01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<year_month_day>("+32768-01-01"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertYearMonthDayToStringShouldThrowExceptionWhenInvalid)
{
	EXPECT_THROW(Convert::ToString(year_month_day{year{2023}, month{2}, day{29}}), std::invalid_argument);
	EXPECT_THROW(Convert::ToString(year_month_day{}), std::invalid_argument);
}

TEST(ConvertChrono, ConvertHhMmSsToString)
{
	using Ms = milliseconds;
	EXPECT_EQ("12:34:56", Convert::ToString(hh_mm_ss<seconds>{hours{12} + minutes{34} + seconds{56}}));
	EXPECT_EQ("12:34:56.789", Convert::ToString(hh_mm_ss<Ms>{hours{12} + minutes{34} + seconds{56} + milliseconds{789}}));
	EXPECT_EQ("12:34:56.78", Convert::ToString(hh_mm_ss<Ms>{hours{12} + minutes{34} + seconds{56} + milliseconds{780}}));
	EXPECT_EQ("12:34:56", Convert::ToString(hh_mm_ss<Ms>{hours{12} + minutes{34} + seconds{56}}));
	EXPECT_EQ("00:00:00", Convert::ToString(hh_mm_ss<Ms>{Ms{0}}));
	EXPECT_EQ("-12:34:56.789", Convert::ToString(hh_mm_ss<Ms>{Ms{-45296789}}));
	EXPECT_EQ("36:00:00", Convert::ToString(hh_mm_ss<seconds>{hours{36}}));
	EXPECT_EQ(L"12:34:56.789", Convert::To<std::wstring>(hh_mm_ss<Ms>{hours{12} + minutes{34} + seconds{56} + milliseconds{789}}));
}

TEST(ConvertChrono, ConvertStringToHhMmSs)
{
	using Ms = milliseconds;
	const auto expected = hh_mm_ss<Ms>{hours{12} + minutes{34} + seconds{56} + milliseconds{789}};
	EXPECT_EQ(expected.to_duration().count(), Convert::To<hh_mm_ss<Ms>>("12:34:56.789").to_duration().count());
	EXPECT_EQ(expected.to_duration().count(), Convert::To<hh_mm_ss<Ms>>("12:34:56,789").to_duration().count());
	EXPECT_EQ(expected.to_duration().count(), Convert::To<hh_mm_ss<Ms>>(L"12:34:56.789").to_duration().count());
	EXPECT_EQ(expected.to_duration().count(), Convert::To<hh_mm_ss<Ms>>(u8"12:34:56.789").to_duration().count());
	EXPECT_EQ(700, Convert::To<hh_mm_ss<Ms>>("00:00:00.7").to_duration().count());
	EXPECT_EQ(780, Convert::To<hh_mm_ss<Ms>>("00:00:00.78").to_duration().count());
	EXPECT_EQ(0, Convert::To<hh_mm_ss<Ms>>("00:00:00").to_duration().count());
	EXPECT_EQ(-45296789, Convert::To<hh_mm_ss<Ms>>("-12:34:56.789").to_duration().count());
	EXPECT_EQ(129600, Convert::To<hh_mm_ss<seconds>>("36:00:00").to_duration().count());
}

TEST(ConvertChrono, ConvertStringToHhMmSsShouldThrowExceptionWhenInvalid)
{
	using Ms = milliseconds;
	EXPECT_THROW(Convert::To<hh_mm_ss<Ms>>("12:60:00"), std::invalid_argument);
	EXPECT_THROW(Convert::To<hh_mm_ss<Ms>>("12:34:60"), std::invalid_argument);
	EXPECT_THROW(Convert::To<hh_mm_ss<Ms>>("12-34-56"), std::invalid_argument);
	EXPECT_THROW(Convert::To<hh_mm_ss<Ms>>("12:34:56."), std::invalid_argument);
	EXPECT_THROW(Convert::To<hh_mm_ss<Ms>>("12:34:56Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<hh_mm_ss<seconds>>("12:34:56.789"), std::invalid_argument);
	EXPECT_THROW(Convert::To<hh_mm_ss<hours>>("12:34:56"), std::out_of_range);
	EXPECT_THROW(Convert::To<hh_mm_ss<Ms>>("9223372036854775808:00:00"), std::out_of_range);
}
#endif
