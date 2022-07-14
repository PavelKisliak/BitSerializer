/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/convert.h"
#include "detect_encodings_fixture.h"

using namespace BitSerializer;

using testing::Types;
typedef Types<Convert::Utf8, Convert::Utf16Le, Convert::Utf16Be, Convert::Utf32Le, Convert::Utf32Be> Implementations;

TYPED_TEST_SUITE(DetectEncodingTest, Implementations);


//-----------------------------------------------------------------------------
// Tests for detecting BOM (Byte Order Mark)
//-----------------------------------------------------------------------------
TYPED_TEST(DetectEncodingTest, ShouldReturnFalseWhenInputDataIsEmpty) {
	EXPECT_FALSE(Convert::StartsWithBom<typename TestFixture::utf_type>(""));
}

TYPED_TEST(DetectEncodingTest, ShouldReturnFalseWhenNoBom) {
	EXPECT_FALSE(Convert::StartsWithBom<typename TestFixture::utf_type>("test"));
}

TYPED_TEST(DetectEncodingTest, ShouldReturnFalseWhenBomIsNotFull)
{
	// Arrange
	this->AppendBom(TestFixture::utf_type::bom);
	this->mEncodedBuffer.pop_back();

	// Act / Assert
	EXPECT_FALSE(Convert::StartsWithBom<typename TestFixture::utf_type>(this->mEncodedBuffer));
}

TYPED_TEST(DetectEncodingTest, ShouldReturnTrueWhenPresentOnlyBom)
{
	// Arrange
	this->AppendBom(TestFixture::utf_type::bom);

	// Act / Assert
	EXPECT_TRUE(Convert::StartsWithBom<typename TestFixture::utf_type>(this->mEncodedBuffer));
}

TYPED_TEST(DetectEncodingTest, ShouldReturnTrueWhenPresentBomAndText)
{
	// Arrange
	this->AppendBom(TestFixture::utf_type::bom);
	this->PrepareEncodedData(U"test!");

	// Act / Assert
	EXPECT_TRUE(Convert::StartsWithBom<typename TestFixture::utf_type>(this->mEncodedBuffer));
}

//-----------------------------------------------------------------------------
// Tests for detecting UTF encoding
//-----------------------------------------------------------------------------
TYPED_TEST(DetectEncodingTest, ShouldDetectInString_WithBom)
{
	this->AppendBom(TestFixture::utf_type::bom);
	this->PrepareEncodedData(U"Hello world!");
	this->TestDetectInString();
}

//TYPED_TEST(DetectEncodingTest, ShouldDetectInString_En)
//{
//	this->PrepareEncodedData(U"Hello world!");
//	this->TestDetectInString();
//}

//TYPED_TEST(DetectEncodingTest, ShouldDetectInString_Ru)
//{
//	this->PrepareEncodedData(U"Привет мир!");
//	this->TestDetectInString();
//}
//
//TYPED_TEST(DetectEncodingTest, ShouldDetectInString_Cn)
//{
//	this->PrepareEncodedData(U"世界，您好！");
//	this->TestDetectInString();
//}

TYPED_TEST(DetectEncodingTest, ShouldDetectInStream_BomWithSkip)
{
	this->AppendBom(TestFixture::utf_type::bom);
	this->PrepareEncodedData(U"Hello world!");
	this->TestDetectInStream();
}

TYPED_TEST(DetectEncodingTest, ShouldNoSkipBomInStream_BomWithNoSkip)
{
	this->AppendBom(TestFixture::utf_type::bom);
	this->PrepareEncodedData(U"Hello world!");
	this->TestDetectInStream(false);
}

//TYPED_TEST(DetectEncodingTest, ShouldDetectInStream_En)
//{
//	this->PrepareEncodedData(U"Hello world!");
//	this->TestDetectInStream();
//}

//TYPED_TEST(DetectEncodingTest, ShouldDetectInStream_Ru)
//{
//	this->PrepareEncodedData(U"Привет мир!");
//	this->TestDetectInStream();
//}
//
//TYPED_TEST(DetectEncodingTest, ShouldDetectInStream_Cn)
//{
//	this->PrepareEncodedData(U"世界，您好！");
//	this->TestDetectInStream();
//}
//
