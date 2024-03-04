/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"
#include "testing_tools/string_utils.h"
#include "testing_tools/auto_fixture.h"

using namespace BitSerializer;

#pragma warning(push)
#pragma warning(disable: 4566)
//-----------------------------------------------------------------------------
// Test conversion for filesystem::path
//-----------------------------------------------------------------------------
TEST(ConvertStd, ConvertAnsiPathToAnyString) {
	EXPECT_EQ("c:/temp", Convert::ToString(std::filesystem::path("c:/temp")));
	EXPECT_EQ(u"c:/temp", Convert::To<std::u16string>(std::filesystem::path(u"c:/temp")));
	EXPECT_EQ(L"c:/temp", Convert::ToWString(std::filesystem::path(L"c:/temp")));
	EXPECT_EQ(U"c:/temp", Convert::To<std::u32string>(std::filesystem::path(U"c:/temp")));
}

TEST(ConvertStd, ConvertNativePathToUtf8) {
	EXPECT_EQ(UTF8("c:/Привет Мир"), Convert::ToString(std::filesystem::path(UPATH("c:/Привет Мир"))));
	EXPECT_EQ(UTF8("c:/世界，您好"), Convert::ToString(std::filesystem::path(UPATH("c:/世界，您好"))));
}

TEST(ConvertStd, ConvertNativePathToWString) {
	EXPECT_EQ(L"c:/Привет Мир", Convert::ToWString(std::filesystem::path(UPATH("c:/Привет Мир"))));
	EXPECT_EQ(L"c:/世界，您好", Convert::ToWString(std::filesystem::path(UPATH("c:/世界，您好"))));
}

TEST(ConvertStd, ConvertNativePathToUtf16) {
	EXPECT_EQ(u"c:/Привет Мир", Convert::To<std::u16string>(std::filesystem::path(UPATH("c:/Привет Мир"))));
	EXPECT_EQ(u"c:/世界，您好", Convert::To<std::u16string>(std::filesystem::path(UPATH("c:/世界，您好"))));
}

TEST(ConvertStd, ConvertNativePathToUtf32) {
	EXPECT_EQ(U"c:/Привет Мир", Convert::To<std::u32string>(std::filesystem::path(UPATH("c:/Привет Мир"))));
	EXPECT_EQ(U"c:/世界，您好", Convert::To<std::u32string>(std::filesystem::path(UPATH("c:/世界，您好"))));
}

//-----------------------------------------------------------------------------

TEST(ConvertStd, ConvertUtf8StringToPath) {
	EXPECT_EQ(UPATH("c:/Привет Мир"), Convert::To<std::filesystem::path>(UTF8("c:/Привет Мир")).native());
	EXPECT_EQ(UPATH("c:/世界，您好"), Convert::To<std::filesystem::path>(UTF8("c:/世界，您好")).native());
}

TEST(ConvertStd, ConvertWStringToPath) {
	EXPECT_EQ(UPATH("c:/Привет Мир"), Convert::To<std::filesystem::path>(L"c:/Привет Мир").native());
	EXPECT_EQ(UPATH("c:/世界，您好"), Convert::To<std::filesystem::path>(L"c:/世界，您好").native());
}

TEST(ConvertStd, ConvertUtf16StringToPath) {
	EXPECT_EQ(UPATH("c:/Привет Мир"), Convert::To<std::filesystem::path>(u"c:/Привет Мир").native());
	EXPECT_EQ(UPATH("c:/世界，您好"), Convert::To<std::filesystem::path>(u"c:/世界，您好").native());
}

TEST(ConvertStd, ConvertUtf32StringToPath) {
	EXPECT_EQ(UPATH("c:/Привет Мир"), Convert::To<std::filesystem::path>(U"c:/Привет Мир").native());
	EXPECT_EQ(UPATH("c:/世界，您好"), Convert::To<std::filesystem::path>(U"c:/世界，您好").native());
}

#pragma warning(pop)
