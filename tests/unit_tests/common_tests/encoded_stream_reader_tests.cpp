/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "encoded_stream_reader_fixture.h"
#include "testing_tools/common_test_entities.h"

using namespace BitSerializer;
using ReaderAccess = Convert::Utf::EncodedStreamReaderAccess;

using testing::Types;
typedef Types<char, char16_t, char32_t, wchar_t> Implementations;

TYPED_TEST_SUITE(EncodedStreamReaderTest, Implementations, );

//------------------------------------------------------------------------------
// Constructor validation
//------------------------------------------------------------------------------

TEST(EncodedStreamReaderConstructorTest, ShouldThrowWhenChunkSizeTooSmall)
{
	std::stringstream ss("data");
	EXPECT_THROW((BitSerializer::Convert::Utf::EncodedStreamReader<char>(ss,
		BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip, nullptr, 31)),
		std::invalid_argument);
}

TEST(EncodedStreamReaderConstructorTest, ShouldThrowWhenChunkSizeNotMultipleOf4)
{
	std::stringstream ss("data");
	EXPECT_THROW((BitSerializer::Convert::Utf::EncodedStreamReader<char>(ss,
		BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip, nullptr, 42)),
		std::invalid_argument);
}

//------------------------------------------------------------------------------
// Encoding detection
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldDetectSourceUtf8)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"test");
	EXPECT_EQ(BitSerializer::Convert::Utf::UtfType::Utf8, this->mEncodedStreamReader->GetSourceUtfType());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldDetectSourceUtf16Le)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf16Le>(U"test");
	EXPECT_EQ(BitSerializer::Convert::Utf::UtfType::Utf16le, this->mEncodedStreamReader->GetSourceUtfType());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldDetectSourceUtf16Be)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf16Be>(U"test");
	EXPECT_EQ(BitSerializer::Convert::Utf::UtfType::Utf16be, this->mEncodedStreamReader->GetSourceUtfType());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldDetectSourceUtf32Le)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf32Le>(U"test");
	EXPECT_EQ(BitSerializer::Convert::Utf::UtfType::Utf32le, this->mEncodedStreamReader->GetSourceUtfType());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldDetectSourceUtf32Be)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf32Be>(U"test");
	EXPECT_EQ(BitSerializer::Convert::Utf::UtfType::Utf32be, this->mEncodedStreamReader->GetSourceUtfType());
}

//------------------------------------------------------------------------------
// Read from all UTF encodings
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromEmptyStream)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"");
	EXPECT_TRUE(this->mEncodedStreamReader->IsEnd());
	EXPECT_TRUE(this->ReadAll().empty());
}

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
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Съешь ещё этих мягких французских булок, да выпей чаю", this->DefaultChunkSize, true);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf16LeWithBom)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf16Le>(U"Съешь ещё этих мягких французских булок, да выпей чаю", this->DefaultChunkSize, true);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf16BeWithBom)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf16Be>(U"Съешь ещё этих мягких французских булок, да выпей чаю", this->DefaultChunkSize, true);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf32LeWithBom)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf32Le>(U"Съешь ещё этих мягких французских булок, да выпей чаю", this->DefaultChunkSize, true);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadFromUtf32BeWithBom)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf32Be>(U"Съешь ещё этих мягких французских булок, да выпей чаю", this->DefaultChunkSize, true);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadMixedScript)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Hello, 世界! ñööbär — Привет! 123");
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadStringEqualToChunkSize)
{
	constexpr size_t chunkSize = 32;
	std::u32string str;
	str.reserve(chunkSize);
	for (char32_t c = U'A'; str.size() < chunkSize; ++c) {
		str += c;
	}
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(str, chunkSize);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadUtf16LeEqualToChunkSize)
{
	constexpr size_t chunkSize = 32;
	std::u32string str;
	str.reserve(16);
	for (char32_t c = U'A'; str.size() < 16; ++c) {
		str += c;
	}
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf16Le>(str, chunkSize);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadStringLargerThanChunkSize)
{
	constexpr size_t chunkSize = 32;
	const auto* longStr = U"Съешь ещё этих мягких французских булок, да выпей чаю. "
		U"Широкая электрификация южных губерний даст мощный толчок подъёму сельского хозяйства.";
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(longStr, chunkSize);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

//------------------------------------------------------------------------------
// PeekChars tests
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldPeekCharsReturnAllWhenEnoughData)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ABCDEFGHIJ");
	const auto view = this->mEncodedStreamReader->PeekChars(10);
	EXPECT_EQ(10, view.size());
	EXPECT_EQ(this->mExpectedString, view);
}

