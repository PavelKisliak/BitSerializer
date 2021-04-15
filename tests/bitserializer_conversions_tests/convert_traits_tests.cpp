/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "test_helpers/common_test_entities.h"

using namespace BitSerializer;

class TestConvertibleClass
{
public:
	std::string ToString() const { return std::string(); }
	std::wstring ToWString() const { return std::wstring(); }
	void FromString(const std::string& str) { }
	void FromString(const std::wstring& str) { }
};

class TestNotConvertibleClass { };

TEST(ConvertTraits, ShouldCheckThatClassHasToStringMethod) {
	bool convertibleResult = Convert::Detail::has_to_string_v<TestConvertibleClass, std::string>;
	EXPECT_TRUE(convertibleResult);
	bool notConvertibleResult = Convert::Detail::has_to_string_v<TestNotConvertibleClass, std::string>;
	EXPECT_FALSE(notConvertibleResult);
}

TEST(ConvertTraits, ShouldCheckThatClassHasToWStringMethod) {
	bool convertibleResult = Convert::Detail::has_to_string_v<TestConvertibleClass, std::wstring>;
	EXPECT_TRUE(convertibleResult);
	bool notConvertibleResult = Convert::Detail::has_to_string_v<TestNotConvertibleClass, std::wstring>;
	EXPECT_FALSE(notConvertibleResult);
}

TEST(ConvertTraits, ShouldCheckThatClassHasFromStringMethod) {
	bool convertibleResult = Convert::Detail::has_from_string_v<TestConvertibleClass, std::string>;
	EXPECT_TRUE(convertibleResult);
	bool notConvertibleResult = Convert::Detail::has_from_string_v<TestNotConvertibleClass, std::string>;
	EXPECT_FALSE(notConvertibleResult);
}

TEST(ConvertTraits, ShouldCheckThatClassHasFromWStringMethod) {
	bool convertibleResult = Convert::Detail::has_from_string_v<TestConvertibleClass, std::wstring>;
	EXPECT_TRUE(convertibleResult);
	bool notConvertibleResult = Convert::Detail::has_from_string_v<TestNotConvertibleClass, std::wstring>;
	EXPECT_FALSE(notConvertibleResult);
}
