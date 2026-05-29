/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "encoded_stream_reader_fixture.h"
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;

using testing::Types;
typedef Types<char, char16_t, char32_t, wchar_t> Implementations;

TYPED_TEST_SUITE(EncodedStreamReaderTest, Implementations, );

//------------------------------------------------------------------------------
// Basic read from all UTF encodings
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf8)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Съешь ещё этих мягких французских булок, да выпей чаю");
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf16Le)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf16Le>(U"Съешь ещё этих мягких французских булок, да выпей чаю");
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf16Be)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf16Be>(U"Съешь ещё этих мягких французских булок, да выпей чаю");
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf32Le)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf32Le>(U"Съешь ещё этих мягких французских булок, да выпей чаю");
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf32Be)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf32Be>(U"Съешь ещё этих мягких французских булок, да выпей чаю");
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf8WithBom)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Съешь ещё этих мягких французских булок, да выпей чаю", true);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf32LeWithBom)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf32Le>(U"Съешь ещё этих мягких французских булок, да выпей чаю", true);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadMixedScript)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Hello, 世界! ñööbär — Привет! 123");
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

//------------------------------------------------------------------------------
// Edge cases: empty stream, boundary sizes
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldReadEmptyStream)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"");
	EXPECT_TRUE(this->mEncodedStreamReader->IsEnd());
	EXPECT_TRUE(this->ReadAll().empty());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadStringShorterThanChunkSize)
{
	constexpr size_t chunkSize = 32;
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Cat", false,
		Convert::Utf::UtfEncodingErrorPolicy::Skip,
		Convert::Utf::Detail::GetDefaultErrorMark<TypeParam>(),
		chunkSize);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadStringEqualToChunkSize)
{
	constexpr size_t chunkSize = 32;
	std::u32string str;
	str.reserve(chunkSize);
	for (char32_t c = U'A'; str.size() < chunkSize; ++c)
	{
		str += c;
	}
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(str, false,
		Convert::Utf::UtfEncodingErrorPolicy::Skip,
		Convert::Utf::Detail::GetDefaultErrorMark<TypeParam>(),
		chunkSize);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadStringLargerThanChunkSize)
{
	constexpr size_t chunkSize = 32;
	const auto* longStr = U"Съешь ещё этих мягких французских булок, да выпей чаю. "
		U"Широкая электрификация южных губерний даст мощный толчок подъёму сельского хозяйства.";
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(longStr, false,
		Convert::Utf::UtfEncodingErrorPolicy::Skip,
		Convert::Utf::Detail::GetDefaultErrorMark<TypeParam>(),
		chunkSize);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

//------------------------------------------------------------------------------
// Position tracking
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldTrackPositionAfterRead)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ПриветМир");
	this->ReadAll();

	if constexpr (std::is_same_v<TypeParam, char>)
	{
		// Raw mode: position tracks byte offset directly
		EXPECT_EQ(18, this->mEncodedStreamReader->GetPosition());
	}
	else
	{
		// Decoded mode: position = raw bytes consumed (9 Russian chars × 2 bytes)
		EXPECT_EQ(18, this->mEncodedStreamReader->GetPosition());
	}
}

TYPED_TEST(EncodedStreamReaderTest, ShouldTrackPositionWithBom)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ПриветМир", true);
	this->ReadAll();

	// Position tracking includes BOM offset (mStreamPos starts at bomSize, 
	// advances through content, ending at total input size)
	const auto inputSize = this->mInputString.size();
	EXPECT_EQ(inputSize, this->mEncodedStreamReader->GetPosition());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldTrackPositionAfterPartialRead)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ПриветМир");

	// Read first 3 characters (6 bytes in UTF-8)
	(void)this->mEncodedStreamReader->PeekData(3);
	this->mEncodedStreamReader->SkipChars(3);

	if constexpr (std::is_same_v<TypeParam, char>)
	{
		// Raw mode: SkipChars advances by byte count, position == bytes consumed
		EXPECT_EQ(3, this->mEncodedStreamReader->GetPosition());
	}
	else
	{
		// Decoded mode: CountRawBytes maps 3 chars to 6 bytes in UTF-8
		EXPECT_EQ(6, this->mEncodedStreamReader->GetPosition());
	}
}

//------------------------------------------------------------------------------
// SetPosition (seek)
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldSetPositionToBeginning)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ПриветМир");
	this->ReadAll();

	this->mEncodedStreamReader->SetPosition(0);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldSetPositionToMiddle)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ПриветМир");

	// Read first 5 characters
	(void)this->mEncodedStreamReader->PeekData(5);
	this->mEncodedStreamReader->SkipChars(5);

	if constexpr (std::is_same_v<TypeParam, char>)
	{
		// Raw mode: seek to byte 5
		this->mEncodedStreamReader->SetPosition(5);
	}
	else
	{
		// Decoded mode: seek to byte 10 (5 Russian chars × 2)
		this->mEncodedStreamReader->SetPosition(10);
	}

	const auto view = this->mEncodedStreamReader->PeekData(4);
	typename TestFixture::target_string_type remaining(view.data(), view.size());
	EXPECT_EQ(this->mExpectedString.substr(5), remaining);
}

