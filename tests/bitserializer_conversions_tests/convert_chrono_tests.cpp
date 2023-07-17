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

constexpr auto tp1678_12_31T23_59_59 = TimePointMs(seconds(-9183024001));
constexpr auto tp1872_01_01T00_00_00 = TimePointMs(seconds(-3092601600));
constexpr auto tp1968_02_29T12_35_45 = TimePointMs(seconds(-58015455));
constexpr auto tp1970_01_01T00_00_00 = TimePointMs(seconds(0));
constexpr auto tp2020_02_29T14_48_32 = TimePointMs(seconds(1582987712));
constexpr auto tp2044_01_01T00_00_00 = TimePointMs(seconds(2335219200));
constexpr auto tp2261_12_31T23_59_59 = TimePointMs(seconds(9214646399));

//-----------------------------------------------------------------------------
// Test conversion from std::chrono::system_clock::time_point to std::string
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertSystemClockTimePointToUtcString) {
	EXPECT_EQ("1678-12-31T23:59:59Z", Convert::ToString(tp1678_12_31T23_59_59));
	EXPECT_EQ("1872-01-01T00:00:00Z", Convert::ToString(tp1872_01_01T00_00_00));
	EXPECT_EQ("1968-02-29T12:35:45Z", Convert::ToString(tp1968_02_29T12_35_45));
	EXPECT_EQ("1970-01-01T00:00:00Z", Convert::ToString(tp1970_01_01T00_00_00));
	EXPECT_EQ(u"2020-02-29T14:48:32Z", Convert::To<std::u16string>(tp2020_02_29T14_48_32));
	EXPECT_EQ(U"2044-01-01T00:00:00Z", Convert::To<std::u32string>(tp2044_01_01T00_00_00));
	EXPECT_EQ(L"2261-12-31T23:59:59Z", Convert::To<std::wstring>(tp2261_12_31T23_59_59));
}

TEST(ConvertChrono, ConvertSystemClockTimePointWithMsToUtcString) {
	EXPECT_EQ("1678-12-31T23:59:59.999Z", Convert::ToString(tp1678_12_31T23_59_59 + 999ms));
	EXPECT_EQ("1872-01-01T00:00:00.001Z", Convert::ToString(tp1872_01_01T00_00_00 + 1ms));
	EXPECT_EQ("1968-02-29T12:35:45.567Z", Convert::ToString(tp1968_02_29T12_35_45 + 567ms));
	EXPECT_EQ("1970-01-01T00:00:00.025Z", Convert::ToString(tp1970_01_01T00_00_00 + 25ms));
	EXPECT_EQ(u"2020-02-29T14:48:32.325Z", Convert::To<std::u16string>(tp2020_02_29T14_48_32 + 325ms));
	EXPECT_EQ(U"2044-01-01T00:00:00.001Z", Convert::To<std::u32string>(tp2044_01_01T00_00_00 + 1ms));
	EXPECT_EQ(L"2261-12-31T23:59:59.999Z", Convert::To<std::wstring>(tp2261_12_31T23_59_59 + 999ms));
}

//-----------------------------------------------------------------------------
// Test conversion from std::string_view to std::chrono::system_clock::time_point
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertUtcStringToSystemClockTimePoint) {
	EXPECT_EQ(tp1678_12_31T23_59_59, Convert::To<TimePointMs>("1678-12-31T23:59:59Z"));
	EXPECT_EQ(tp1872_01_01T00_00_00, Convert::To<TimePointMs>("1872-01-01T00:00:00Z"));
	EXPECT_EQ(tp1968_02_29T12_35_45, Convert::To<TimePointMs>("1968-02-29T12:35:45Z"));
	EXPECT_EQ(tp1970_01_01T00_00_00, Convert::To<TimePointMs>("1970-01-01T00:00:00Z"));
	EXPECT_EQ(tp2020_02_29T14_48_32, Convert::To<TimePointMs>(u"2020-02-29T14:48:32Z"));
	EXPECT_EQ(tp2044_01_01T00_00_00, Convert::To<TimePointMs>(U"2044-01-01T00:00:00Z"));
	EXPECT_EQ(tp2261_12_31T23_59_59, Convert::To<TimePointMs>(L"2261-12-31T23:59:59Z"));
}

TEST(ConvertChrono, ConvertUtcStringWithMsToSystemClockTimePoint) {
	EXPECT_EQ(tp1678_12_31T23_59_59 + 999ms, Convert::To<TimePointMs>("1678-12-31T23:59:59.999Z"));
	EXPECT_EQ(tp1872_01_01T00_00_00 + 1ms, Convert::To<TimePointMs>("1872-01-01T00:00:00.001Z"));
	EXPECT_EQ(tp1968_02_29T12_35_45 + 567ms, Convert::To<TimePointMs>("1968-02-29T12:35:45.567Z"));
	EXPECT_EQ(tp1970_01_01T00_00_00 + 25ms, Convert::To<TimePointMs>("1970-01-01T00:00:00.025Z"));
	EXPECT_EQ(tp2020_02_29T14_48_32 + 325ms, Convert::To<TimePointMs>(u"2020-02-29T14:48:32.325Z"));
	EXPECT_EQ(tp2044_01_01T00_00_00 + 1ms, Convert::To<TimePointMs>(U"2044-01-01T00:00:00.001Z"));
	EXPECT_EQ(tp2261_12_31T23_59_59 + 999ms, Convert::To<TimePointMs>(L"2261-12-31T23:59:59.999Z"));
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
