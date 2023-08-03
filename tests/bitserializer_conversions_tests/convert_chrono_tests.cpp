/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"

using namespace BitSerializer;
using namespace std::chrono;

//-----------------------------------------------------------------------------
// Test data
//-----------------------------------------------------------------------------
using TimePointMs = time_point<system_clock, milliseconds>;
using TimePointNs = time_point<system_clock, nanoseconds>;

// Year before introduction of the Gregorian calendar are not automatically allowed by the standard
constexpr auto tp0000_01_01T00_00_00 = TimePointMs(seconds(-62167219200));
// First year that allowed by the ISO 8601 standard
constexpr auto tp1583_01_01T00_00_00 = TimePointMs(seconds(-12212553600));
// Min timepoint for nanoseconds as duration
constexpr auto tp1677_09_21T00_12_44 = std::chrono::time_point_cast<std::chrono::seconds>(TimePointNs::min());
constexpr auto tp1872_01_01T00_00_00 = TimePointMs(seconds(-3092601600));
constexpr auto tp1968_12_31T23_59_59 = TimePointMs(seconds(-31536001));
// Unix time EPOCH
constexpr auto tp1970_01_01T00_00_00 = TimePointMs(seconds(0));
constexpr auto tp2044_01_01T00_00_00 = TimePointMs(seconds(2335219200));
// Max timepoint for nanoseconds as duration
constexpr auto tp2262_04_11T23_47_16 = std::chrono::time_point_cast<std::chrono::seconds>(TimePointNs::max());
constexpr auto tp9999_12_31T23_59_59 = TimePointMs(seconds(253402300799));

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
}

