/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/auto_fixture.h"
#include "bitserializer/msgpack_archive.h"
#include "msgpack_reader_fixture.h"


using testing::Types;
typedef Types<BitSerializer::MsgPack::Detail::CMsgPackStringReader> Implementations;
//typedef Types<BitSerializer::MsgPack::Detail::CMsgPackStringWriter, BitSerializer::MsgPack::Detail::CMsgPackStreamWriter> Implementations;

// Tests for all implementations of IMsgPackWriter
TYPED_TEST_SUITE(MsgPackReaderTest, Implementations);

//------------------------------------------------------------------------------

TYPED_TEST(MsgPackReaderTest, ShouldReadNil)
{
	this->PrepareReader("\xC0");
	nullptr_t value;
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
}

TYPED_TEST(MsgPackReaderTest, ShouldReadBoolean)
{
	this->PrepareReader("\xC2\xC3");
	bool value = false;
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(true, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(false, value);
}

//-----------------------------------------------------------------------------
// Tests of reading integral values
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldReadInt8Fix)
{
	this->PrepareReader({ '\xE0', '\x09', '\x20', '\x7F' });
	int8_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-32, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(9, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(32, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(127, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadUInt8Fix)
{
	this->PrepareReader({ '\x00', '\x20', '\x7F' });
	uint8_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x20, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x7F, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadInt8)
{
	this->PrepareReader({
		'\xD0', '\x80',		// From int8
		'\xD0', '\xCE',		// From int8
		'\xD0', '\x7F',		// From int8
		});
	int8_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int8_t>::min(), value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-50, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int8_t>::max(), value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadUInt8)
{
	this->PrepareReader({
		'\xCC', '\x80',		// From uint8
		'\xCC', '\xC1',		// From uint8
		'\xCC', '\xFF'		// From uint8
		});
	uint8_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x80, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0xC1, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<uint8_t>::max(), value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadInt8FromLongerType)
{
	this->PrepareReader({
		'\xD1', '\xFF', '\xCE',														// From int16
		'\xD2', '\xFF', '\xFF', '\xFF', '\xCF',										// From int32
		'\xD3', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xD0',		// From int64
		'\xD1', '\x00', '\x6F',														// From int16
		'\xD2', '\x00', '\x00', '\x00', '\x7F',										// From int32
		'\xD3', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x5F'		// From int64
		});
	int8_t value = false;

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-50, value);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-49, value);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-48, value);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x6F, value);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x7F, value);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x5F, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadUInt8FromLongerType)
{
	this->PrepareReader({
		'\xCC', '\x20',																// From uint8
		'\xCD', '\x00', '\x6F',														// From uint16
		'\xCE', '\x00', '\x00', '\x00', '\xCF',										// From uint32
		'\xCF', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xFF'		// From uint64
		});
	uint8_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x20, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x6F, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0xCF, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0xFF, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadInt16)
{
	this->PrepareReader({
		'\xD1', '\x80', '\x00',		// From int16
		'\xD1', '\xA0','\x83',		// From int16
		'\xD1', '\x7F','\xFF'		// From int16
		});
	int16_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int16_t>::min(), value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-24445, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int16_t>::max(), value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadUInt16)
{
	this->PrepareReader({
		'\xCD', '\x40', '\x80',			// From uint16
		'\xCD', '\xCA', '\xFE',			// From uint16
		'\xCD', '\xFF', '\xFF'	 		// From uint16
		});
	uint16_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x4080, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0xCAFE, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<uint16_t>::max(), value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadInt16FromLongerType)
{
	this->PrepareReader({
		'\xD2', '\xFF', '\xFF', '\xFF', '\xCF',										// From int32
		'\xD3', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xD0',		// From int64
		'\xCE', '\x00', '\x00', '\x00', '\x7F',										// From uint32
		'\xCF', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x10', '\x5F'		// From uint64
		});
	int16_t value = false;

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-49, value);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-48, value);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x7F, value);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x105F, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadUInt16FromLongerType)
{
	this->PrepareReader({
		'\xD2', '\x00', '\x00', '\x00', '\x6F',										// From int32
		'\xD3', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x6F',		// From int64
		'\xCE', '\x00', '\x00', '\x00', '\xCF',										// From uint32
		'\xCF', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xFA', '\xAF'		// From uint64
		});
	uint16_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x6F, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x6F, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0xCF, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0xFAAF, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadInt32)
{
	this->PrepareReader({
		'\xD2', '\x80', '\x00', '\x00', '\x00',		// From int32
		'\xD2', '\xA0','\x83', '\x00', '\x00',		// From int32
		'\xD2', '\x7F','\xFF', '\xFF', '\xFF'		// From int32
		});
	int32_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int32_t>::min(), value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-1602027520, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int32_t>::max(), value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadUInt32)
{
	this->PrepareReader({
		'\xCE', '\x40', '\x80', '\x20', '\x10',		// From uint32
		'\xCE', '\xCA', '\xFE', '\x20', '\x30',		// From uint32
		'\xCE', '\xFF', '\xFF', '\xFF', '\xFF' 		// From uint32
		});
	uint32_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x40802010, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0xCAFE2030, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<uint32_t>::max(), value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadInt32FromLongerType)
{
	this->PrepareReader({
		'\xD3', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xD0',		// From int64
		'\xCF', '\x00', '\x00', '\x00', '\x00', '\x00', '\x50', '\x90', '\x7F'		// From uint64
		});
	int32_t value = false;

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-48, value);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x50907F, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadUInt32FromLongerType)
{
	this->PrepareReader({
		'\xD3', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x80', '\x6F',		// From int64
		'\xCF', '\x00', '\x00', '\x00', '\x00', '\x00', '\x70', '\x80', '\xCF'		// From uint64
		});
	uint32_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x806F, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0x7080CF, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadInt64)
{
	this->PrepareReader({
		'\xD3', '\x80', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',		// From int64
		'\xD3', '\x7F','\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF'		// From int64
		});
	int64_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int64_t>::min(), value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int64_t>::max(), value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadUInt64)
{
	this->PrepareReader({
		'\xCF', '\xA0', '\x90', '\x80', '\x70', '\x60', '\x50', '\x40', '\x30',		// From uint64
		'\xCF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF' 		// From uint64
		});
	uint64_t value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0xA090807060504030ul, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<uint64_t>::max(), value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadIntFromBoolean)
{
	this->PrepareReader({ '\xC2', '\xC3' } );
	int value = 100;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(1, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0, value);
}