TYPED_TEST(EncodedStreamReaderTest, ShouldSeekForwardWithinDecodedBuffer)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ПриветМир");

	// Read first 8 chars to populate decoded buffer
	(void)this->mEncodedStreamReader->PeekData(8);
	this->mEncodedStreamReader->SkipChars(8);

	if constexpr (std::is_same_v<TypeParam, char>)
	{
		// Raw mode: seek to byte 6
		this->mEncodedStreamReader->SetPosition(6);
	}
	else
	{
		// Decoded mode: seek to byte 12 (6 Russian chars × 2)
		this->mEncodedStreamReader->SetPosition(12);
	}
	const auto view = this->mEncodedStreamReader->PeekData(3);
	typename TestFixture::target_string_type tail(view.data(), view.size());
	EXPECT_EQ(this->mExpectedString.substr(6), tail);
}

TYPED_TEST(EncodedStreamReaderTest, ShouldSetPositionThrowsWhenOutOfRange)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ABC");
	EXPECT_THROW(this->mEncodedStreamReader->SetPosition(999), std::out_of_range);
}

//------------------------------------------------------------------------------
// Char-by-char API
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldPeekCharWithoutAdvancing)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ABC");

	const auto checkedPeekChar = [this]() -> TypeParam
	{
		auto ch = this->mEncodedStreamReader->PeekChar();
		if (!ch)
		{
			throw std::runtime_error("Unexpected end of stream");
		}
		return *ch;
	};

	EXPECT_EQ(BitSerializer::Convert::To<TypeParam>(U'A'), checkedPeekChar());

	// Second peek returns same char
	const auto ch1 = checkedPeekChar();
	const auto ch2 = checkedPeekChar();
	EXPECT_EQ(ch1, ch2);

	// Position unchanged
	const auto posBefore = this->mEncodedStreamReader->GetPosition();
	(void)checkedPeekChar();
	EXPECT_EQ(posBefore, this->mEncodedStreamReader->GetPosition());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadCharByChar)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ABC");

	const auto readChar = [this]() -> TypeParam
	{
		auto ch = this->mEncodedStreamReader->ReadChar();
		if (!ch)
		{
			throw std::runtime_error("Unexpected end of stream");
		}
		return *ch;
	};
	EXPECT_EQ(BitSerializer::Convert::To<TypeParam>(U'A'), readChar());
	EXPECT_EQ(BitSerializer::Convert::To<TypeParam>(U'B'), readChar());
	EXPECT_EQ(BitSerializer::Convert::To<TypeParam>(U'C'), readChar());
	EXPECT_FALSE(this->mEncodedStreamReader->ReadChar().has_value());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadAllViaReadChar)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Съешь ещё этих мягких французских булок, да выпей чаю");
	EXPECT_EQ(this->mExpectedString, this->ReadAllCharByChar());
}

//------------------------------------------------------------------------------
// Large data (triggers TrimDecodedBuf)
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldReadLargeDataWithMultipleTrimCycles)
{
	std::u32string largeStr;
	largeStr.reserve(200);
	for (int i = 0; i < 200; ++i)
	{
		largeStr += U'А' + (i % 32);
	}
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(largeStr);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

//------------------------------------------------------------------------------
// Error handling
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldSkipInvalidUtf8WithErrorMark)
{
	const std::string invalidUtf8("\xC0\xFFvalid");
	this->PrepareRawStream(invalidUtf8, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	const auto result = this->ReadAll();

	if constexpr (std::is_same_v<TypeParam, char>)
	{
		// Raw mode: bytes pass through without validation
		EXPECT_EQ(7, result.size());
	}
	else
	{
		// Non-raw mode: 2-byte invalid sequence is replaced by error mark, "valid" is decoded
		EXPECT_EQ(6, result.size());
	}
}

TYPED_TEST(EncodedStreamReaderTest, ShouldNotThrowOnInvalidUtf8WhenPolicySkip)
{
	const std::string invalidUtf8("\xC0\xFF");
	this->PrepareRawStream(invalidUtf8, Convert::Utf::UtfEncodingErrorPolicy::Skip);

	EXPECT_NO_THROW(this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldThrowOnInvalidUtf8WhenPolicyThrowError)
{
	const std::string invalidUtf8("\xC0\xFF");
	this->PrepareRawStream(invalidUtf8, Convert::Utf::UtfEncodingErrorPolicy::ThrowError);

	if constexpr (!std::is_same_v<TypeParam, char>)
	{
		EXPECT_THROW(this->ReadAll(), std::invalid_argument);
	}
}
