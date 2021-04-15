/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "test_helpers/common_test_entities.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test conversion for enum types
//-----------------------------------------------------------------------------
TEST(ConvertEnums, EnumFromString) {
	EXPECT_EQ(TestEnum::One, Convert::To<TestEnum>("One"));
}

TEST(ConvertEnums, EnumFromWString) {
	EXPECT_EQ(TestEnum::Two, Convert::To<TestEnum>(L"Two"));
}

TEST(ConvertEnums, EnumToString) {
	EXPECT_EQ("Three", Convert::ToString(TestEnum::Three));
}

TEST(ConvertEnums, EnumToWString) {
	EXPECT_EQ(L"Four", Convert::ToWString(TestEnum::Four));
}

//-----------------------------------------------------------------------------
// Test conversion for class types (struct, class, union)
//-----------------------------------------------------------------------------
TEST(ConvertClasses, ClassFromString) {
	auto actual = Convert::To<TestPointClass>("100 -200");
	EXPECT_EQ(TestPointClass(100, -200), actual);
}

TEST(ConvertClasses, ClassFromWString) {
	auto actual = Convert::To<TestPointClass>(L"-123 555");
	EXPECT_EQ(TestPointClass(-123, 555), actual);
}

TEST(ConvertClasses, ClassToString) {
	EXPECT_EQ("16384 32768", Convert::ToString(TestPointClass(16384, 32768)));
}

TEST(ConvertClasses, ClassToWString) {
	EXPECT_EQ(L"-777 -888", Convert::ToWString(TestPointClass(-777, -888)));
}
