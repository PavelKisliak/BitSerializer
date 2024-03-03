/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <array>
#include "testing_tools/auto_fixture.h"
#include "bitserializer/msgpack_archive.h"
#include "msgpack_writer_fixture.h"


using testing::Types;
typedef Types<BitSerializer::MsgPack::Detail::CMsgPackStringWriter, BitSerializer::MsgPack::Detail::CMsgPackStreamWriter> Implementations;

// Tests for all implementations of IMsgPackWriter
TYPED_TEST_SUITE(MsgPackWriterTest, Implementations);

//------------------------------------------------------------------------------

TYPED_TEST(MsgPackWriterTest, ShouldWriteBoolean)
{
	this->mMsgPackWriter->WriteValue(false);
	EXPECT_EQ("\xC2", this->TakeResult());

	this->mMsgPackWriter->WriteValue(true);
	EXPECT_EQ("\xC3", this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteNil)
{
	this->mMsgPackWriter->WriteValue(nullptr);
	EXPECT_EQ("\xC0", this->TakeResult());
}

//-----------------------------------------------------------------------------
// Tests of writing integral values
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackWriterTest, ShouldWriteUInt8)
{
	this->mMsgPackWriter->WriteValue(std::numeric_limits<uint8_t>::min());
	EXPECT_EQ(std::string(1, '\x00'), this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<uint8_t>(0x7f));
	EXPECT_EQ("\x7F", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<uint8_t>(0x80));
	EXPECT_EQ("\xCC\x80", this->TakeResult());

	this->mMsgPackWriter->WriteValue(std::numeric_limits<uint8_t>::max());
	EXPECT_EQ("\xCC\xFF", this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteUInt16)
{
	this->mMsgPackWriter->WriteValue(static_cast<uint16_t>(0x20));
	EXPECT_EQ("\x20", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<uint16_t>(0xcafe));
	EXPECT_EQ("\xCD\xCA\xFE", this->TakeResult());

	this->mMsgPackWriter->WriteValue(std::numeric_limits<uint16_t>::max());
	EXPECT_EQ("\xCD\xFF\xFF", this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteUInt32)
{
	this->mMsgPackWriter->WriteValue(static_cast<uint32_t>(0x45));
	EXPECT_EQ("\x45", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<uint32_t>(0xcafe));
	EXPECT_EQ("\xCD\xCA\xFE", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<uint32_t>(0xcafe1230));
	EXPECT_EQ("\xCE\xCA\xFE\x12\x30", this->TakeResult());

	this->mMsgPackWriter->WriteValue(std::numeric_limits<uint32_t>::max());
	EXPECT_EQ("\xCE\xFF\xFF\xFF\xFF", this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteUInt64)
{
	this->mMsgPackWriter->WriteValue(static_cast<uint64_t>(0x45));
	EXPECT_EQ("\x45", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<uint64_t>(0xcafe));
	EXPECT_EQ("\xCD\xCA\xFE", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<uint64_t>(0xcafe0830));
	EXPECT_EQ("\xCE\xCA\xFE\x08\x30", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<uint64_t>(0xcafe1230cafe1830));
	EXPECT_EQ("\xCF\xCA\xFE\x12\x30\xCA\xFE\x18\x30", this->TakeResult());

	this->mMsgPackWriter->WriteValue(std::numeric_limits<uint64_t>::max());
	EXPECT_EQ("\xCF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteInt8)
{
	this->mMsgPackWriter->WriteValue(std::numeric_limits<int8_t>::min());
	EXPECT_EQ("\xD0\x80", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<int8_t>(-127));
	EXPECT_EQ("\xD0\x81", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<int8_t>(-33));
	EXPECT_EQ("\xD0\xDF", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<int8_t>(-32));
	EXPECT_EQ("\xE0", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<int8_t>(0));
	EXPECT_EQ(std::string(1, '\x00'), this->TakeResult());

	this->mMsgPackWriter->WriteValue(std::numeric_limits<int8_t>::max());
	EXPECT_EQ("\x7F", this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteInt16)
{
	this->mMsgPackWriter->WriteValue(static_cast<int16_t>(-16));
	EXPECT_EQ("\xF0", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<int16_t>(0x1045));
	EXPECT_EQ("\xD1\x10\x45", this->TakeResult());

	this->mMsgPackWriter->WriteValue(std::numeric_limits<int16_t>::min());
	EXPECT_EQ(std::string({ '\xD1', '\x80', '\x00' }), this->TakeResult());

	this->mMsgPackWriter->WriteValue(std::numeric_limits<int16_t>::max());
	EXPECT_EQ("\xD1\x7F\xFF", this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteInt32)
{
	this->mMsgPackWriter->WriteValue(static_cast<int16_t>(-21));
	EXPECT_EQ("\xEB", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<int16_t>(0x7055));
	EXPECT_EQ("\xD1\x70\x55", this->TakeResult());

	this->mMsgPackWriter->WriteValue(std::numeric_limits<int32_t>::min());
	EXPECT_EQ(std::string({ '\xD2', '\x80', '\x00', '\x00', '\x00' }), this->TakeResult());

	this->mMsgPackWriter->WriteValue(std::numeric_limits<int32_t>::max());
	EXPECT_EQ("\xD2\x7F\xFF\xFF\xFF", this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteInt64)
{
	this->mMsgPackWriter->WriteValue(static_cast<int64_t>(-21));
	EXPECT_EQ("\xEB", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<int64_t>(0x7055));
	EXPECT_EQ("\xD1\x70\x55", this->TakeResult());

	this->mMsgPackWriter->WriteValue(static_cast<int64_t>(0x60807090));
	EXPECT_EQ("\xD2\x60\x80\x70\x90", this->TakeResult());

	this->mMsgPackWriter->WriteValue(std::numeric_limits<int64_t>::min());
	EXPECT_EQ(std::string({ '\xD3', '\x80', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00' }), this->TakeResult());

	this->mMsgPackWriter->WriteValue(std::numeric_limits<int64_t>::max());
	EXPECT_EQ("\xD3\x7F\xFF\xFF\xFF\xFF\xFF\xFF\xFF", this->TakeResult());
}

//-----------------------------------------------------------------------------
// Tests of writing floating types
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackWriterTest, ShouldWriteFloat)
{
	this->mMsgPackWriter->WriteValue(3.14f);
	EXPECT_EQ("\xCA\x40\x48\xF5\xC3", this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteDouble)
{
	this->mMsgPackWriter->WriteValue(3.141592654);
	EXPECT_EQ("\xCB\x40\x09\x21\xFB\x54\x52\x45\x50", this->TakeResult());
}

//-----------------------------------------------------------------------------
// Tests of writing strings
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackWriterTest, ShouldWriteStringEmptySize)
{
	this->mMsgPackWriter->WriteValue("");
	const std::string expectedStr = { static_cast<char>(0b10100000) };
	EXPECT_EQ(expectedStr, this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteStringWhenSizeLessThan32)
{
	const auto testStr = this->GenTestString(31);
	this->mMsgPackWriter->WriteValue(testStr);
	std::string expectedStr = { static_cast<char>(static_cast<uint8_t>(testStr.size()) | 0b10100000) };
	expectedStr += testStr;
	EXPECT_EQ(expectedStr, this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteStringWhenSizeFitToUint8)
{
	const std::string testStr1 = this->GenTestString(32);
	const auto expectedStr1 = std::string({ '\xD9', static_cast<char>(testStr1.size()) }) + testStr1;
	this->mMsgPackWriter->WriteValue(testStr1);
	EXPECT_EQ(expectedStr1, this->TakeResult());

	const std::string testStr2 = this->GenTestString(std::numeric_limits<uint8_t>::max());
	const auto expectedStr2 = std::string({ '\xD9', static_cast<char>(testStr2.size()) }) + testStr2;
	this->mMsgPackWriter->WriteValue(testStr2);
	EXPECT_EQ(expectedStr2, this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteStringWhenSizeFitToUint16)
{
	constexpr size_t checkSampleSize = 10;
	const std::string testStr = this->GenTestString(std::numeric_limits<uint16_t>::max());
	const auto expectedStr = std::string({ '\xDA', '\xFF', '\xFF' }) + testStr.substr(0, checkSampleSize - 3);
	this->mMsgPackWriter->WriteValue(testStr);
	const auto result = this->TakeResult();
	ASSERT_EQ(testStr.size() + 3, result.size());
	EXPECT_EQ(expectedStr, result.substr(0, checkSampleSize));
	EXPECT_EQ(testStr[testStr.size() - 1], result[result.size() - 1]);
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteStringWhenSizeFitToUint32)
{
	constexpr size_t checkSampleSize = 15;
	const std::string testStr = this->GenTestString(std::numeric_limits<uint16_t>::max() + 3);
	const auto expectedStr = std::string({ '\xDB', '\x00', '\x01', '\x00', '\x02' }) + testStr.substr(0, checkSampleSize - 5);
	this->mMsgPackWriter->WriteValue(testStr);
	const auto result = this->TakeResult();
	ASSERT_EQ(testStr.size() + 5, result.size());
	EXPECT_EQ(expectedStr, result.substr(0, checkSampleSize));
	EXPECT_EQ(testStr[testStr.size() - 1], result[result.size() - 1]);
}

//-----------------------------------------------------------------------------
// Tests of writing arrays
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackWriterTest, ShouldWriteArrayWhenEmptySize)
{
	constexpr size_t arrSize = 0;
	std::string expectedStr = { static_cast<char>(static_cast<uint8_t>(arrSize) | 0b10010000) };
	this->mMsgPackWriter->BeginArray(arrSize);
	EXPECT_EQ(expectedStr, this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteArrayWhenSizeLessThan16)
{
	constexpr size_t arrSize = 15;
	std::string expectedStr = { static_cast<char>(static_cast<uint8_t>(arrSize) | 0b10010000) };
	this->mMsgPackWriter->BeginArray(arrSize);
	EXPECT_EQ(expectedStr, this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteArrayWhenSizeFitToUint16)
{
	constexpr size_t arrSize = std::numeric_limits<uint16_t>::max();
	std::string expectedStr = { '\xDC', '\xFF', '\xFF' };
	this->mMsgPackWriter->BeginArray(arrSize);
	EXPECT_EQ(expectedStr, this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteArrayWhenSizeFitToUint32)
{
	constexpr size_t arrSize = std::numeric_limits<uint16_t>::max() + 1;
	std::string expectedStr = { '\xDD', '\x00', '\x01', '\x00', '\x00' };
	this->mMsgPackWriter->BeginArray(arrSize);
	EXPECT_EQ(expectedStr, this->TakeResult());
}

//-----------------------------------------------------------------------------
// Tests of writing binary arrays
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackWriterTest, ShouldWriteBinaryArrayWithEmptySize)
{
	this->mMsgPackWriter->BeginBinary(0);
	const std::string expectedStr = { '\xC4', '\x0' };
	EXPECT_EQ(expectedStr, this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteBinaryArrayWhenSizeFitToUint8)
{
	const std::string testStr = this->GenTestString(std::numeric_limits<uint8_t>::max());
	const auto expectedStr1 = std::string({ '\xC4', static_cast<char>(testStr.size()) }) + testStr;
	this->mMsgPackWriter->BeginBinary(testStr.size());
	for (char ch : testStr) {
		this->mMsgPackWriter->WriteBinary(ch);
	}
	EXPECT_EQ(expectedStr1, this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteBinaryArrayWhenSizeFitToUint16)
{
	const std::string testStr = this->GenTestString(std::numeric_limits<uint16_t>::max());
	const auto expectedStr = std::string({ '\xC5', '\xFF', '\xFF' }) + testStr;
	this->mMsgPackWriter->BeginBinary(testStr.size());
	for (char ch : testStr) {
		this->mMsgPackWriter->WriteBinary(ch);
	}
	const auto result = this->TakeResult();
	EXPECT_EQ(expectedStr, result);
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteBinaryArrayWhenSizeFitToUint32)
{
	const std::string testStr = this->GenTestString(std::numeric_limits<uint16_t>::max() + 3);
	const auto expectedStr = std::string({ '\xC6', '\x00', '\x01', '\x00', '\x02' }) + testStr;
	this->mMsgPackWriter->BeginBinary(testStr.size());
	for (char ch : testStr) {
		this->mMsgPackWriter->WriteBinary(ch);
	}
	const auto result = this->TakeResult();
	EXPECT_EQ(expectedStr, result);
}

//-----------------------------------------------------------------------------
// Tests of writing maps
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackWriterTest, ShouldWriteMapWhenEmptySize)
{
	constexpr size_t mapSize = 0;
	std::string expectedStr = { static_cast<char>(static_cast<uint8_t>(mapSize) | 0b10000000) };
	this->mMsgPackWriter->BeginMap(mapSize);
	EXPECT_EQ(expectedStr, this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteMapWhenSizeLessThan16)
{
	constexpr size_t mapSize = 15;
	std::string expectedStr = { static_cast<char>(static_cast<uint8_t>(mapSize) | 0b10000000) };
	this->mMsgPackWriter->BeginMap(mapSize);
	EXPECT_EQ(expectedStr, this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteMapWhenSizeFitToUint16)
{
	constexpr size_t mapSize = std::numeric_limits<uint16_t>::max();
	std::string expectedStr = { '\xDE', '\xFF', '\xFF' };
	this->mMsgPackWriter->BeginMap(mapSize);
	EXPECT_EQ(expectedStr, this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteMapWhenSizeFitToUint32)
{
	constexpr size_t mapSize = std::numeric_limits<uint16_t>::max() + 1;
	std::string expectedStr = { '\xDF', '\x00', '\x01', '\x00', '\x00' };
	this->mMsgPackWriter->BeginMap(mapSize);
	EXPECT_EQ(expectedStr, this->TakeResult());
}

//-----------------------------------------------------------------------------
// Tests of writing timestamps
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackWriterTest, ShouldWriteTimestamp32)
{
	BitSerializer::Detail::CBinTimestamp timeSpec(0x8090A0B0);
	this->mMsgPackWriter->WriteValue(timeSpec);
	EXPECT_EQ("\xD6\xFF\x80\x90\xA0\xB0", this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteTimestamp64)
{
	BitSerializer::Detail::CBinTimestamp timeSpec(0x10203040, 0x01020304);
	this->mMsgPackWriter->WriteValue(timeSpec);
	EXPECT_EQ("\xD7\xFF\x04\x08\x0C\x10\x10\x20\x30\x40", this->TakeResult());
}

TYPED_TEST(MsgPackWriterTest, ShouldWriteTimestamp96)
{
	BitSerializer::Detail::CBinTimestamp timeSpec(0x0102030405060708, 0x090A0B0C);
	this->mMsgPackWriter->WriteValue(timeSpec);
	EXPECT_EQ("\xC7\x0C\xFF\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C", this->TakeResult());
}
