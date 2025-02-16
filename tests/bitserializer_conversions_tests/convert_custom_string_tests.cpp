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
	class CCustomString  // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		CCustomString() = default;
		explicit CCustomString(std::string_view str) : mInternalString(str) {}

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
	void To(const T& in, CCustomString& out)
	{
		out.Append(Convert::To<std::string>(in));
	}

	template <typename T>
	void To(const CCustomString& in, T& out)
	{
		out = Convert::To<T>(in.ToGenericString());
	}
}

//-----------------------------------------------------------------------------
// Test conversion for custom strings
//-----------------------------------------------------------------------------
TEST(ConvertCustomString, ConvertCustomStringToNumber)
{
	EXPECT_EQ(100, Convert::To<int>(TestSpace::CCustomString("100")));
	EXPECT_EQ(123.123f, Convert::To<float>(TestSpace::CCustomString("123.123")));
}

TEST(ConvertCustomString, ConvertCustomStringToStdString)
{
	EXPECT_EQ("Hello world!", Convert::To<std::string>(TestSpace::CCustomString("Hello world!")));
	EXPECT_EQ(u"Привет мир!", Convert::To<std::u16string>(TestSpace::CCustomString(UTF8("Привет мир!"))));
	EXPECT_EQ(U"世界，您好！", Convert::To<std::u32string>(TestSpace::CCustomString(UTF8("世界，您好！"))));
}

TEST(ConvertCustomString, ConvertNumberToCustomString)
{
	EXPECT_EQ("100", Convert::To<TestSpace::CCustomString>(100).ToGenericString());
	EXPECT_EQ("123.123", Convert::To<TestSpace::CCustomString>(123.123f).ToGenericString());
}

TEST(ConvertCustomString, ConvertRawStringToCustomString)
{
	EXPECT_EQ("Hello world!", Convert::To<TestSpace::CCustomString>("Hello world!").ToGenericString());
	EXPECT_EQ(UTF8("Привет мир!"), Convert::To<TestSpace::CCustomString>(UTF8("Привет мир!")).ToGenericString());

	EXPECT_EQ(UTF8("世界，您好！"), Convert::To<TestSpace::CCustomString>(u"世界，您好！").ToGenericString());
	EXPECT_EQ(UTF8("😀😎🙋"), Convert::To<TestSpace::CCustomString>(U"😀😎🙋").ToGenericString());
}

TEST(ConvertCustomString, ConvertStdStringToCustomString)
{
	EXPECT_EQ("Hello world!", Convert::To<TestSpace::CCustomString>(std::string("Hello world!")).ToGenericString());
	EXPECT_EQ(UTF8("Привет мир!"), Convert::To<TestSpace::CCustomString>(std::u16string(u"Привет мир!")).ToGenericString());
	EXPECT_EQ(UTF8("😀😎🙋"), Convert::To<TestSpace::CCustomString>(std::u32string(U"😀😎🙋")).ToGenericString());
}

#pragma warning(pop)
