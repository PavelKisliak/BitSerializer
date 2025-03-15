/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test conversion for enum types
//-----------------------------------------------------------------------------
TEST(ConvertEnums, EnumFromCStr) {
	EXPECT_EQ(TestEnum::One, Convert::To<TestEnum>("One"));
	EXPECT_EQ(TestEnum::Two, Convert::To<TestEnum>(u"TWO"));
	EXPECT_EQ(TestEnum::Three, Convert::To<TestEnum>(U"three"));
}

TEST(ConvertEnums, EnumToString) {
	EXPECT_EQ("One", Convert::ToString(TestEnum::One));
	EXPECT_EQ(u"Two", Convert::To<std::u16string>(TestEnum::Two));
	EXPECT_EQ(U"Three", Convert::To<std::u32string>(TestEnum::Three));
}

TEST(ConvertEnums, ConvertEnumToStream) {
	std::ostringstream oss;
	oss << TestEnum::Five;
	EXPECT_EQ("Five", oss.str());
}

TEST(ConvertEnums, ConvertEnumToWStream) {
	std::wostringstream oss;
	oss << TestEnum::Five;
	EXPECT_EQ(L"Five", oss.str());
}

TEST(ConvertEnums, ConvertEnumFromStream) {
	std::stringstream stream("Five");
	TestEnum actual;
	stream >> actual;
	EXPECT_EQ(TestEnum::Five, actual);
}

TEST(ConvertEnums, ConvertEnumFromWStream) {
	std::wstringstream stream(L"Two");
	TestEnum actual;
	stream >> actual;
	EXPECT_EQ(TestEnum::Two, actual);
}

TEST(ConvertEnums, ConvertEnumFromStreamWithSkipSpaces) {
	std::stringstream stream("\t\t  Three ");
	TestEnum actual;
	stream >> actual;
	EXPECT_EQ(TestEnum::Three, actual);
}

TEST(ConvertEnums, ConvertEnumChainFromStream) {
	std::stringstream stream("One Two");
	TestEnum actualEnum1, actualEnum2;
	stream >> actualEnum1 >> actualEnum2;
	EXPECT_EQ(TestEnum::One, actualEnum1);
	EXPECT_EQ(TestEnum::Two, actualEnum2);
}