//-----------------------------------------------------------------------------
// Tests of reading floating types
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldReadFloat)
{
	this->PrepareReader("\xCA\x40\x48\xF5\xC3");
	float value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_FLOAT_EQ(3.14f, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadDouble)
{
	this->PrepareReader("\xCB\x40\x09\x21\xFB\x54\x52\x45\x50");
	double value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_DOUBLE_EQ(3.141592654, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadDoubleFromFloat)
{
	this->PrepareReader("\xCA\x40\x48\xF5\xC3");
	double value = false;

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_FLOAT_EQ(3.14f, static_cast<float>(value));		// Cast back to float for able to compare
}

//-----------------------------------------------------------------------------
// Tests of reading strings
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldReadStringWhenSizeLessThan32)
{
	const auto expectedStr = this->GenTestString(31);
	this->PrepareReader(std::string({ static_cast<char>(static_cast<uint8_t>(expectedStr.size()) | 0b10100000) }) + expectedStr);

	std::string_view actualStr;
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(actualStr));
	EXPECT_EQ(expectedStr, actualStr);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadStringWhenEmptySize)
{
	this->PrepareReader(std::string({ static_cast<char>(0b10100000) }));

	std::string_view actualStr = "123";
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(actualStr));
	EXPECT_EQ("", actualStr);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadStringWhenSizeFitToUint8)
{
	const auto expectedStr = this->GenTestString(std::numeric_limits<uint8_t>::max());
	this->PrepareReader(std::string({ '\xD9', '\xFF' }) + expectedStr);

	std::string_view actualStr;
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(actualStr));
	EXPECT_EQ(expectedStr, actualStr);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadStringWhenSizeFitToUint16)
{
	const auto expectedStr = this->GenTestString(std::numeric_limits<uint16_t>::max());
	this->PrepareReader(std::string({ '\xDA', '\xFF', '\xFF' }) + expectedStr);

	std::string_view actualStr;
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(actualStr));
	ASSERT_EQ(expectedStr.size(), actualStr.size());
	EXPECT_EQ(std::hash<std::string>()(expectedStr), std::hash<std::string_view>()(actualStr));
}

