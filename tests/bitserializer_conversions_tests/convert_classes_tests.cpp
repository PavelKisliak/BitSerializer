/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Test conversion for class types (struct, class, union)
//-----------------------------------------------------------------------------
class Utf8InternalConvertFixture
{
public:
	Utf8InternalConvertFixture() = default;
	explicit Utf8InternalConvertFixture(std::string expectedValue) noexcept : value(std::move(expectedValue)) { }

	[[nodiscard]] std::string ToString() const { return value; }
	void FromString(std::string_view str) { value = str; }

	std::string value;
};

TEST(ConvertClasses, ConvertToAnyStringViaInternalToString) {
	const auto testFixture = Utf8InternalConvertFixture("16384");
	EXPECT_EQ("16384", Convert::ToString(testFixture));
	EXPECT_EQ(u"16384", Convert::To<std::u16string>(testFixture));
	EXPECT_EQ(U"16384", Convert::To<std::u32string>(testFixture));
}

TEST(ConvertClasses, ConvertFromAnyStringViaInternalFromString) {
	EXPECT_EQ("16384", Convert::To<Utf8InternalConvertFixture>("16384").value);
	EXPECT_EQ("16384", Convert::To<Utf8InternalConvertFixture>(u"16384").value);
	EXPECT_EQ("16384", Convert::To<Utf8InternalConvertFixture>(U"16384").value);
}

//-----------------------------------------------------------------------------

class Utf16InternalConvertFixture
{
public:
	Utf16InternalConvertFixture() = default;
	explicit Utf16InternalConvertFixture(std::u16string expectedValue) noexcept : value(std::move(expectedValue)) { }

	[[nodiscard]] std::u16string ToU16String() const { return value; }
	void FromString(std::u16string_view str) { value = str; }

	std::u16string value;
};

TEST(ConvertClasses, ConvertToAnyStringViaInternalToU16String) {
	const auto testFixture = Utf16InternalConvertFixture(u"16384");
	EXPECT_EQ("16384", Convert::ToString(testFixture));
	EXPECT_EQ(u"16384", Convert::To<std::u16string>(testFixture));
	EXPECT_EQ(U"16384", Convert::To<std::u32string>(testFixture));
}

TEST(ConvertClasses, ConvertFromAnyStringViaInternalFromUtf16String) {
	EXPECT_EQ(u"16384", Convert::To<Utf16InternalConvertFixture>("16384").value);
	EXPECT_EQ(u"16384", Convert::To<Utf16InternalConvertFixture>(u"16384").value);
	EXPECT_EQ(u"16384", Convert::To<Utf16InternalConvertFixture>(U"16384").value);
}

//-----------------------------------------------------------------------------

class Utf32InternalConvertFixture
{
public:
	Utf32InternalConvertFixture() = default;
	explicit Utf32InternalConvertFixture(std::u32string expectedValue) noexcept : value(std::move(expectedValue)) { }

	[[nodiscard]] std::u32string ToU32String() const { return value; }
	void FromString(std::u32string_view str) { value = str; }
	std::u32string value;
};

TEST(ConvertClasses, ConvertToAnyStringViaInternalToU32String) {
	const auto testFixture = Utf32InternalConvertFixture(U"16384");
	EXPECT_EQ("16384", Convert::ToString(testFixture));
	EXPECT_EQ(u"16384", Convert::To<std::u16string>(testFixture));
	EXPECT_EQ(U"16384", Convert::To<std::u32string>(testFixture));
}

TEST(ConvertClasses, ConvertFromAnyStringViaInternalFromUtf32String) {
	EXPECT_EQ(U"16384", Convert::To<Utf32InternalConvertFixture>("16384").value);
	EXPECT_EQ(U"16384", Convert::To<Utf32InternalConvertFixture>(u"16384").value);
	EXPECT_EQ(U"16384", Convert::To<Utf32InternalConvertFixture>(U"16384").value);
}

//-----------------------------------------------------------------------------
// Test conversion via global function
//-----------------------------------------------------------------------------
class Utf8ExternalConvertFixture
{
public:
	Utf8ExternalConvertFixture() = default;
	explicit Utf8ExternalConvertFixture(std::string expectedValue) noexcept : value(std::move(expectedValue)) { }

	std::string value;
};


// ReSharper disable once CppDeclaratorNeverUsed
static void To(const Utf8ExternalConvertFixture& in, std::string& out)
{
	out = in.value;
}

// ReSharper disable once CppDeclaratorNeverUsed
static void To(std::string_view in, Utf8ExternalConvertFixture& out)
{
	out.value = in;
}

TEST(ConvertClasses, ConvertToStringViaExternalConvertFunc) {
	const auto testFixture = Utf8ExternalConvertFixture("16384");
	EXPECT_EQ("16384", Convert::ToString(testFixture));
}

TEST(ConvertClasses, ConvertFromStringViaExternalConvertFunc) {
	EXPECT_EQ("16384", Convert::To<Utf8ExternalConvertFixture>("16384").value);
}
