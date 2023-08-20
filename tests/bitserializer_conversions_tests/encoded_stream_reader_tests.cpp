/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "encoded_stream_reader_fixture.h"
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;

using testing::Types;
typedef Types<BitSerializer::Convert::Utf8, BitSerializer::Convert::Utf16Le, BitSerializer::Convert::Utf32Le> Implementations;

// Tests for all implementations of ICsvReader
TYPED_TEST_SUITE(EncodedStreamReaderTest, Implementations);


//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromInputStringLessThanChunkSize)
{
	// Arrange
	this->template PrepareEncodedStreamReader<Convert::Utf8>(U"Cat");

	// Act
	this->ReadFromStream();

	// Assert
	EXPECT_EQ(this->mExpectedString, this->mActualString);
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromInputStringEqualWithChunkSize)
{
	// Arrange
	this->template PrepareEncodedStreamReader<Convert::Utf8>(U"Test");

	// Act
	this->ReadFromStream();

	// Assert
	EXPECT_EQ(this->mExpectedString, this->mActualString);
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf8StreamWithoutBom)
{
	// Arrange
	this->template PrepareEncodedStreamReader<Convert::Utf8>(U"Привет мир!", false);

	// Act
	this->ReadFromStream();

	// Assert
	EXPECT_EQ(this->mExpectedString, this->mActualString);
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf8StreamWithBom)
{
	// Arrange
	this->template PrepareEncodedStreamReader<Convert::Utf8>(U"Привет мир!", true);

	// Act
	this->ReadFromStream();

	// Assert
	EXPECT_EQ(this->mExpectedString, this->mActualString);
}

//TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf16LeStreamWithoutBom)
//{
//	// Arrange
//	this->template PrepareEncodedStreamReader<Convert::Utf16Le>(U"Привет мир!", false);
//
//	// Act
//	this->ReadFromStream();
//
//	// Assert
//	EXPECT_EQ(this->mExpectedString, this->mActualString);
//}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf16LeStreamWithBom)
{
	// Arrange
	this->template PrepareEncodedStreamReader<Convert::Utf16Le>(U"Привет мир!", true);

	// Act
	this->ReadFromStream();

	// Assert
	EXPECT_EQ(this->mExpectedString, this->mActualString);
}

//TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf16BeStreamWithoutBom)
//{
//	// Arrange
//	this->template PrepareEncodedStreamReader<Convert::Utf16Be>(U"Привет мир!", false);
//
//	// Act
//	this->ReadFromStream();
//
//	// Assert
//	EXPECT_EQ(this->mExpectedString, this->mActualString);
//}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf16BeStreamWithBom)
{
	// Arrange
	this->template PrepareEncodedStreamReader<Convert::Utf16Be>(U"Привет мир!", true);

	// Act
	this->ReadFromStream();

	// Assert
	EXPECT_EQ(this->mExpectedString, this->mActualString);
}

//TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf32LeStreamWithoutBom)
//{
//	// Arrange
//	this->template PrepareEncodedStreamReader<Convert::Utf23Le>(U"Привет мир!", false);
//
//	// Act
//	this->ReadFromStream();
//
//	// Assert
//	EXPECT_EQ(this->mExpectedString, this->mActualString);
//}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf32LeStreamWithBom)
{
	// Arrange
	this->template PrepareEncodedStreamReader<Convert::Utf32Le>(U"Привет мир!", true);

	// Act
	this->ReadFromStream();

	// Assert
	EXPECT_EQ(this->mExpectedString, this->mActualString);
}

//TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf32BeStreamWithoutBom)
//{
//	// Arrange
//	this->template PrepareEncodedStreamReader<Convert::Utf32Be>(U"Привет мир!", false);
//
//	// Act
//	this->ReadFromStream();
//
//	// Assert
//	EXPECT_EQ(this->mExpectedString, this->mActualString);
//}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf32BeStreamWithBom)
{
	// Arrange
	this->template PrepareEncodedStreamReader<Convert::Utf32Be>(U"Привет мир!", true);

	// Act
	this->ReadFromStream();

	// Assert
	EXPECT_EQ(this->mExpectedString, this->mActualString);
}