TYPED_TEST(MsgPackReaderTest, ShouldReadStringWhenSizeFitToUint32)
{
	const auto expectedStr = this->GenTestString(std::numeric_limits<uint16_t>::max() + 3);
	this->PrepareReader(std::string({ '\xDB', '\x00', '\x01', '\x00', '\x02' }) + expectedStr);

	std::string_view actualStr;
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(actualStr));
	ASSERT_EQ(expectedStr.size(), actualStr.size());
	EXPECT_EQ(std::hash<std::string>()(expectedStr), std::hash<std::string_view>()(actualStr));
}

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenUnexpectedEndOfString)
{
	std::string_view actualStr;

	this->PrepareReader(std::string({ static_cast<char>(0b10100001) }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(actualStr), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xD9', '\x02', '1' }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(actualStr), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xDA', '\x01', '\x00', '1' }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(actualStr), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xDB', '\x01', '\x00', '\x00', '\x00', '1' }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(actualStr), BitSerializer::ParsingException);
}

//-----------------------------------------------------------------------------
// Tests of reading arrays
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldReadArrayWhenSizeLessThan16)
{
	this->PrepareReader({ static_cast<char>(0b10011111) });
	size_t size = 0;

	EXPECT_TRUE(this->mMsgPackReader->ReadArraySize(size));
	EXPECT_EQ(15, size);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadArrayWithEmptySize)
{
	this->PrepareReader({ static_cast<char>(0b10010000) });
	size_t size = 100;

	ASSERT_TRUE(this->mMsgPackReader->ReadArraySize(size));
	EXPECT_EQ(0, size);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadArrayWhenSizeFitToUint16)
{
	this->PrepareReader({
		'\xDC', '\xFF', '\xFF',
		'\xDC', '\x00', '\x00',
		'\xDC', '\x40', '\x00'
		});
	size_t size = 0;

	ASSERT_TRUE(this->mMsgPackReader->ReadArraySize(size));
	EXPECT_EQ(std::numeric_limits<uint16_t>::max(), size);

	ASSERT_TRUE(this->mMsgPackReader->ReadArraySize(size));
	EXPECT_EQ(std::numeric_limits<uint16_t>::min(), size);

	ASSERT_TRUE(this->mMsgPackReader->ReadArraySize(size));
	EXPECT_EQ(0x4000, size);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadArrayWhenSizeFitToUint32)
{
	this->PrepareReader({
		'\xDD', '\xFF', '\xFF', '\xFF', '\xFF',
		'\xDD', '\x00', '\x00', '\x00', '\x00',
		'\xDD', '\x40', '\x30', '\x20', '\x10'
		});
	size_t size = 0;

	ASSERT_TRUE(this->mMsgPackReader->ReadArraySize(size));
	EXPECT_EQ(std::numeric_limits<uint32_t>::max(), size);

	ASSERT_TRUE(this->mMsgPackReader->ReadArraySize(size));
	EXPECT_EQ(std::numeric_limits<uint32_t>::min(), size);

	ASSERT_TRUE(this->mMsgPackReader->ReadArraySize(size));
	EXPECT_EQ(0x40302010, size);
}

//-----------------------------------------------------------------------------
// Tests of reading maps
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldReadMapWhenSizeLessThan16)
{
	this->PrepareReader({ static_cast<char>(0b10001111) });
	size_t size = 0;

	EXPECT_TRUE(this->mMsgPackReader->ReadMapSize(size));
	EXPECT_EQ(15, size);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadMapWithEmptySize)
{
	this->PrepareReader({ static_cast<char>(0b10000000) });
	size_t size = 100;

	ASSERT_TRUE(this->mMsgPackReader->ReadMapSize(size));
	EXPECT_EQ(0, size);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadMapWhenSizeFitToUint16)
{
	this->PrepareReader({
		'\xDE', '\xFF', '\xFF',
		'\xDE', '\x00', '\x00',
		'\xDE', '\x40', '\x00'
		});
	size_t size = 0;

	ASSERT_TRUE(this->mMsgPackReader->ReadMapSize(size));
	EXPECT_EQ(std::numeric_limits<uint16_t>::max(), size);

	ASSERT_TRUE(this->mMsgPackReader->ReadMapSize(size));
	EXPECT_EQ(std::numeric_limits<uint16_t>::min(), size);

	ASSERT_TRUE(this->mMsgPackReader->ReadMapSize(size));
	EXPECT_EQ(0x4000, size);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadMapWhenSizeFitToUint32)
{
	this->PrepareReader({
		'\xDF', '\xFF', '\xFF', '\xFF', '\xFF',
		'\xDF', '\x00', '\x00', '\x00', '\x00',
		'\xDF', '\x40', '\x30', '\x20', '\x10'
		});
	size_t size = 0;

	ASSERT_TRUE(this->mMsgPackReader->ReadMapSize(size));
	EXPECT_EQ(std::numeric_limits<uint32_t>::max(), size);

	ASSERT_TRUE(this->mMsgPackReader->ReadMapSize(size));
	EXPECT_EQ(std::numeric_limits<uint32_t>::min(), size);

	ASSERT_TRUE(this->mMsgPackReader->ReadMapSize(size));
	EXPECT_EQ(0x40302010, size);
}

//-----------------------------------------------------------------------------
// Tests of get/set positions
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldGetPosition)
{
	this->PrepareReader("\xC2\xC3");
	bool value = false;
	EXPECT_EQ(0, this->mMsgPackReader->GetPosition());
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(1, this->mMsgPackReader->GetPosition());
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(2, this->mMsgPackReader->GetPosition());
}

