/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"
#include "detect_encodings_fixture.h"

using namespace BitSerializer;

using testing::Types;
typedef Types<Convert::Utf::Utf8, Convert::Utf::Utf16Le, Convert::Utf::Utf16Be, Convert::Utf::Utf32Le, Convert::Utf::Utf32Be> Implementations;

TYPED_TEST_SUITE(DetectEncodingTest, Implementations, );


//-----------------------------------------------------------------------------
// Tests for detecting BOM (Byte Order Mark)
//-----------------------------------------------------------------------------
TYPED_TEST(DetectEncodingTest, ShouldReturnFalseWhenInputDataIsEmpty) {
	EXPECT_FALSE(Convert::Utf::StartsWithBom<typename TestFixture::utf_type>(""));
}

TYPED_TEST(DetectEncodingTest, ShouldReturnFalseWhenNoBom) {
	EXPECT_FALSE(Convert::Utf::StartsWithBom<typename TestFixture::utf_type>("test"));
}

TYPED_TEST(DetectEncodingTest, ShouldReturnFalseWhenBomIsNotFull)
{
	// Arrange
	this->AppendBom(TestFixture::utf_type::bom);
	this->mEncodedBuffer.pop_back();

	// Act / Assert
	EXPECT_FALSE(Convert::Utf::StartsWithBom<typename TestFixture::utf_type>(this->mEncodedBuffer));
}

TYPED_TEST(DetectEncodingTest, ShouldReturnTrueWhenPresentOnlyBom)
{
	// Arrange
	this->AppendBom(TestFixture::utf_type::bom);

	// Act / Assert
	EXPECT_TRUE(Convert::Utf::StartsWithBom<typename TestFixture::utf_type>(this->mEncodedBuffer));
}

TYPED_TEST(DetectEncodingTest, ShouldReturnTrueWhenPresentBomAndText)
{
	// Arrange
	this->AppendBom(TestFixture::utf_type::bom);
	this->PrepareEncodedData(U"test!");

	// Act / Assert
	EXPECT_TRUE(Convert::Utf::StartsWithBom<typename TestFixture::utf_type>(this->mEncodedBuffer));
}

//-----------------------------------------------------------------------------
// Tests for detecting UTF encoding in string (via detect BOM and data analysis)
//-----------------------------------------------------------------------------
TYPED_TEST(DetectEncodingTest, ShouldDetectInString_WithBom)
{
	this->AppendBom(TestFixture::utf_type::bom);
	this->PrepareEncodedData(U"Hello world!");
	this->TestDetectInString();
}

TYPED_TEST(DetectEncodingTest, ShouldDetectInString_NoBom)
{
	this->PrepareEncodedData(U"Hello world!");
	this->TestDetectInString();
}


//-----------------------------------------------------------------------------
// Tests for detecting UTF encoding in stream (via detect BOM and data analysis)
//-----------------------------------------------------------------------------
TYPED_TEST(DetectEncodingTest, ShouldDetectInStream_BomSkip)
{
	this->AppendBom(TestFixture::utf_type::bom);
	this->PrepareEncodedData(U"Hello world!");
	this->TestDetectInStream();
}

TYPED_TEST(DetectEncodingTest, ShouldDetectInStream_BomNoSkip)
{
	this->AppendBom(TestFixture::utf_type::bom);
	this->PrepareEncodedData(U"Hello world!");
	this->TestDetectInStream(false);
}

TYPED_TEST(DetectEncodingTest, ShouldDetectInStream_NoBom)
{
	this->PrepareEncodedData(U"Hello world!");
	this->TestDetectInStream();
}
