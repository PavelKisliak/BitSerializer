/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "encoded_stream_writer_fixture.h"
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;

using testing::Types;
typedef Types<Convert::Utf::Utf8, Convert::Utf::Utf16Le, Convert::Utf::Utf16Be, Convert::Utf::Utf32Le, Convert::Utf::Utf32Be> Implementations;

// Tests for all possible variants of UTF encoding
TYPED_TEST_SUITE(EncodedStreamWriterTest, Implementations);


//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamWriterTest, ShouldWriteAnsiString)
{
	// Act
	this->TestWrite("Hello world!");

	// Assert
	this->Assert();
}

TYPED_TEST(EncodedStreamWriterTest, ShouldWriteUtf8String)
{
	// Act
	this->TestWrite(UTF8("Съешь ещё этих мягких французских булок, да выпей чаю"));

	// Assert
	this->Assert();
}

TYPED_TEST(EncodedStreamWriterTest, ShouldWriteUtf8StringWithBom)
{
	// Act
	this->WithBom().TestWrite(UTF8("Съешь ещё этих мягких французских булок, да выпей чаю"));

	// Assert
	this->Assert();
}

TYPED_TEST(EncodedStreamWriterTest, ShouldWriteUtf16String)
{
	// Act
	this->TestWrite(u"Laŭ Ludoviko Zamenhof bongustas freŝa ĉeĥa manĝaĵo kun spicoj.");

	// Assert
	this->Assert();
}

TYPED_TEST(EncodedStreamWriterTest, ShouldWriteUtf16StringWithBom)
{
	// Act
	this->WithBom().TestWrite(u"Laŭ Ludoviko Zamenhof bongustas freŝa ĉeĥa manĝaĵo kun spicoj.");

	// Assert
	this->Assert();
}

TYPED_TEST(EncodedStreamWriterTest, ShouldWriteUtf32String)
{
	// Act
	this->TestWrite(U"Laŭ Ludoviko Zamenhof bongustas freŝa ĉeĥa manĝaĵo kun spicoj.");

	// Assert
	this->Assert();
}

TYPED_TEST(EncodedStreamWriterTest, ShouldWriteUtf32StringWithBom)
{
	// Act
	this->WithBom().TestWrite(U"Laŭ Ludoviko Zamenhof bongustas freŝa ĉeĥa manĝaĵo kun spicoj.");

	// Assert
	this->Assert();
}

TYPED_TEST(EncodedStreamWriterTest, ShouldWriteMixedStrings)
{
	// Act
	this->TestWrite(UTF8("Съешь ещё этих мягких французских булок, да выпей чаю"));
	this->TestWrite(u"Широкая электрификация южных губерний даст мощный толчок подъёму сельского хозяйства");
	this->TestWrite(U"Laŭ Ludoviko Zamenhof bongustas freŝa ĉeĥa manĝaĵo kun spicoj.");

	// Assert
	this->Assert();
}

TYPED_TEST(EncodedStreamWriterTest, ShouldSkipInvalidUtfWhenPolicyIsSkip)
{
	// Arrange
	const std::string wrongStartCodes(MakeStringFromSequence(0b11111110, 0b11111111));
	const std::string source = wrongStartCodes + "test" + wrongStartCodes;
	this->WithBom(Convert::Utf::UtfEncodingErrorPolicy::Skip);

	// Act / Assert
	EXPECT_EQ(BitSerializer::Convert::Utf::UtfEncodingErrorCode::Success, this->TestWrite(source));
	this->Assert();
}

TYPED_TEST(EncodedStreamWriterTest, ShouldStopEncodingInvalidUtfWhenPolicyIsThrowError)
{
	// Skip for UTF-8 type since there is just copying data "as is" without analysis
	if constexpr (!std::is_same_v<typename TestFixture::encoded_char_type, char>)
	{
		// Arrange
		const std::string wrongStartCodes(MakeStringFromSequence(0b11111110, 0b11111111));
		const std::string source = wrongStartCodes + "test" + wrongStartCodes;
		this->WithBom(Convert::Utf::UtfEncodingErrorPolicy::ThrowError);

		// Act / Assert
		EXPECT_EQ(BitSerializer::Convert::Utf::UtfEncodingErrorCode::InvalidSequence, this->TestWrite(source));
	}
}
