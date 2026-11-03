/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"
#include "testing_tools/string_utils.h"

using namespace BitSerializer;

#pragma warning(push)
#pragma warning(disable: 4566)

namespace TestSpace
{
	// Custom string test fixture
	class CustomString  // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		CustomString() = default;
		explicit CustomString(std::string_view str) : mInternalString(str) {}

		[[nodiscard]] const std::string& ToGenericString() const {
			return mInternalString;
		}

		void Append(std::string_view str) {
			mInternalString.append(str);
		}

	private:
		std::string mInternalString;
	};

	template <typename T>
	[[maybe_unused]] static void To(const T& in, CustomString& out)
	{
		out.Append(Convert::To<std::string>(in));
	}

	template <typename T>
	[[maybe_unused]] static void To(const CustomString& in, T& out)
	{
		out = Convert::To<T>(in.ToGenericString());
	}
}

//-----------------------------------------------------------------------------
// Test conversion for custom strings
//-----------------------------------------------------------------------------
TEST(ConvertCustomString, ConvertCustomStringToNumber)
{
	EXPECT_EQ(100, Convert::To<int>(TestSpace::CustomString("100")));
	EXPECT_EQ(123.123f, Convert::To<float>(TestSpace::CustomString("123.123")));
}

TEST(ConvertCustomString, ConvertCustomStringToStdString)
{
	EXPECT_EQ("Hello world!", Convert::To<std::string>(TestSpace::CustomString("Hello world!")));
	EXPECT_EQ(u"Привет мир!", Convert::To<std::u16string>(TestSpace::CustomString(UTF8("Привет мир!"))));
	EXPECT_EQ(U"世界，您好！", Convert::To<std::u32string>(TestSpace::CustomString(UTF8("世界，您好！"))));
}

TEST(ConvertCustomString, ConvertNumberToCustomString)
{
	EXPECT_EQ("100", Convert::To<TestSpace::CustomString>(100).ToGenericString());
	EXPECT_EQ("-0.1875", Convert::To<TestSpace::CustomString>(-0.1875f).ToGenericString());
}

TEST(ConvertCustomString, ConvertRawStringToCustomString)
{
	EXPECT_EQ("Hello world!", Convert::To<TestSpace::CustomString>("Hello world!").ToGenericString());
	EXPECT_EQ(UTF8("Привет мир!"), Convert::To<TestSpace::CustomString>(UTF8("Привет мир!")).ToGenericString());

	EXPECT_EQ(UTF8("世界，您好！"), Convert::To<TestSpace::CustomString>(u"世界，您好！").ToGenericString());
	EXPECT_EQ(UTF8("😀😎🙋"), Convert::To<TestSpace::CustomString>(U"😀😎🙋").ToGenericString());
}

TEST(ConvertCustomString, ConvertStdStringToCustomString)
{
	EXPECT_EQ("Hello world!", Convert::To<TestSpace::CustomString>(std::string("Hello world!")).ToGenericString());
	EXPECT_EQ(UTF8("Привет мир!"), Convert::To<TestSpace::CustomString>(std::u16string(u"Привет мир!")).ToGenericString());
	EXPECT_EQ(UTF8("😀😎🙋"), Convert::To<TestSpace::CustomString>(std::u32string(U"😀😎🙋")).ToGenericString());
}

#pragma warning(pop)