TYPED_TEST(EncodedStreamReaderTest, ShouldPeekCharsReturnAvailableWhenMinCharsExceedsData)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Hello");
	const auto view = this->mEncodedStreamReader->PeekChars(64);
	EXPECT_EQ(5, view.size());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldPeekCharsReturnEmptyWhenStreamExhausted)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Hi");
	(void)this->mEncodedStreamReader->PeekChars(2);
	this->mEncodedStreamReader->SkipChars(2);
	const auto view = this->mEncodedStreamReader->PeekChars(1);
	EXPECT_TRUE(view.empty());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldPeekCharsWorkWithUtf16LeWhenMinCharsExceedsData)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf16Le>(U"Hello");
	const auto view = this->mEncodedStreamReader->PeekChars(64);
	EXPECT_EQ(5, view.size());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldPeekCharsAndSkipCharsInSequence)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ABCDE");

	auto view = this->mEncodedStreamReader->PeekChars(3);
	EXPECT_EQ(5, view.size());
	this->mEncodedStreamReader->SkipChars(2);

	view = this->mEncodedStreamReader->PeekChars(3);
	EXPECT_EQ(3, view.size());
	EXPECT_EQ(this->mExpectedString.substr(2), view);
	this->mEncodedStreamReader->SkipChars(3);

	EXPECT_TRUE(this->mEncodedStreamReader->PeekChars(1).empty());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldAccumulateDecodedDataOnSequentialPeeks)
{
	constexpr size_t chunkSize = 32;
	std::u32string source;
	source.reserve(64);
	for (char32_t c = U'A'; source.size() < 64; ++c) {
		source += c;
	}
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(source, chunkSize);

	auto view = this->mEncodedStreamReader->PeekChars(32);
	EXPECT_EQ(this->mExpectedString.substr(0, 32), view.substr(0, 32));

	if constexpr (std::is_same_v<TypeParam, char>)
	{
		// Raw mode: PeekChars returns a view from mStreamPos through end of raw buffer.
		// mStreamPos is never advanced by PeekChars, and EnsureRawDataAvailable only
		// reads ahead if mStreamPos + chunkSize > rawBytes.size(). So without SkipChars,
		// subsequent PeekChars calls return the same view.
		const auto size1 = view.size();
		const auto view2 = this->mEncodedStreamReader->PeekChars(48);
		EXPECT_EQ(size1, view2.size());
		const auto view3 = this->mEncodedStreamReader->PeekChars(64);
		EXPECT_EQ(size1, view3.size());
		EXPECT_EQ(0, memcmp(view.data(), view2.data(), size1));
	}
	else
	{
		// Decoded mode: PeekChars decodes incrementally into mDecodedBuf.
		// Second call - requests more than first chunk without SkipChars.
		view = this->mEncodedStreamReader->PeekChars(48);
		ASSERT_GE(view.size(), 48);
		EXPECT_EQ(this->mExpectedString.substr(0, 48), view.substr(0, 48));

		// Third call - reads all remaining data
		view = this->mEncodedStreamReader->PeekChars(64);
		ASSERT_GE(view.size(), 64);
		EXPECT_EQ(this->mExpectedString.substr(0, 64), view.substr(0, 64));
	}
}

//------------------------------------------------------------------------------
// GetPosition tests
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
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ПриветМир", this->DefaultChunkSize, true);
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
	(void)this->mEncodedStreamReader->PeekChars(3);
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

TYPED_TEST(EncodedStreamReaderTest, ShouldSeekBackwardWithinDecodedBuffer)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ПриветМир");

	// Read first 8 chars (16 UTF-8 bytes) to populate decoded buffer
	(void)this->mEncodedStreamReader->PeekChars(8);
	this->mEncodedStreamReader->SkipChars(8);

	if constexpr (std::is_same_v<TypeParam, char>)
	{
		this->mEncodedStreamReader->SetPosition(6);
	}
	else
	{
		// Decoded mode: seek back to UTF-8 byte 12 = 6 chars × 2 bytes
		this->mEncodedStreamReader->SetPosition(12);
	}
	const auto view = this->mEncodedStreamReader->PeekChars(3);
	EXPECT_EQ(this->mExpectedString.substr(6), view);
}

TYPED_TEST(EncodedStreamReaderTest, ShouldSetPositionThrowsWhenOutOfRange)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ABC");
	EXPECT_THROW(this->mEncodedStreamReader->SetPosition(999), std::out_of_range);
}