TYPED_TEST(MsgPackReaderTest, ShouldSetPosition)
{
	this->PrepareReader("\xC2\xC3");
	bool value = false;
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	this->mMsgPackReader->SetPosition(0);
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(true, value);
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(false, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldCheckIsEnd)
{
	this->PrepareReader("\xC2\xC3");
	bool value = false;
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_FALSE(this->mMsgPackReader->IsEnd());
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_TRUE(this->mMsgPackReader->IsEnd());
}

//-----------------------------------------------------------------------------
// Tests for reading value types
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfFixedSignedInt)
{
	constexpr auto expectedType = BitSerializer::MsgPack::Detail::ValueType::SignedInteger;
	for (char i = -32; i < 0; ++i)
	{
		this->PrepareReader({ i });
		EXPECT_EQ(expectedType, this->mMsgPackReader->ReadValueType());
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfFixedUnsignedInt)
{
	constexpr auto expectedType = BitSerializer::MsgPack::Detail::ValueType::UnsignedInteger;
	for (char i = 0; i < 127; ++i)
	{
		this->PrepareReader({ i });
		EXPECT_EQ(expectedType, this->mMsgPackReader->ReadValueType());
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfUnsignedInt)
{
	constexpr auto expectedType = BitSerializer::MsgPack::Detail::ValueType::UnsignedInteger;
	for (const char testCode : { '\xCC', '\xCD', '\xCE', '\xCF' })
	{
		this->PrepareReader({ testCode });
		EXPECT_EQ(expectedType, this->mMsgPackReader->ReadValueType());
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfSignedInt)
{
	constexpr auto expectedType = BitSerializer::MsgPack::Detail::ValueType::SignedInteger;
	for (const char testCode : { '\xD0', '\xD1', '\xD2', '\xD3' })
	{
		this->PrepareReader({ testCode });
		EXPECT_EQ(expectedType, this->mMsgPackReader->ReadValueType());
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfBoolean)
{
	constexpr auto expectedType = BitSerializer::MsgPack::Detail::ValueType::Boolean;
	for (const char testCode : { '\xC2', '\xC3' })
	{
		this->PrepareReader({ testCode });
		EXPECT_EQ(expectedType, this->mMsgPackReader->ReadValueType());
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfNil)
{
	this->PrepareReader("\xC0");
	EXPECT_EQ(BitSerializer::MsgPack::Detail::ValueType::Nil, this->mMsgPackReader->ReadValueType());
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfFloat)
{
	this->PrepareReader("\xCA");
	EXPECT_EQ(BitSerializer::MsgPack::Detail::ValueType::Float, this->mMsgPackReader->ReadValueType());
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfDouble)
{
	this->PrepareReader("\xCB");
	EXPECT_EQ(BitSerializer::MsgPack::Detail::ValueType::Double, this->mMsgPackReader->ReadValueType());
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfString)
{
	constexpr auto expectedType = BitSerializer::MsgPack::Detail::ValueType::String;
	for (const char testCode : { '\xD9', '\xDA', '\xDB' })
	{
		this->PrepareReader({ testCode });
		EXPECT_EQ(expectedType, this->mMsgPackReader->ReadValueType());
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfFixedString)
{
	constexpr auto expectedType = BitSerializer::MsgPack::Detail::ValueType::String;
	for (uint8_t i = 0; i < 0b00011111; ++i)
	{
		this->PrepareReader({ static_cast<char>(i | 0b10100000) });
		EXPECT_EQ(expectedType, this->mMsgPackReader->ReadValueType());
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfMap)
{
	constexpr auto expectedType = BitSerializer::MsgPack::Detail::ValueType::Map;
	for (const char testCode : { '\xDE', '\xDF' })
	{
		this->PrepareReader({ testCode });
		EXPECT_EQ(expectedType, this->mMsgPackReader->ReadValueType());
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfFixMap)
{
	constexpr auto expectedType = BitSerializer::MsgPack::Detail::ValueType::Map;
	for (uint8_t i = 0; i < 0b00001111; ++i)
	{
		this->PrepareReader({ static_cast<char>(i | 0b10000000) });
		EXPECT_EQ(expectedType, this->mMsgPackReader->ReadValueType());
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfArray)
{
	constexpr auto expectedType = BitSerializer::MsgPack::Detail::ValueType::Array;
	for (const char testCode : { '\xDC', '\xDD' })
	{
		this->PrepareReader({ testCode });
		EXPECT_EQ(expectedType, this->mMsgPackReader->ReadValueType());
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfFixArray)
{
	constexpr auto expectedType = BitSerializer::MsgPack::Detail::ValueType::Array;
	for (uint8_t i = 0; i < 0b00001111; ++i)
	{
		this->PrepareReader({ static_cast<char>(i | 0b10010000) });
		EXPECT_EQ(expectedType, this->mMsgPackReader->ReadValueType());
	}
}
