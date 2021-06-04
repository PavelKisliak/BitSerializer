/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"

using namespace BitSerializer;


// Custom string test fixture
class CCustomString  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	CCustomString() = default;
	explicit CCustomString(std::string_view str) : mInternalString(str) { }

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
static std::errc To(const T& in, CCustomString& out)
{
	out.Append(Convert::To<std::string>(in));
	return std::errc();
}

template <typename T>
static std::errc To(const CCustomString& in, T& out)
{
	out = Convert::To<T>(in.ToGenericString());
	return std::errc();
}

//-----------------------------------------------------------------------------
// Test conversion for custom strings
//-----------------------------------------------------------------------------
TEST(ConvertCustomString, ConvertFromCustomString) {
	EXPECT_EQ(100, Convert::To<int>(CCustomString("100")));
	EXPECT_EQ(123.123f, Convert::To<float>(CCustomString("123.123")));
}

TEST(ConvertCustomString, ConvertToCustomString) {
	EXPECT_EQ("100", Convert::To<CCustomString>(100).ToGenericString());
	EXPECT_EQ("123.123", Convert::To<CCustomString>(123.123f).ToGenericString());
}
