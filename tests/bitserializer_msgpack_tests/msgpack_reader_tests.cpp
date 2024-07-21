/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/auto_fixture.h"
#include "bitserializer/msgpack_archive.h"
#include "msgpack_reader_fixture.h"


using testing::Types;
typedef Types<BitSerializer::MsgPack::Detail::CMsgPackStringReader, BitSerializer::MsgPack::Detail::CMsgPackStreamReader> Implementations;

// Tests for all implementations of IMsgPackWriter
TYPED_TEST_SUITE(MsgPackReaderTest, Implementations);

//------------------------------------------------------------------------------

namespace
{
	template <typename T, std::enable_if_t<sizeof(T) == 1 && std::is_integral_v<T>, int> = 0>
	std::string EncodeIntegralValue(uint8_t code, T value)
	{
		std::string outStr;
		outStr.push_back(static_cast<char>(code));
		outStr.push_back(static_cast<char>(value));
		return outStr;
	}

	template <typename T, std::enable_if_t<sizeof(T) >= 2 && std::is_integral_v<T>, int> = 0>
	std::string EncodeIntegralValue(uint8_t code, T value)
	{
		std::string outStr;
		outStr.push_back(static_cast<char>(code));
		const auto networkVal = BitSerializer::Memory::NativeToBigEndian(value);
		outStr.append(reinterpret_cast<const char*>(&networkVal), sizeof(T));
		return outStr;
	}
}


TYPED_TEST(MsgPackReaderTest, ShouldReadNil)
{
	this->PrepareReader("\xC0");
	std::nullptr_t value;
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
}