TEST(ConvertChrono, ConvertUtcStringWithMsToTimePoint) {
	EXPECT_EQ(tp1677_09_21T00_12_44 + 999ms, Convert::To<TimePointMs>("1677-09-21T00:12:44.999Z"));
	EXPECT_EQ(tp1872_01_01T00_00_00 + 1ms, Convert::To<TimePointMs>("1872-01-01T00:00:00.001Z"));
	EXPECT_EQ(tp1968_12_31T23_59_59 + 567ms, Convert::To<TimePointMs>("1968-12-31T23:59:59.567Z"));
	EXPECT_EQ(tp1970_01_01T00_00_00 + 25ms, Convert::To<TimePointMs>("1970-01-01T00:00:00.025Z"));
	EXPECT_EQ(tp2044_01_01T00_00_00 + 1ms, Convert::To<TimePointMs>(U"2044-01-01T00:00:00.001Z"));
	EXPECT_EQ(tp2262_04_11T23_47_16 + 999ms, Convert::To<TimePointMs>(L"2262-04-11T23:47:16.999Z"));
	EXPECT_EQ(tp9999_12_31T23_59_59 + 999ms, Convert::To<TimePointMs>(L"9999-12-31T23:59:59.999Z"));
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenInvalidDelimiters) {
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01 00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970/01/01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00.00.00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:00:00:000Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00-00-00"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenExtraMinus) {
	EXPECT_THROW(Convert::To<TimePointMs>("-1970-01-01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970--01-01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01--01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T-00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:-00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:00:-00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:00:00.-000Z"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenInvalidDate) {
	EXPECT_THROW(Convert::To<TimePointMs>("1970-13-01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-32T00:00:00Z"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenInvalidTime) {
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T25:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:60:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:00:60Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<TimePointMs>("1970-01-01T00:00:00.1000Z"), std::invalid_argument);
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
	EXPECT_THROW(Convert::To<TimePointNs>("1677-09-21T00:12:43Z"), std::out_of_range);
	EXPECT_THROW(Convert::To<TimePointNs>("2262-04-11T23:47:17Z"), std::out_of_range);
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

TEST(ConvertChrono, ConvertDurationToStringWithDiscardMs) {
	EXPECT_EQ("PT1S", Convert::ToString(1s + 100ms));
	EXPECT_EQ("PT1M", Convert::ToString(1min + 999ms));
}

TEST(ConvertChrono, ConvertDurationToStringWhenNegative) {
	EXPECT_EQ("-PT1S", Convert::ToString(-1s));
	EXPECT_EQ("-P10DT25M", Convert::ToString(-24h * 10 - 25min));
	EXPECT_EQ("-P120DT3H3M3S", Convert::ToString(-24h * 120 - 3h - 3min - 3s));
}

//-----------------------------------------------------------------------------
// Test conversion from std::string_view to std::chrono::duration
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertStringToDuration) {
	EXPECT_EQ(25h + 1min + 1s, Convert::To<std::chrono::seconds>("P1DT1H1M1S"));
	EXPECT_EQ(24h * 25 + 55min + 41s, Convert::To<std::chrono::seconds>(u"P25DT55M41S"));
	EXPECT_EQ(10h + 20s, Convert::To<std::chrono::seconds>(U"PT10H20S"));
	EXPECT_EQ(48h + 44s, Convert::To<std::chrono::seconds>(L"P2DT44S"));
}

TEST(ConvertChrono, ConvertStringToDurationWhenOnlySinglePart) {
	EXPECT_EQ(24h * 7, Convert::To<std::chrono::seconds>("P1W"));
	EXPECT_EQ(24h, Convert::To<std::chrono::seconds>("P1D"));
	EXPECT_EQ(1h, Convert::To<std::chrono::seconds>("PT1H"));
	EXPECT_EQ(1min, Convert::To<std::chrono::seconds>("PT1M"));
	EXPECT_EQ(1s, Convert::To<std::chrono::seconds>("PT1S"));
}

TEST(ConvertChrono, ConvertStringToDurationWhenZero) {
	EXPECT_EQ(0s, Convert::To<std::chrono::seconds>("PT0S"));
	EXPECT_EQ(0s, Convert::To<std::chrono::seconds>("PT0M"));
	EXPECT_EQ(0s, Convert::To<std::chrono::seconds>("PT0H"));
	EXPECT_EQ(0s, Convert::To<std::chrono::seconds>("P0D"));
	EXPECT_EQ(0s, Convert::To<std::chrono::seconds>("P0W"));
}

TEST(ConvertChrono, ConvertStringToDurationWhenNegative) {
	EXPECT_EQ(-1s, Convert::To<std::chrono::seconds>("-PT1S"));
	EXPECT_EQ(-24h * 10 - 25min, Convert::To<std::chrono::seconds>("-P10DT25M"));
	EXPECT_EQ(-24h * 120 - 3h - 3min - 3s, Convert::To<std::chrono::seconds>("-P120DT3H3M3S"));
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

TEST(ConvertChrono, ConvertStringToDurationShouldThrowExceptionWhenContainsDecimalFraction) {
	// Accordingly to ISO standard, smallest value may have a decimal fraction, but this is not supported now
	EXPECT_THROW(Convert::To<std::chrono::seconds>("P0.5D"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertStringToDurationShouldThrowExceptionWhenEmpty) {
	EXPECT_THROW(Convert::To<std::chrono::seconds>(""), std::invalid_argument);
}

//-----------------------------------------------------------------------------
// Test conversion from time_t to std::string
//-----------------------------------------------------------------------------

constexpr time_t ctime0000_01_01T00_00_00 = -62167219200;
constexpr time_t ctime1583_01_01T00_00_00 = -12212553600;
constexpr time_t ctime1969_12_31T00_59_00 = -13219141;
constexpr time_t ctime1969_08_01T00_00_00 = -13219200;
constexpr time_t ctime1969_12_31T06_53_00 = -61620;
constexpr time_t ctime1969_12_31T23_59_59 = -1;
constexpr time_t ctime1970_01_01T00_00_00 = 0;
constexpr time_t ctime1970_01_01T00_00_01 = 1;
constexpr time_t ctime2044_01_01T00_00_00 = 2335219200;
constexpr time_t ctime9999_12_31T23_59_59 = 253402300799;

//-----------------------------------------------------------------------------

TEST(ConvertChrono, ConvertCTimeSinceEpochToUtcString) {
	EXPECT_EQ("1970-01-01T00:00:00Z", Convert::To<std::string>(CRawTime(ctime1970_01_01T00_00_00)));
	EXPECT_EQ(u"1970-01-01T00:00:01Z", Convert::To<std::u16string>(CRawTime(ctime1970_01_01T00_00_01)));
	EXPECT_EQ(U"2044-01-01T00:00:00Z", Convert::To<std::u32string>(CRawTime(ctime2044_01_01T00_00_00)));
	EXPECT_EQ(L"9999-12-31T23:59:59Z", Convert::To<std::wstring>(CRawTime(ctime9999_12_31T23_59_59)));
}

TEST(ConvertChrono, ConvertCTimeBeforeEpochToUtcString) {
	EXPECT_EQ("1969-12-31T23:59:59Z", Convert::ToString(CRawTime(ctime1969_12_31T23_59_59)));
	EXPECT_EQ("1969-12-31T06:53:00Z", Convert::ToString(CRawTime(ctime1969_12_31T06_53_00)));
	EXPECT_EQ("1969-08-01T00:00:00Z", Convert::ToString(CRawTime(ctime1969_08_01T00_00_00)));
	EXPECT_EQ("1969-08-01T00:00:59Z", Convert::ToString(CRawTime(ctime1969_12_31T00_59_00)));
	EXPECT_EQ("1583-01-01T00:00:00Z", Convert::ToString(CRawTime(ctime1583_01_01T00_00_00)));
	EXPECT_EQ("0000-01-01T00:00:00Z", Convert::ToString(CRawTime(ctime0000_01_01T00_00_00)));
}

//-----------------------------------------------------------------------------
// Test conversion from std::string to time_t
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertUtcSinceEpochStringToCTime) {
	EXPECT_EQ(ctime1970_01_01T00_00_00, Convert::To<CRawTime>("1970-01-01T00:00:00Z"));
	EXPECT_EQ(ctime1970_01_01T00_00_01, Convert::To<CRawTime>(u"1970-01-01T00:00:01Z"));
	EXPECT_EQ(ctime2044_01_01T00_00_00, Convert::To<CRawTime>(U"2044-01-01T00:00:00Z"));
	EXPECT_EQ(ctime9999_12_31T23_59_59, Convert::To<CRawTime>(L"9999-12-31T23:59:59Z"));
}

TEST(ConvertChrono, ConvertUtcBeforeEpochStringToCTime) {
	EXPECT_EQ(ctime1969_12_31T23_59_59, Convert::To<CRawTime>("1969-12-31T23:59:59Z"));
	EXPECT_EQ(ctime1969_12_31T06_53_00, Convert::To<CRawTime>("1969-12-31T06:53:00Z"));
	EXPECT_EQ(ctime1969_08_01T00_00_00, Convert::To<CRawTime>("1969-08-01T00:00:00Z"));
	EXPECT_EQ(ctime1969_12_31T00_59_00, Convert::To<CRawTime>(u"1969-08-01T00:00:59Z"));
	EXPECT_EQ(ctime1583_01_01T00_00_00, Convert::To<CRawTime>(U"1583-01-01T00:00:00Z"));
	EXPECT_EQ(ctime0000_01_01T00_00_00, Convert::To<CRawTime>(L"0000-01-01T00:00:00Z"));
}

TEST(ConvertChrono, ConvertCTimeShouldThrowExceptionWhenEmpty) {
	EXPECT_THROW(Convert::To<CRawTime>(""), std::invalid_argument);
}