TYPED_TEST(EncodedStreamReaderTest, ShouldSetPositionBeyondFirstChunk)
{
	constexpr size_t chunkSize = 32;
	std::u32string source;
	source.reserve(100);
	for (char32_t c = U'A'; source.size() < 100; ++c) {
		source += c;
	}

	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(source, chunkSize);

	// Constructor reads first chunk (32 bytes). Seek to position 50 (beyond first chunk).
	this->mEncodedStreamReader->SetPosition(50);
	EXPECT_EQ(this->mExpectedString.substr(50), this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldSeekBackwardViaRawFallback)
{
	constexpr size_t chunkSize = 32;
	std::u32string source;
	source.reserve(100);
	for (char32_t c = U'A'; source.size() < 100; ++c) {
		source += c;
	}

	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(source, chunkSize);

	// Seek forward with no decoded data — sets mConsumedRawBytes past mStreamOffset
	this->mEncodedStreamReader->SetPosition(20);

	// Seek backward in raw buffer — pos < mConsumedRawBytes, no decoded data → fallback
	this->mEncodedStreamReader->SetPosition(5);
	EXPECT_EQ(this->mExpectedString.substr(5), this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldSeekToBeginningAfterTrim)
{
	constexpr size_t chunkSize = 32;
	std::u32string str;
	str.reserve(40);
	for (char32_t c = U'A'; str.size() < 40; ++c) {
		str += c;
	}

	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(str, chunkSize);

	// Read all - this triggers TrimConsumedData for decoded modes
	this->ReadAll();

	// Seek back to beginning — must work even after trim
	this->mEncodedStreamReader->SetPosition(0);
	EXPECT_EQ(this->mExpectedString, this->ReadAll());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldSeekToMiddleAfterTrim)
{
	constexpr size_t chunkSize = 32;
	std::u32string str;
	str.reserve(40);
	for (char32_t c = U'A'; str.size() < 40; ++c) {
		str += c;
	}

	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(str, chunkSize);

	// Read first half to trigger trim
	(void)this->mEncodedStreamReader->PeekChars(35);
	this->mEncodedStreamReader->SkipChars(35);

	// Seek back to byte 5 and verify
	this->mEncodedStreamReader->SetPosition(5);
	const auto view = this->mEncodedStreamReader->PeekChars(35);
	EXPECT_EQ(this->mExpectedString.substr(5), view);
}

//------------------------------------------------------------------------------
// Char-by-char API
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldPeekCharWithoutAdvancing)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ABC");

	EXPECT_EQ(static_cast<TypeParam>(U'A'), this->mEncodedStreamReader->PeekChar());

	// Second peek returns same char
	const auto ch1 = this->mEncodedStreamReader->PeekChar();
	const auto ch2 = this->mEncodedStreamReader->PeekChar();
	EXPECT_EQ(ch1, ch2);

	// Position unchanged
	const auto posBefore = this->mEncodedStreamReader->GetPosition();
	(void)this->mEncodedStreamReader->PeekChar();
	EXPECT_EQ(posBefore, this->mEncodedStreamReader->GetPosition());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadCharByChar)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"ABC");

	EXPECT_EQ(static_cast<TypeParam>(U'A'), this->mEncodedStreamReader->ReadChar());
	EXPECT_EQ(static_cast<TypeParam>(U'B'), this->mEncodedStreamReader->ReadChar());
	EXPECT_EQ(static_cast<TypeParam>(U'C'), this->mEncodedStreamReader->ReadChar());
	EXPECT_FALSE(this->mEncodedStreamReader->ReadChar().has_value());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReadAllViaReadChar)
{
	constexpr size_t chunkSize = 32;
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Съешь ещё этих мягких французских булок, да выпей чаю"
		U"Широкая электрификация южных губерний даст мощный толчок подъёму сельского хозяйства.", chunkSize);
	EXPECT_EQ(this->mExpectedString, this->ReadAllCharByChar());
}

//------------------------------------------------------------------------------
// Large data (triggers TrimConsumedData)
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

TYPED_TEST(EncodedStreamReaderTest, ShouldCompactRawBufferOnTrim)
{
	constexpr size_t chunkSize = 32;
	std::u32string source;
	source.reserve(200);
	for (int i = 0; i < 200; ++i) {
		source += U'A' + (i % 52);
	}
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(source, chunkSize);

	const auto inputSize = this->mInputString.size();
	this->ReadAll();
	if constexpr (std::is_same_v<TypeParam, char>)
	{
		// Raw mode: no decode/trim path, raw buffer holds all input
		EXPECT_EQ(inputSize, ReaderAccess::GetRawBufferSize(*this->mEncodedStreamReader));
	}
	else
	{
		// Decoded mode: buffers are compacted on trim, stay bounded
		EXPECT_LE(ReaderAccess::GetRawBufferSize(*this->mEncodedStreamReader), chunkSize * 2);
		EXPECT_LE(ReaderAccess::GetDecodedBufferSize(*this->mEncodedStreamReader), chunkSize * 2);
	}
}

//------------------------------------------------------------------------------
// IsEnd tests
//------------------------------------------------------------------------------

TYPED_TEST(EncodedStreamReaderTest, ShouldReturnIsEndAfterFullRead)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Hello");
	EXPECT_FALSE(this->mEncodedStreamReader->IsEnd());
	this->ReadAll();
	EXPECT_TRUE(this->mEncodedStreamReader->IsEnd());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReturnIsEndFalseMidStream)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Hello World");
	this->mEncodedStreamReader->SkipChars(5);
	EXPECT_FALSE(this->mEncodedStreamReader->IsEnd());
}

TYPED_TEST(EncodedStreamReaderTest, ShouldReturnIsEndFalseBeforeAnyRead)
{
	this->template PrepareEncodedStreamReader<Convert::Utf::Utf8>(U"Hello");
	EXPECT_FALSE(this->mEncodedStreamReader->IsEnd());
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