TYPED_TEST(MsgPackReaderTest, ShouldReadBoolean)
{
	this->PrepareReader("\xC3\xC2");
	bool value = false;
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(true, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(false, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenReadBooleanToWrongType)
{
	this->PrepareReader("\xC3");
	std::string_view wrongType;
	EXPECT_THROW(this->mMsgPackReader->ReadValue(wrongType), BitSerializer::SerializationException);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipBooleanWhenPolicyIsSkip)
{
	this->PrepareReader("\xC3", BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	std::string_view wrongType;
	EXPECT_FALSE(this->mMsgPackReader->ReadValue(wrongType));
	EXPECT_TRUE(wrongType.empty());
	EXPECT_TRUE(this->mMsgPackReader->IsEnd()) << "Value should be skipped";
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
		'\xD1', '\xA0', '\x83',		// From int16
		'\xD1', '\x7F', '\xFF'		// From int16
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
		'\xD2', '\xA0', '\x83', '\x00', '\x00',		// From int32
		'\xD2', '\x7F', '\xFF', '\xFF', '\xFF'		// From int32
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
		'\xD3', '\x7F', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF'		// From int64
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
	EXPECT_EQ(0, value);

	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(1, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenReadInt32ToWrongType)
{
	this->PrepareReader({	'\xD2', '\x80', '\x00', '\x00', '\x00' });
	std::string_view wrongType;
	EXPECT_THROW(this->mMsgPackReader->ReadValue(wrongType), BitSerializer::SerializationException);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipInt32WhenPolicyIsSkip)
{
	this->PrepareReader({ '\xD2', '\x80', '\x00', '\x00', '\x00' },
		BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	std::string_view wrongType;
	EXPECT_FALSE(this->mMsgPackReader->ReadValue(wrongType));
	EXPECT_TRUE(wrongType.empty());
	EXPECT_TRUE(this->mMsgPackReader->IsEnd()) << "Value should be skipped";
}

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenOverflowInt16)
{
	this->PrepareReader({ '\xD2', '\x80', '\x00', '\x00', '\x00' });
	int16_t shortInt;
	EXPECT_THROW(this->mMsgPackReader->ReadValue(shortInt), BitSerializer::SerializationException);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipInt16OverflowWhenPolicyIsSkip)
{
	this->PrepareReader({ '\xD2', '\x80', '\x00', '\x00', '\x00' }, BitSerializer::OverflowNumberPolicy::Skip);
	int16_t shortInt = 0x1020;
	EXPECT_FALSE(this->mMsgPackReader->ReadValue(shortInt));
	EXPECT_EQ(0x1020, shortInt);
	EXPECT_TRUE(this->mMsgPackReader->IsEnd()) << "Value should be skipped";
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

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenReadFloatToWrongType)
{
	this->PrepareReader("\xCA\x40\x48\xF5\xC3");
	std::string_view wrongType;
	EXPECT_THROW(this->mMsgPackReader->ReadValue(wrongType), BitSerializer::SerializationException);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipFloatWhenPolicyIsSkip)
{
	this->PrepareReader("\xCA\x40\x48\xF5\xC3",
		BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	std::string_view wrongType;
	EXPECT_FALSE(this->mMsgPackReader->ReadValue(wrongType));
	EXPECT_TRUE(wrongType.empty());
	EXPECT_TRUE(this->mMsgPackReader->IsEnd()) << "Value should be skipped";
}

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenReadDoubleToWrongType)
{
	this->PrepareReader("\xCB\x40\x09\x21\xFB\x54\x52\x45\x50");
	std::string_view wrongType;
	EXPECT_THROW(this->mMsgPackReader->ReadValue(wrongType), BitSerializer::SerializationException);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipDoubleWhenPolicyIsSkip)
{
	this->PrepareReader("\xCB\x40\x09\x21\xFB\x54\x52\x45\x50",
		BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	std::string_view wrongType;
	EXPECT_FALSE(this->mMsgPackReader->ReadValue(wrongType));
	EXPECT_TRUE(wrongType.empty());
	EXPECT_TRUE(this->mMsgPackReader->IsEnd()) << "Value should be skipped";
}

//-----------------------------------------------------------------------------
// Tests of reading strings
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldReadStringWithFixedSize)
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

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenReadStringToWrongType)
{
	const auto testStr = this->GenTestString(std::numeric_limits<uint8_t>::max());
	this->PrepareReader(std::string({ '\xD9', '\xFF' }) + testStr);
	bool wrongType;
	EXPECT_THROW(this->mMsgPackReader->ReadValue(wrongType), BitSerializer::SerializationException);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipStringWhenPolicyIsSkip)
{
	const auto testStr = this->GenTestString(std::numeric_limits<uint8_t>::max());
	this->PrepareReader(std::string({ '\xD9', '\xFF' }) + testStr,
		BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	int wrongType = 0x70605040;
	EXPECT_FALSE(this->mMsgPackReader->ReadValue(wrongType));
	EXPECT_EQ(0x70605040, wrongType);
	EXPECT_TRUE(this->mMsgPackReader->IsEnd()) << "Value should be skipped";
}

//-----------------------------------------------------------------------------
// Tests of reading arrays
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldReadArrayWithFixedSize)
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

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenReadFixedArrayToWrongType)
{
	this->PrepareReader({static_cast<char>(0b10010010), '\xC2', '\xC3' });
	bool wrongType;
	EXPECT_THROW(this->mMsgPackReader->ReadValue(wrongType), BitSerializer::SerializationException);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipFixedArrayWhenPolicyIsSkip)
{
	this->PrepareReader({ static_cast<char>(0b10010010), '\xC2', '\xC3' },
		BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	int wrongType = 0x70605040;
	EXPECT_FALSE(this->mMsgPackReader->ReadValue(wrongType));
	EXPECT_EQ(0x70605040, wrongType);
	EXPECT_TRUE(this->mMsgPackReader->IsEnd()) << "Value should be skipped";
}

//-----------------------------------------------------------------------------
// Tests of reading binary arrays
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldReadBinaryArrayWhenEmptySize)
{
	this->PrepareReader({ '\xC4', '\x0' });

	size_t actualSize = 3;
	EXPECT_TRUE(this->mMsgPackReader->ReadBinarySize(actualSize));
	EXPECT_EQ(0, actualSize);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadBinaryArrayWhenSizeFitToUint8)
{
	const auto expectedStr = this->GenTestString(std::numeric_limits<uint8_t>::max());
	this->PrepareReader(std::string({ '\xC4', '\xFF' }) + expectedStr);

	size_t actualSize = 3;
	std::string actualStr;
	EXPECT_TRUE(this->mMsgPackReader->ReadBinarySize(actualSize));
	EXPECT_EQ(expectedStr.size(), actualSize);
	for (size_t i = 0; i < expectedStr.size(); ++i)
	{
		actualStr.push_back(this->mMsgPackReader->ReadBinary());
	}
	ASSERT_EQ(expectedStr.size(), actualStr.size());
	EXPECT_EQ(expectedStr, actualStr);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadBinaryArrayWhenSizeFitToUint16)
{
	const auto expectedStr = this->GenTestString(std::numeric_limits<uint16_t>::max());
	this->PrepareReader(std::string({ '\xC5', '\xFF', '\xFF' }) + expectedStr);

	size_t actualSize = 3;
	std::string actualStr;
	EXPECT_TRUE(this->mMsgPackReader->ReadBinarySize(actualSize));
	EXPECT_EQ(expectedStr.size(), actualSize);
	for (size_t i = 0; i < expectedStr.size(); ++i)
	{
		actualStr.push_back(this->mMsgPackReader->ReadBinary());
	}
	ASSERT_EQ(expectedStr.size(), actualStr.size());
	EXPECT_EQ(expectedStr, actualStr);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadBinaryArrayWhenSizeFitToUint32)
{
	const auto expectedStr = this->GenTestString(std::numeric_limits<uint16_t>::max() + 3);
	this->PrepareReader(std::string({ '\xC6', '\x00', '\x01', '\x00', '\x02' }) + expectedStr);

	size_t actualSize = 3;
	std::string actualStr;
	EXPECT_TRUE(this->mMsgPackReader->ReadBinarySize(actualSize));
	EXPECT_EQ(expectedStr.size(), actualSize);
	for (size_t i = 0; i < expectedStr.size(); ++i)
	{
		actualStr.push_back(this->mMsgPackReader->ReadBinary());
	}
	ASSERT_EQ(expectedStr.size(), actualStr.size());
	EXPECT_EQ(expectedStr, actualStr);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadBinaryArrayWhenUnexpectedEnd)
{
	size_t actualSize = 3;

	this->PrepareReader(std::string({ '\xC4' }));
	EXPECT_THROW(this->mMsgPackReader->ReadBinarySize(actualSize), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xC4', '\x00' }));
	EXPECT_TRUE(this->mMsgPackReader->ReadBinarySize(actualSize));
	EXPECT_EQ(0, actualSize);
	EXPECT_THROW(this->mMsgPackReader->ReadBinary(), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xC5', '\x00', '\x00' }));
	EXPECT_TRUE(this->mMsgPackReader->ReadBinarySize(actualSize));
	EXPECT_EQ(0, actualSize);
	EXPECT_THROW(this->mMsgPackReader->ReadBinary(), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xC6', '\x00', '\x00', '\x00', '\x00' }));
	EXPECT_TRUE(this->mMsgPackReader->ReadBinarySize(actualSize));
	EXPECT_EQ(0, actualSize);
	EXPECT_THROW(this->mMsgPackReader->ReadBinary(), BitSerializer::ParsingException);
}

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenReadBinaryArrayToWrongType)
{
	this->PrepareReader({ '\xC4', '\x03', '\x01', '\x02', '\x03' });
	bool wrongType;
	EXPECT_THROW(this->mMsgPackReader->ReadValue(wrongType), BitSerializer::SerializationException);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipBinaryArrayWhenPolicyIsSkip)
{
	this->PrepareReader({ '\xC4', '\x03', '\x01', '\x02', '\x03' },
		BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	int wrongType = 0x70605040;
	EXPECT_FALSE(this->mMsgPackReader->ReadValue(wrongType));
	EXPECT_EQ(0x70605040, wrongType);
	EXPECT_TRUE(this->mMsgPackReader->IsEnd()) << "Value should be skipped";
}

//-----------------------------------------------------------------------------
// Tests of reading maps
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldReadMapWithFixedSize)
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

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenReadFixedMapToWrongType)
{
	this->PrepareReader({
		static_cast<char>(0b10000001), '\x01', '\x10'
	});
	bool wrongType;
	EXPECT_THROW(this->mMsgPackReader->ReadValue(wrongType), BitSerializer::SerializationException);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipFixedMapWhenPolicyIsSkip)
{
	this->PrepareReader({static_cast<char>(0b10000010), '\x01', '\x10', '\x02', '\x20' },
		BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	int wrongType = 0x70605040;
	EXPECT_FALSE(this->mMsgPackReader->ReadValue(wrongType));
	EXPECT_EQ(0x70605040, wrongType);
	EXPECT_TRUE(this->mMsgPackReader->IsEnd()) << "Value should be skipped";
}

//-----------------------------------------------------------------------------
// Tests of reading timestamps
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldReadTimestamp32)
{
	this->PrepareReader({
		'\xD6', '\xFF', '\x10', '\x20', '\x30', '\x40',
		'\xD6', '\xFF', '\x80', '\x90', '\xA0', '\xB0'
		});
	BitSerializer::Detail::CBinTimestamp timestamp;

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(timestamp));
	EXPECT_EQ(0x10203040, timestamp.Seconds);
	EXPECT_EQ(0, timestamp.Nanoseconds);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(timestamp));
	EXPECT_EQ(0x8090A0B0, timestamp.Seconds);
	EXPECT_EQ(0, timestamp.Nanoseconds);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTimestamp64)
{
	this->PrepareReader({
		'\xD7', '\xFF', '\x04', '\x08', '\x0C', '\x10', '\x10', '\x20', '\x30', '\x40',
		'\xD7', '\xFF', '\x05', '\x09', '\x0D', '\x20', '\x20', '\x30', '\x40', '\x50'
		});
	BitSerializer::Detail::CBinTimestamp timestamp;

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(timestamp));
	EXPECT_EQ(0x10203040, timestamp.Seconds);
	EXPECT_EQ(0x01020304, timestamp.Nanoseconds);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(timestamp));
	EXPECT_EQ(0x20304050, timestamp.Seconds);
	EXPECT_EQ(0x01424348, timestamp.Nanoseconds);
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTimestamp96)
{
	this->PrepareReader({
		'\xC7', '\x0C', '\xFF', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\x09', '\x0A', '\x0B', '\x0C',
		'\xC7', '\x0C', '\xFF', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1A', '\x1B', '\x1C'
		});
	BitSerializer::Detail::CBinTimestamp timestamp;

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(timestamp));
	EXPECT_EQ(0x0102030405060708, timestamp.Seconds);
	EXPECT_EQ(0x090A0B0C, timestamp.Nanoseconds);

	ASSERT_TRUE(this->mMsgPackReader->ReadValue(timestamp));
	EXPECT_EQ(0x1112131415161718, timestamp.Seconds);
	EXPECT_EQ(0x191A1B1C, timestamp.Nanoseconds);
}

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenUnexpectedEndOfTimestamp32)
{
	BitSerializer::Detail::CBinTimestamp timestamp;

	this->PrepareReader(std::string({ '\xD6',  }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(timestamp), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xD6', '\xFF' }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(timestamp), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xD6', '\xFF', '\x10' }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(timestamp), BitSerializer::ParsingException);
}

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenUnexpectedEndOfTimestamp64)
{
	BitSerializer::Detail::CBinTimestamp timestamp;

	this->PrepareReader(std::string({ '\xD7', }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(timestamp), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xD7', '\xFF' }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(timestamp), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xD7', '\xFF', '\x04' }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(timestamp), BitSerializer::ParsingException);
}

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenUnexpectedEndOfTimestamp96)
{
	BitSerializer::Detail::CBinTimestamp timestamp;

	this->PrepareReader(std::string({ '\xC7', }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(timestamp), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xC7', '\x0C' }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(timestamp), BitSerializer::ParsingException);

	this->PrepareReader(std::string({ '\xC7', '\x0C', '\xFF' }));
	EXPECT_THROW(this->mMsgPackReader->ReadValue(timestamp), BitSerializer::ParsingException);
}

TYPED_TEST(MsgPackReaderTest, ShouldThrowExceptionWhenReadTimestampToWrongType)
{
	this->PrepareReader({ '\xD6', '\xFF', '\x10', '\x20', '\x30', '\x40' });
	std::string_view wrongType;
	EXPECT_THROW(this->mMsgPackReader->ReadValue(wrongType), BitSerializer::SerializationException);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipTimestampWhenPolicyIsSkip)
{
	this->PrepareReader({ '\xD6', '\xFF', '\x10', '\x20', '\x30', '\x40' }, 
		BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	std::string_view wrongType;
	EXPECT_FALSE(this->mMsgPackReader->ReadValue(wrongType));
	EXPECT_TRUE(wrongType.empty());
	EXPECT_TRUE(this->mMsgPackReader->IsEnd()) << "Value should be skipped";
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
	this->PrepareReader("\xC3\xC2");
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
	for (int i = 0xE0; i <= 0xFF; ++i)
	{
		this->PrepareReader({ static_cast<char>(i) });
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

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfTimestamp32)
{
	this->PrepareReader({ '\xD6', '\xFF', '\x10', '\x20', '\x30', '\x40' });
	EXPECT_EQ(BitSerializer::MsgPack::Detail::ValueType::Timestamp, this->mMsgPackReader->ReadValueType());
	EXPECT_EQ(0, this->mMsgPackReader->GetPosition());
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfTimestamp64)
{
	this->PrepareReader({ '\xD7', '\xFF', '\x04', '\x08', '\x0C', '\x10', '\x10', '\x20', '\x30', '\x40' });
	EXPECT_EQ(BitSerializer::MsgPack::Detail::ValueType::Timestamp, this->mMsgPackReader->ReadValueType());
	EXPECT_EQ(0, this->mMsgPackReader->GetPosition());
}

TYPED_TEST(MsgPackReaderTest, ShouldReadTypeOfTimestamp96)
{
	this->PrepareReader({ '\xC7', '\x0C', '\xFF', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\x09', '\x0A', '\x0B', '\x0C' });
	EXPECT_EQ(BitSerializer::MsgPack::Detail::ValueType::Timestamp, this->mMsgPackReader->ReadValueType());
	EXPECT_EQ(0, this->mMsgPackReader->GetPosition());
}

//-----------------------------------------------------------------------------
// Tests for skip values
//-----------------------------------------------------------------------------
TYPED_TEST(MsgPackReaderTest, ShouldSkipFixInt)
{
	for (int i = -32; i < 128; ++i)
	{
		this->PrepareReader({ static_cast<char>(i), '\x10' });
		this->mMsgPackReader->SkipValue();
		char value = '\x80';
		EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
		EXPECT_EQ(16, value);
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipNil)
{
	this->PrepareReader("\xC0\xC2");
	bool value;
	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipBoolean)
{
	this->PrepareReader("\xC2\xC3");
	this->mMsgPackReader->SkipValue();
	bool value = false;
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(true, value);

	this->PrepareReader("\xC3\xC2");
	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(false, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipFloat)
{
	this->PrepareReader("\xCA\x40\x48\xF5\xC3\x10");
	char value = '\xFF';

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(16, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipDouble)
{
	this->PrepareReader("\xCB\x40\x09\x21\xFB\x54\x52\x45\x50\x10");
	char value = '\xFF';

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(16, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipUint8)
{
	this->PrepareReader({
		'\xCC', '\x80',		// From uint8
		'\xCC', '\x10',		// From uint8
		});
	uint8_t value = 0;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(16, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipUint16)
{
	this->PrepareReader({
		'\xCD', '\x40', '\x80',			// From uint16
		'\xCD', '\xCA', '\xFE'			// From uint16
	});
	uint16_t value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0xCAFE, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipUint32)
{
	this->PrepareReader({
		'\xCE', '\x40', '\x80', '\x20', '\x10',		// From uint32
		'\xCE', '\xCA', '\xFE', '\x20', '\x30'		// From uint32
	});
	uint32_t value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(0xCAFE2030, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipUint64)
{
	this->PrepareReader({
		'\xCF', '\xA0', '\x90', '\x80', '\x70', '\x60', '\x50', '\x40', '\x30',		// From uint64
		'\xCF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF' 		// From uint64
	});
	uint64_t value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<uint64_t>::max(), value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipInt8)
{
	this->PrepareReader({
		'\xD0', '\x80',		// From int8
		'\xD0', '\xCE'		// From int8
	});
	int8_t value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-50, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipInt16)
{
	this->PrepareReader({
		'\xD1', '\x80', '\x00',		// From int16
		'\xD1', '\xA0', '\x83'		// From int16
	});
	int16_t value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-24445, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipInt32)
{
	this->PrepareReader({
		'\xD2', '\x80', '\x00', '\x00', '\x00',		// From int32
		'\xD2', '\xA0', '\x83', '\x00', '\x00'		// From int32
	});
	int32_t value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(-1602027520, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipInt64)
{
	this->PrepareReader({
		'\xD3', '\x80', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',		// From int64
		'\xD3', '\x7F', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF'		// From int64
	});
	int64_t value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int64_t>::max(), value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipStringWithFixedSize)
{
	const std::string expectedStr = "Hello world!";
	for (int i = 0; i < 31; ++i)
	{
		this->PrepareReader(static_cast<char>(i | 0b10100000) + this->GenTestString(i) +
			std::string({ static_cast<char>(static_cast<uint8_t>(expectedStr.size()) | 0b10100000) }) + expectedStr
		);

		std::string_view actualStr;
		this->mMsgPackReader->SkipValue();
		EXPECT_TRUE(this->mMsgPackReader->ReadValue(actualStr));
		EXPECT_EQ(expectedStr, actualStr);
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipStringWhenSizeFitToUint8)
{
	const std::string expectedStr = "Hello world!";
	this->PrepareReader(
		std::string({ '\xD9', '\xFF' }) + this->GenTestString(std::numeric_limits<uint8_t>::max()) +
		std::string({ static_cast<char>(static_cast<uint8_t>(expectedStr.size()) | 0b10100000) }) + expectedStr
	);

	std::string_view actualStr;
	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(actualStr));
	EXPECT_EQ(expectedStr, actualStr);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipStringWhenSizeFitToUint16)
{
	const std::string expectedStr = "Hello world!";
	this->PrepareReader(
		std::string({ '\xDA', '\xFF', '\xFF' }) + this->GenTestString(std::numeric_limits<uint16_t>::max()) +
		std::string({ static_cast<char>(static_cast<uint8_t>(expectedStr.size()) | 0b10100000) }) + expectedStr
	);

	std::string_view actualStr;
	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(actualStr));
	ASSERT_EQ(expectedStr.size(), actualStr.size());
	EXPECT_EQ(std::hash<std::string>()(expectedStr), std::hash<std::string_view>()(actualStr));
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipStringWhenSizeFitToUint32)
{
	const std::string expectedStr = "Hello world!";
	this->PrepareReader(
		std::string({ '\xDB', '\x00', '\x01', '\x00', '\x02' }) + this->GenTestString(std::numeric_limits<uint16_t>::max() + 3) +
		std::string({ static_cast<char>(static_cast<uint8_t>(expectedStr.size()) | 0b10100000) }) + expectedStr
	);

	std::string_view actualStr;
	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(actualStr));
	ASSERT_EQ(expectedStr.size(), actualStr.size());
	EXPECT_EQ(std::hash<std::string>()(expectedStr), std::hash<std::string_view>()(actualStr));
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipBinArrayWhenSizeFitToUint8)
{
	this->PrepareReader({
		'\xC4', '\x03', '\x01', '\x02', '\x03',
		'\xC4', '\x01', '\x10' });
	this->mMsgPackReader->SkipValue();
	size_t arraySize;
	EXPECT_TRUE(this->mMsgPackReader->ReadBinarySize(arraySize));
	EXPECT_EQ(1, arraySize);
	EXPECT_EQ('\x10', this->mMsgPackReader->ReadBinary());
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipBinArrayWhenSizeFitToUint16)
{
	this->PrepareReader({
		'\xC5', '\x00', '\x03', '\x01', '\x02', '\x03',
		'\xC5', '\x00', '\x01', '\x10' });
	this->mMsgPackReader->SkipValue();
	size_t arraySize;
	EXPECT_TRUE(this->mMsgPackReader->ReadBinarySize(arraySize));
	EXPECT_EQ(1, arraySize);
	EXPECT_EQ('\x10', this->mMsgPackReader->ReadBinary());
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipBinArrayWhenSizeFitToUint32)
{
	this->PrepareReader({
		'\xC6', '\x00', '\x00', '\x00', '\x03', '\x01', '\x02', '\x03',
		'\xC6', '\x00', '\x00', '\x00', '\x01', '\x10' });
	this->mMsgPackReader->SkipValue();
	size_t arraySize;
	EXPECT_TRUE(this->mMsgPackReader->ReadBinarySize(arraySize));
	EXPECT_EQ(1, arraySize);
	EXPECT_EQ('\x10', this->mMsgPackReader->ReadBinary());
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipFixedExt1)
{
	this->PrepareReader({
		'\xD4', '\xAA', '\x10',
		'\xC3'
		});
	bool value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_TRUE(value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipFixedExt2)
{
	this->PrepareReader({
		'\xD5', '\xAA', '\x10', '\x20',
		'\xC3'
		});
	bool value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_TRUE(value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipFixedExt4)
{
	this->PrepareReader({
		'\xD6', '\xAA', '\x10', '\x20', '\x30', '\x40',
		'\xC3'
		});
	bool value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_TRUE(value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipFixedExt8)
{
	this->PrepareReader({
		'\xD7', '\xAA', '\x10', '\x20', '\x30', '\x40', '\x50', '\x60', '\x70', '\x80',
		'\xC3'
		});
	bool value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_TRUE(value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipFixedExt16)
{
	this->PrepareReader({
		'\xD8', '\xAA', '\x21', '\x22', '\x23', '\x24', '\x25', '\x26', '\x27', '\x28', '\x29', '\x30', '\x31', '\x32', '\x33', '\x34', '\x35', '\x36',
		'\xC3'
		});
	bool value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_TRUE(value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipExtArrayWhenSizeFitToUint8)
{
	this->PrepareReader({
		'\xC7', '\x05', '\xAA', '\x01', '\x02', '\x03', '\x04', '\x05',
		'\xC3'
	});
	bool value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_TRUE(value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipExtArrayWhenSizeFitToUint16)
{
	this->PrepareReader(
		std::string({ '\xC8', '\xFF', '\xFF', '\xAA' }) + this->GenTestString(std::numeric_limits<uint16_t>::max()) +
		std::string({ '\xC3' })
	);
	bool value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_TRUE(value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipExtArrayWhenSizeFitToUint32)
{
	this->PrepareReader(
		std::string({ '\xC9', '\x00', '\x01', '\x00', '\x02', '\xAA' }) + this->GenTestString(std::numeric_limits<uint16_t>::max() + 3) +
		std::string({ '\xC3' })
	);
	bool value = false;

	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	EXPECT_TRUE(value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipArrayWithFixedSize)
{
	for (uint16_t i = 0; i < 15; ++i)
	{
		std::string data = { static_cast<char>('\x90' + i) };
		data.reserve(data.size() + i * (sizeof i + 1));
		for (uint16_t k = 0; k < i; ++k)
		{
			data += EncodeIntegralValue('\xCD', k);
		}
		data += '\xC3';
		this->PrepareReader(std::move(data));

		bool value;
		this->mMsgPackReader->SkipValue();
		EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
		ASSERT_EQ(true, value);
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipArrayWhenSizeFitToUint16)
{
	constexpr uint16_t testSize = 0x0410;
	std::string data = EncodeIntegralValue('\xDC', testSize);
	data.reserve(data.size() + testSize * (sizeof testSize + 1));
	for (uint16_t i = 0; i < testSize; ++i)
	{
		data += EncodeIntegralValue('\xCD', i);
	}
	data += '\xC3';
	this->PrepareReader(std::move(data));

	bool value;
	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	ASSERT_EQ(true, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipArrayWhenSizeFitToUint32)
{
	constexpr uint32_t testSize = 0x10000;
	std::string data = EncodeIntegralValue('\xDD', testSize);
	data.reserve(data.size() + testSize * (sizeof testSize + 1));
	for (uint32_t i = 0; i < testSize; ++i)
	{
		data += EncodeIntegralValue('\xCE', i);
	}
	data += '\xC3';
	this->PrepareReader(std::move(data));

	bool value;
	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	ASSERT_EQ(true, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipMapWithFixedSize)
{
	for (uint16_t i = 0; i < 15; ++i)
	{
		std::string data = { static_cast<char>('\x80' + i) };
		data.reserve(data.size() + i * (sizeof i + 1));
		for (uint16_t k = 0; k < i; ++k)
		{
			data += EncodeIntegralValue('\xCD', k);
			data += EncodeIntegralValue('\xCD', k);
		}
		data += '\xC3';
		this->PrepareReader(std::move(data));

		bool value;
		this->mMsgPackReader->SkipValue();
		EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
		ASSERT_EQ(true, value);
	}
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipMapWhenSizeFitToUint16)
{
	constexpr uint16_t testSize = 0x0410;
	std::string data = EncodeIntegralValue('\xDE', testSize);
	data.reserve(data.size() + testSize * (sizeof testSize * 2 + 1));
	for (uint16_t i = 0; i < testSize; ++i)
	{
		data += EncodeIntegralValue('\xCD', i);
		data += EncodeIntegralValue('\xCD', i);
	}
	data += '\xC3';
	this->PrepareReader(std::move(data));

	bool value;
	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	ASSERT_EQ(true, value);
}

TYPED_TEST(MsgPackReaderTest, ShouldSkipMapWhenSizeFitToUint32)
{
	constexpr uint32_t testSize = 0x10000;
	std::string data = EncodeIntegralValue('\xDF', testSize);
	data.reserve(data.size() + testSize * (sizeof testSize * 2 + 1));
	for (uint32_t i = 0; i < testSize; ++i)
	{
		data += EncodeIntegralValue('\xCE', i);
		data += EncodeIntegralValue('\xCE', i);
	}
	data += '\xC3';
	this->PrepareReader(std::move(data));

	bool value;
	this->mMsgPackReader->SkipValue();
	EXPECT_TRUE(this->mMsgPackReader->ReadValue(value));
	ASSERT_EQ(true, value);
}
