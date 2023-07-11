/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"

using namespace BitSerializer;
using namespace std::chrono;

const auto tp1970_01_01T00_00_00 = system_clock::from_time_t(0);
const auto tp1991_07_10T23_16_53 = system_clock::from_time_t(679187813);
const auto tp2044_12_31T00_00_00 = system_clock::from_time_t(2366755200);
const auto tp3000_05_15T10_25_15 = system_clock::from_time_t(32515295115);

//-----------------------------------------------------------------------------
// Test conversion from std::chrono::system_clock::time_point to std::string
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertSystemClockTimePointToUtcString) {
	EXPECT_EQ("1970-01-01T00:00:00Z", Convert::ToString(tp1970_01_01T00_00_00));
	EXPECT_EQ(u"1991-07-10T23:16:53Z", Convert::To<std::u16string>(tp1991_07_10T23_16_53));
	EXPECT_EQ(U"2044-12-31T00:00:00Z", Convert::To<std::u32string>(tp2044_12_31T00_00_00));
	EXPECT_EQ(L"3000-05-15T10:25:15Z", Convert::To<std::wstring>(tp3000_05_15T10_25_15));
}

TEST(ConvertChrono, ConvertSystemClockTimePointWithMsToUtcString) {
	EXPECT_EQ("1970-01-01T00:00:00.567Z", Convert::ToString(tp1970_01_01T00_00_00 + 567ms));
	EXPECT_EQ(u"1991-07-10T23:16:53.001Z", Convert::To<std::u16string>(tp1991_07_10T23_16_53 + 1ms));
	EXPECT_EQ(U"2044-12-31T00:00:00.999Z", Convert::To<std::u32string>(tp2044_12_31T00_00_00 + 999ms));
	EXPECT_EQ(L"3000-05-15T10:25:15.555Z", Convert::To<std::wstring>(tp3000_05_15T10_25_15 + 555ms));
}

TEST(ConvertChrono, ConvertSystemClockTimePointShouldThrowExceptionWhenNegativeTime) {
	EXPECT_THROW(Convert::ToString(system_clock::from_time_t(-4957113600)), std::out_of_range);
}


//-----------------------------------------------------------------------------
// Test conversion from std::string_view to std::chrono::system_clock::time_point
//-----------------------------------------------------------------------------
TEST(ConvertChrono, ConvertUtcStringToSystemClockTimePoint) {
	EXPECT_EQ(tp1970_01_01T00_00_00, Convert::To<system_clock::time_point>("1970-01-01T00:00:00Z"));
	EXPECT_EQ(tp1991_07_10T23_16_53, Convert::To<system_clock::time_point>("1991-07-10T23:16:53Z"));
	EXPECT_EQ(tp2044_12_31T00_00_00, Convert::To<system_clock::time_point>("2044-12-31T00:00:00Z"));
	EXPECT_EQ(tp3000_05_15T10_25_15, Convert::To<system_clock::time_point>("3000-05-15T10:25:15Z"));
}

TEST(ConvertChrono, ConvertUtcStringWithMsToSystemClockTimePoint) {
	EXPECT_EQ(tp1970_01_01T00_00_00, Convert::To<system_clock::time_point>("1970-01-01T00:00:00.000Z"));
	EXPECT_EQ(tp1991_07_10T23_16_53 + 1ms, Convert::To<system_clock::time_point>(u"1991-07-10T23:16:53.001Z"));
	EXPECT_EQ(tp2044_12_31T00_00_00 + 999ms, Convert::To<system_clock::time_point>(U"2044-12-31T00:00:00.999Z"));
	EXPECT_EQ(tp3000_05_15T10_25_15 + 567ms, Convert::To<system_clock::time_point>(L"3000-05-15T10:25:15.567Z"));
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenInvalidDelimiters) {
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01 00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970/01/01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01T00.00.00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01T00:00:00/000Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01T00-00-00"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenExtraMinus) {
	EXPECT_THROW(Convert::To<system_clock::time_point>("-1970-01-01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970--01-01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01--01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01T-00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01T00:-00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01T00:00:-00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01T00:00:00.-000Z"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenInvalidDate) {
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-13-01T00:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-32T00:00:00Z"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringShouldThrowExceptionWhenInvalidTime) {
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01T25:00:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01T00:60:00Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01T00:00:60Z"), std::invalid_argument);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1970-01-01T00:00:00.1000Z"), std::invalid_argument);
}

TEST(ConvertChrono, ConvertUtcStringThrowExceptionWhenDateLessThanEpoch) {
	EXPECT_THROW(Convert::To<system_clock::time_point>("970-12-31T00:00:00Z"), std::out_of_range);
	EXPECT_THROW(Convert::To<system_clock::time_point>("1950-01-01T00:00:00Z"), std::out_of_range);
}
