/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "msgpack_readers.h"

/*
-----------------------------------------------------------
Format name 	First byte(in binary) 	First byte(in hex)
-----------------------------------------------------------
positive fixint 0xxxxxxx				0x00 - 0x7f
fixmap 			1000xxxx 				0x80 - 0x8f
fixarray		1001xxxx 				0x90 - 0x9f
fixstr 			101xxxxx 				0xa0 - 0xbf
nil 			11000000 				0xc0
(never used)	11000001 				0xc1
false 			11000010 				0xc2
true 			11000011 				0xc3
bin 8 			11000100 				0xc4
bin 16 			11000101 				0xc5
bin 32 			11000110 				0xc6
ext 8 			11000111 				0xc7
ext 16 			11001000 				0xc8
ext 32 			11001001 				0xc9
float 32		11001010 				0xca
float 64		11001011 				0xcb
uint 8 			11001100 				0xcc
uint 16			11001101 				0xcd
uint 32			11001110 				0xce
uint 64			11001111 				0xcf
int 8 			11010000 				0xd0
int 16 			11010001 				0xd1
int 32 			11010010 				0xd2
int 64 			11010011 				0xd3
fixext 1		11010100 				0xd4
fixext 2		11010101 				0xd5
fixext 4		11010110 				0xd6
fixext 8		11010111 				0xd7
fixext 16 		11011000 				0xd8
str 8 			11011001 				0xd9
str 16 			11011010 				0xda
str 32 			11011011 				0xdb
array 16		11011100 				0xdc
array 32		11011101 				0xdd
map 16 			11011110 				0xde
map 32 			11011111 				0xdf
negative fixint 111xxxxx				0xe0 - 0xff
*/

namespace
{
	using namespace BitSerializer;
	using ValueType = MsgPack::Detail::ValueType;

	struct ByteCodeMetaInfo
	{
		ByteCodeMetaInfo(MsgPack::Detail::ValueType InType, uint_fast8_t InFixedSeq = 0, uint_fast8_t InDataSize = 0, uint_fast8_t InExtSize = 0)
			: Type(InType)
			, FixedSeq(InFixedSeq)
			, DataSize(InDataSize)
			, ExtSize(InExtSize)
		{ }

		ValueType Type = ValueType::Unknown;
		uint_fast8_t FixedSeq = 0;		// Size of fixed bytes sequence
		uint_fast8_t DataSize = 0;		// Size of data (like int, float, etc)
		uint_fast8_t ExtSize = 0;		// Number of bytes which is used for represent length of sequence like array or map
	};

	constexpr ValueType UInt = ValueType::UnsignedInteger;
	constexpr ValueType SInt = ValueType::SignedInteger;

	ByteCodeMetaInfo ByteCodeTable[256] =
	{
		// Fixed positive int (0x0 - 0x7f)
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },
		{ UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt }, { UInt },

		// Fixed map 0x80 - 0x8f
		{ ValueType::Map, 0 }, { ValueType::Map, 1 }, { ValueType::Map, 2 }, { ValueType::Map, 3 },
		{ ValueType::Map, 4 }, { ValueType::Map, 5 }, { ValueType::Map, 6 }, { ValueType::Map, 7 },
		{ ValueType::Map, 8 }, { ValueType::Map, 9 }, { ValueType::Map, 10 }, { ValueType::Map, 11 },
		{ ValueType::Map, 12 }, { ValueType::Map, 13 }, { ValueType::Map, 14 }, { ValueType::Map, 15 },

		// Fixed array 0x90 - 0x9f
		{ ValueType::Array, 0 }, { ValueType::Array, 1 }, { ValueType::Array, 2 }, { ValueType::Array, 3 },
		{ ValueType::Array, 4 }, { ValueType::Array, 5 }, { ValueType::Array, 6 }, { ValueType::Array, 7 },
		{ ValueType::Array, 8 }, { ValueType::Array, 9 }, { ValueType::Array, 10 }, { ValueType::Array, 11 },
		{ ValueType::Array, 12 }, { ValueType::Array, 13 }, { ValueType::Array, 14 }, { ValueType::Array, 15 },

		// Fixed string (0xa0 - 0xbf)
		{ ValueType::String, 0 }, { ValueType::String, 1 }, { ValueType::String, 2 }, { ValueType::String, 3 },
		{ ValueType::String, 4 }, { ValueType::String, 5 }, { ValueType::String, 6 }, { ValueType::String, 7 },
		{ ValueType::String, 8 }, { ValueType::String, 9 }, { ValueType::String, 10 }, { ValueType::String, 11 },
		{ ValueType::String, 12 }, { ValueType::String, 13 }, { ValueType::String, 14 }, { ValueType::String, 15 },
		{ ValueType::String, 16 }, { ValueType::String, 17 }, { ValueType::String, 18 }, { ValueType::String, 19 },
		{ ValueType::String, 20 }, { ValueType::String, 21 }, { ValueType::String, 22 }, { ValueType::String, 23 },
		{ ValueType::String, 24 }, { ValueType::String, 25 }, { ValueType::String, 26 }, { ValueType::String, 27 },
		{ ValueType::String, 28 }, { ValueType::String, 29 }, { ValueType::String, 30 }, { ValueType::String, 31 },

		// Nil (0xc0)
		{ ValueType::Nil },

		// Never used (0xc1)
		{ ValueType::Unknown },

		// Boolean (false) 0xc2
		{ ValueType::Boolean },
		// Boolean (true) 0xc3
		{ ValueType::Boolean },

		// Binary 8 (0xc4)
		{ ValueType::Unknown, 0, 0, 1 },
		// Binary 16 (0xc5)
		{ ValueType::Unknown, 0, 0, 2 },
		// Binary 32 (0xc6)
		{ ValueType::Unknown, 0, 0, 4 },

		// ext 8 (0xc7)
		{ ValueType::Unknown, 0, 1, 1 },
		// ext 16 (0xc8)
		{ ValueType::Unknown, 0, 1, 2 },
		// ext 32 (0xc9)
		{ ValueType::Unknown, 0, 1, 4 },

		// float 32 (0xca)
		{ ValueType::Float, 0, 4 },
		// double 64 (0xcb)
		{ ValueType::Double, 0, 8 },

		// uint 8 (0xcc)
		{ ValueType::UnsignedInteger, 0, 1 },
		// uint 16 (0xcd)
		{ ValueType::UnsignedInteger, 0, 2 },
		// uint 32 (0xce)
		{ ValueType::UnsignedInteger, 0, 4 },
		// uint 64 (0xcf)
		{ ValueType::UnsignedInteger, 0, 8 },

		// int 8 (0xd0)
		{ ValueType::SignedInteger, 0, 1 },
		// int 16 (0xd1)
		{ ValueType::SignedInteger, 0, 2 },
		// int 32 (0xd2)
		{ ValueType::SignedInteger, 0, 4 },
		// int 64 (0xd3)
		{ ValueType::SignedInteger, 0, 8 },

		// fixext 1 (0xd4)
		{ ValueType::Unknown, 0, 2 },
		// fixext 2 (0xd5)
		{ ValueType::Unknown, 0, 3 },
		// fixext 4 (0xd6)
		{ ValueType::Unknown, 0, 5 },
		// fixext 8 (0xd7)
		{ ValueType::Unknown, 0, 9 },
		// fixext 16 (0xd8)
		{ ValueType::Unknown, 0, 17 },

		// str 8 (0xd9)
		{ ValueType::String, 0, 0, 1 },
		// str 16 (0xda)
		{ ValueType::String, 0, 0, 2 },
		// str 32 (0xdb)
		{ ValueType::String, 0, 0, 4 },

		// array 16 (0xdc)
		{ ValueType::Array, 0, 0, 2 },
		// array 32 (0xdd)
		{ ValueType::Array, 0, 0, 4 },

		// map 16 (0xde)
		{ ValueType::Map, 0, 0, 2 },
		// map 32 (0xdf)
		{ ValueType::Map, 0, 0, 4 },

		// Fixed negative int (0xe0 - 0xff)
		{ SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt },
		{ SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt },
		{ SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt },
		{ SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt }, { SInt }
	};

	void HandleMismatchedTypesPolicy(MsgPack::Detail::ValueType actualType, MismatchedTypesPolicy mismatchedTypesPolicy)
	{
		// Null value is excluded from MismatchedTypesPolicy processing
		if (actualType != MsgPack::Detail::ValueType::Nil && mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
		{
			throw SerializationException(SerializationErrorCode::MismatchedTypes,
				"The type of target field does not match the value being loaded");
		}
	}
}


//-----------------------------------------------------------------------------
// CMsgPackStreamReader
//-----------------------------------------------------------------------------
namespace
{
	MsgPack::Detail::ValueType ReadValueTypeImpl(const std::string_view& inputData, const size_t& pos)
	{
		if (pos < inputData.size())
		{
			const auto byteCode = static_cast<uint_fast8_t>(inputData[pos]);
			return ByteCodeTable[byteCode].Type;
		}
		throw ParsingException("No more values to read", 0, pos);
	}

	template <typename T, std::enable_if_t<sizeof T == 1 && std::is_integral_v<T>, int> = 0>
	void GetValue(const std::string_view& inputData, size_t& pos, T & outValue)
	{
		if (pos < inputData.size())
		{
			outValue = static_cast<unsigned char>(inputData[pos++]);
		}
		else {
			throw ParsingException("Unexpected end of input archive", 0, pos);
		}
	}

	template <typename T, std::enable_if_t<sizeof T == 2 && std::is_integral_v<T>, int> = 0>
	void GetValue(const std::string_view& inputData, size_t& pos, T & outValue)
	{
		if (pos + sizeof(T) <= inputData.size())
		{
			char* valPtr = reinterpret_cast<char*>(&outValue);
			valPtr[1] = inputData[pos++];
			valPtr[0] = inputData[pos++];
		}
		else {
			throw ParsingException("Unexpected end of input archive", 0, pos);
		}
	}

	template <typename T, std::enable_if_t<sizeof T == 4 && std::is_integral_v<T>, int> = 0>
	void GetValue(const std::string_view& inputData, size_t& pos, T & outValue)
	{
		if (pos + sizeof(T) <= inputData.size())
		{
			char* valPtr = reinterpret_cast<char*>(&outValue);
			valPtr[3] = inputData[pos++];
			valPtr[2] = inputData[pos++];
			valPtr[1] = inputData[pos++];
			valPtr[0] = inputData[pos++];
		}
		else {
			throw ParsingException("Unexpected end of input archive", 0, pos);
		}
	}

	template <typename T, std::enable_if_t<sizeof T == 8 && std::is_integral_v<T>, int> = 0>
	void GetValue(const std::string_view& inputData, size_t& pos, T & outValue)
	{
		if (pos + sizeof(T) <= inputData.size())
		{
			char* valPtr = reinterpret_cast<char*>(&outValue);
			valPtr[7] = inputData[pos++];
			valPtr[6] = inputData[pos++];
			valPtr[5] = inputData[pos++];
			valPtr[4] = inputData[pos++];
			valPtr[3] = inputData[pos++];
			valPtr[2] = inputData[pos++];
			valPtr[1] = inputData[pos++];
			valPtr[0] = inputData[pos++];
		}
		else {
			throw ParsingException("Unexpected end of input archive", 0, pos);
		}
	}

	uint32_t ReadExtSize(uint_fast8_t extSizeBytesNum, std::string_view inputData, size_t pos)
	{
		if (pos + extSizeBytesNum < inputData.size())
		{
			if (extSizeBytesNum == 1)
			{
				uint8_t sz8;
				GetValue(inputData, pos, sz8);
				return sz8;
			}
			if (extSizeBytesNum == 2)
			{
				uint16_t sz16;
				GetValue(inputData, pos, sz16);
				return sz16;
			}
			if (extSizeBytesNum == 4)
			{
				uint32_t sz32;
				GetValue(inputData, pos, sz32);
				return sz32;
			}
			throw std::invalid_argument("Internal error: invalid range of 'extSizeBytesNum'");
		}
		throw ParsingException("No more values to read", 0, pos);
	}

	template <typename T>
	bool ReadInteger(const std::string_view& inputData, size_t& pos, T& outValue, const SerializationOptions& serializationOptions)
	{
		if (pos < inputData.size())
		{
			const char ch = inputData[pos];
			if (ch >= -32)
			{
				++pos;
				return Detail::SafeNumberCast(ch, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (ch == '\xCC')
			{
				++pos;
				uint8_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (ch == '\xCD')
			{
				++pos;
				uint16_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (ch == '\xCE')
			{
				++pos;
				uint32_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (ch == '\xCF')
			{
				++pos;
				uint64_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (ch == '\xD0')
			{
				++pos;
				int8_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (ch == '\xD1')
			{
				++pos;
				int16_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (ch == '\xD2')
			{
				++pos;
				int32_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (ch == '\xD3')
			{
				++pos;
				int64_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			// Read from boolean
			if (ch == '\xC2')
			{
				++pos;
				return Detail::SafeNumberCast(0, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (ch == '\xC3')
			{
				++pos;
				return Detail::SafeNumberCast(1, outValue, serializationOptions.overflowNumberPolicy);
			}
			HandleMismatchedTypesPolicy(ReadValueTypeImpl(inputData, pos), serializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, pos);
	}
}

namespace BitSerializer::MsgPack::Detail
{
	CMsgPackStringReader::CMsgPackStringReader(std::string_view inputData, const SerializationOptions& serializationOptions)
		: mInputData(inputData)
		, mSerializationOptions(serializationOptions)
	{ }

	void CMsgPackStringReader::SetPosition(size_t pos)
	{
		if (pos <= mInputData.size()) {
			mPos = pos;
		}
		else {
			throw std::invalid_argument("Internal error: position is out of range of input data");
		}
	}

	ValueType CMsgPackStringReader::ReadValueType()
	{
		return ReadValueTypeImpl(mInputData, mPos);
	}

	bool CMsgPackStringReader::ReadValue(std::nullptr_t&)
	{
		if (mPos < mInputData.size())
		{
			const auto ch = mInputData[mPos];
			if (ch == '\xC0')
			{
				++mPos;
				return true;
			}

			HandleMismatchedTypesPolicy(ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, mPos);
	}

	bool CMsgPackStringReader::ReadValue(bool& value)
	{
		return ReadInteger(mInputData, mPos, value, mSerializationOptions);
	}

	bool CMsgPackStringReader::ReadValue(uint8_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mSerializationOptions);
	}

	bool CMsgPackStringReader::ReadValue(uint16_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mSerializationOptions);
	}

	bool CMsgPackStringReader::ReadValue(uint32_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mSerializationOptions);
	}

	bool CMsgPackStringReader::ReadValue(uint64_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mSerializationOptions);
	}

	bool CMsgPackStringReader::ReadValue(char& value)
	{
		return ReadInteger(mInputData, mPos, value, mSerializationOptions);
	}

	bool CMsgPackStringReader::ReadValue(int8_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mSerializationOptions);
	}

	bool CMsgPackStringReader::ReadValue(int16_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mSerializationOptions);
	}

	bool CMsgPackStringReader::ReadValue(int32_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mSerializationOptions);
	}

	bool CMsgPackStringReader::ReadValue(int64_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mSerializationOptions);
	}

	bool CMsgPackStringReader::ReadValue(float& value)
	{
		if (mPos < mInputData.size())
		{
			const auto ch = mInputData[mPos];
			if (ch == '\xCA')
			{
				++mPos;
				uint32_t buf;
				GetValue(mInputData, mPos, buf);
				std::memcpy(&value, &buf, sizeof(uint32_t));
				return true;
			}
			if (ch == '\xCB')
			{
				++mPos;
				uint64_t buf;
				GetValue(mInputData, mPos, buf);
				double temp;
				std::memcpy(&temp, &buf, sizeof(uint64_t));
				return BitSerializer::Detail::SafeNumberCast(temp, value, mSerializationOptions.overflowNumberPolicy);
			}

			HandleMismatchedTypesPolicy(ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, mPos);
	}

	bool CMsgPackStringReader::ReadValue(double& value)
	{
		if (mPos < mInputData.size())
		{
			const auto ch = mInputData[mPos];
			if (ch == '\xCB')
			{
				++mPos;
				uint64_t buf;
				GetValue(mInputData, mPos, buf);
				std::memcpy(&value, &buf, sizeof(uint64_t));
				return true;
			}
			if (ch == '\xCA')
			{
				++mPos;
				uint32_t buf;
				GetValue(mInputData, mPos, buf);
				float temp;
				std::memcpy(&temp, &buf, sizeof(uint32_t));
				value = static_cast<double>(temp);
				return true;
			}

			HandleMismatchedTypesPolicy(ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, mPos);
	}

	bool CMsgPackStringReader::ReadValue(std::string_view& value)
	{
		if (mPos < mInputData.size())
		{
			size_t size;
			const auto ch = mInputData[mPos];
			if ((static_cast<uint8_t>(ch) & 0b11100000) == 0b10100000) {
				size = ch & 0b00011111;
				++mPos;
			}
			else if (ch == '\xD9')
			{
				++mPos;
				uint8_t sz8;
				GetValue(mInputData, mPos, sz8);
				size = sz8;
			}
			else if (ch == '\xDA')
			{
				++mPos;
				uint16_t sz16;
				GetValue(mInputData, mPos, sz16);
				size = sz16;
			}
			else if (ch == '\xDB')
			{
				++mPos;
				uint32_t sz32;
				GetValue(mInputData, mPos, sz32);
				size = sz32;
			}
			else
			{
				HandleMismatchedTypesPolicy(ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
				return false;
			}

			if (mPos + size <= mInputData.size())
			{
				value = std::string_view(mInputData.data() + mPos, size);
				mPos += size;
				return true;
			}
			throw ParsingException("Unexpected end of input archive", 0, mPos);
		}
		throw ParsingException("No more values to read", 0, mPos);
	}

	bool CMsgPackStringReader::ReadArraySize(size_t& arraySize)
	{
		if (mPos < mInputData.size())
		{
			const auto ch = mInputData[mPos];
			if ((static_cast<uint8_t>(ch) & 0b11110000) == 0b10010000)
			{
				++mPos;
				arraySize = ch & 0b00001111;
				return true;
			}
			if (ch == '\xDC')
			{
				++mPos;
				uint16_t sz16;
				GetValue(mInputData, mPos, sz16);
				arraySize = sz16;
				return true;
			}
			if (ch == '\xDD')
			{
				++mPos;
				uint32_t sz32;
				GetValue(mInputData, mPos, sz32);
				arraySize = sz32;
				return true;
			}

			HandleMismatchedTypesPolicy(ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, mPos);
	}

	bool CMsgPackStringReader::ReadMapSize(size_t& mapSize)
	{
		if (mPos < mInputData.size())
		{
			const auto ch = mInputData[mPos];
			if ((static_cast<uint8_t>(ch) & 0b11110000) == 0b10000000)
			{
				++mPos;
				mapSize = ch & 0b00001111;
				return true;
			}
			if (ch == '\xDE')
			{
				++mPos;
				uint16_t sz16;
				GetValue(mInputData, mPos, sz16);
				mapSize = sz16;
				return true;
			}
			if (ch == '\xDF')
			{
				++mPos;
				uint32_t sz32;
				GetValue(mInputData, mPos, sz32);
				mapSize = sz32;
				return true;
			}

			HandleMismatchedTypesPolicy(ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, mPos);
	}

	bool CMsgPackStringReader::ReadBinarySize(size_t& binarySize)
	{
		if (mPos < mInputData.size())
		{
			const auto ch = mInputData[mPos];
			if (ch == '\xC4')
			{
				++mPos;
				uint8_t sz8;
				GetValue(mInputData, mPos, sz8);
				binarySize = sz8;
				return true;
			}
			if (ch == '\xC5')
			{
				++mPos;
				uint16_t sz16;
				GetValue(mInputData, mPos, sz16);
				binarySize = sz16;
				return true;
			}
			if (ch == '\xC6')
			{
				++mPos;
				uint32_t sz32;
				GetValue(mInputData, mPos, sz32);
				binarySize = sz32;
				return true;
			}
		}
		throw ParsingException("No more values to read", 0, mPos);
	}

	char CMsgPackStringReader::ReadBinary()
	{
		if (mPos < mInputData.size())
		{
			return mInputData[mPos++];
		}
		throw ParsingException("No more values to read", 0, mPos);
	}

	void CMsgPackStringReader::SkipValue()
	{
		if (mPos < mInputData.size())
		{
			const auto byteCode = static_cast<uint_fast8_t>(mInputData[mPos]);
			const auto& byteCodeInfo = ByteCodeTable[byteCode];

			size_t size = 1 + byteCodeInfo.DataSize;
			uint32_t extSize = 0;
			if (byteCodeInfo.FixedSeq)
			{
				extSize += byteCodeInfo.FixedSeq;
			}
			else if (byteCodeInfo.ExtSize)
			{
				size += byteCodeInfo.ExtSize;
				extSize = ReadExtSize(byteCodeInfo.ExtSize, mInputData, mPos + 1);
			}

			if (byteCodeInfo.Type == ValueType::String)
			{
				size += extSize;
				extSize = 0;
			}

			if (mPos + size <= mInputData.size())
			{
				mPos += size;
				if (extSize)
				{
					if (byteCodeInfo.Type == ValueType::Map)
					{
						for (uint32_t i = 0; i < extSize; ++i)
						{
							SkipValue();
							SkipValue();
						}
					}
					else if (byteCodeInfo.Type == ValueType::Array)
					{
						for (uint32_t i = 0; i < extSize; ++i)
						{
							SkipValue();
						}
					}
				}
				return;
			}
			throw ParsingException("Unexpected end of input archive", 0, mPos);
		}
		throw ParsingException("No more values to read", 0, mPos);
	}
}


//-----------------------------------------------------------------------------
// CMsgPackStreamReader
//-----------------------------------------------------------------------------
namespace
{
	template <typename T, std::enable_if_t<sizeof T == 1 && std::is_integral_v<T>, int> = 0>
	void GetValue(Detail::CBinaryStreamReader& binaryStreamReader, T& outValue)
	{
		if (const auto value = binaryStreamReader.ReadByte())
		{
			outValue = static_cast<unsigned char>(*value);
		}
		else {
			throw ParsingException("Unexpected end of input archive", 0, binaryStreamReader.GetPosition());
		}
	}

	template <typename T, std::enable_if_t<sizeof T == 2 && std::is_integral_v<T>, int> = 0>
	void GetValue(Detail::CBinaryStreamReader& binaryStreamReader, T& outValue)
	{
		if (const auto data = binaryStreamReader.ReadSolidBlock(sizeof(T)); !data.empty())
		{
			char* valPtr = reinterpret_cast<char*>(&outValue);
			valPtr[1] = data[0];
			valPtr[0] = data[1];
		}
		else {
			throw ParsingException("Unexpected end of input archive", 0, binaryStreamReader.GetPosition());
		}
	}

	template <typename T, std::enable_if_t<sizeof T == 4 && std::is_integral_v<T>, int> = 0>
	void GetValue(Detail::CBinaryStreamReader& binaryStreamReader, T& outValue)
	{
		if (const auto data = binaryStreamReader.ReadSolidBlock(sizeof(T)); !data.empty())
		{
			char* valPtr = reinterpret_cast<char*>(&outValue);
			valPtr[3] = data[0];
			valPtr[2] = data[1];
			valPtr[1] = data[2];
			valPtr[0] = data[3];
		}
		else {
			throw ParsingException("Unexpected end of input archive", 0, binaryStreamReader.GetPosition());
		}
	}

	template <typename T, std::enable_if_t<sizeof T == 8 && std::is_integral_v<T>, int> = 0>
	void GetValue(Detail::CBinaryStreamReader& binaryStreamReader, T& outValue)
	{
		if (const auto data = binaryStreamReader.ReadSolidBlock(sizeof(T)); !data.empty())
		{
			char* valPtr = reinterpret_cast<char*>(&outValue);
			valPtr[7] = data[0];
			valPtr[6] = data[1];
			valPtr[5] = data[2];
			valPtr[4] = data[3];
			valPtr[3] = data[4];
			valPtr[2] = data[5];
			valPtr[1] = data[6];
			valPtr[0] = data[7];
		}
		else {
			throw ParsingException("Unexpected end of input archive", 0, binaryStreamReader.GetPosition());
		}
	}

	uint32_t ReadExtSize(Detail::CBinaryStreamReader& binaryStreamReader, uint_fast8_t extSizeBytesNum)
	{
		if (const auto data = binaryStreamReader.ReadSolidBlock(extSizeBytesNum); !data.empty())
		{
			size_t pos = 0;	// ToDo: think to remove
			if (extSizeBytesNum == 1)
			{
				uint8_t sz8;
				GetValue(data, pos, sz8);
				return sz8;
			}
			if (extSizeBytesNum == 2)
			{
				uint16_t sz16;
				GetValue(data, pos,sz16);
				return sz16;
			}
			if (extSizeBytesNum == 4)
			{
				uint32_t sz32;
				GetValue(data, pos, sz32);
				return sz32;
			}
			throw std::invalid_argument("Internal error: invalid range of 'extSizeBytesNum'");
		}
		throw ParsingException("No more values to read", 0, binaryStreamReader.GetPosition());
	}

	template <typename T>
	bool ReadInteger(Detail::CBinaryStreamReader& binaryStreamReader, T& outValue, const SerializationOptions& serializationOptions)
	{
		if (const auto byteCode = binaryStreamReader.PeekByte())
		{
			if (byteCode >= -32)
			{
				binaryStreamReader.GotoNextByte();
				return Detail::SafeNumberCast(*byteCode, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xCC')
			{
				binaryStreamReader.GotoNextByte();
				uint8_t val;
				GetValue(binaryStreamReader, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xCD')
			{
				binaryStreamReader.GotoNextByte();
				uint16_t val;
				GetValue(binaryStreamReader, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xCE')
			{
				binaryStreamReader.GotoNextByte();
				uint32_t val;
				GetValue(binaryStreamReader, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xCF')
			{
				binaryStreamReader.GotoNextByte();
				uint64_t val;
				GetValue(binaryStreamReader, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xD0')
			{
				binaryStreamReader.GotoNextByte();
				int8_t val;
				GetValue(binaryStreamReader, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xD1')
			{
				binaryStreamReader.GotoNextByte();
				int16_t val;
				GetValue(binaryStreamReader, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xD2')
			{
				binaryStreamReader.GotoNextByte();
				int32_t val;
				GetValue(binaryStreamReader, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xD3')
			{
				binaryStreamReader.GotoNextByte();
				int64_t val;
				GetValue(binaryStreamReader, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			// Read from boolean
			if (byteCode == '\xC2')
			{
				binaryStreamReader.GotoNextByte();
				return Detail::SafeNumberCast(0, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xC3')
			{
				binaryStreamReader.GotoNextByte();
				return Detail::SafeNumberCast(1, outValue, serializationOptions.overflowNumberPolicy);
			}
			HandleMismatchedTypesPolicy(ByteCodeTable[static_cast<uint8_t>(*byteCode)].Type, serializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, binaryStreamReader.GetPosition());
	}
}

namespace BitSerializer::MsgPack::Detail
{
	CMsgPackStreamReader::CMsgPackStreamReader(std::istream& inputStream, const SerializationOptions& serializationOptions)
		: mBinaryStreamReader(inputStream)
		, mSerializationOptions(serializationOptions)
	{ }

	ValueType CMsgPackStreamReader::ReadValueType()
	{
		if (const auto byteCode = mBinaryStreamReader.PeekByte())
		{
			return ByteCodeTable[static_cast<uint8_t>(*byteCode)].Type;
		}
		throw ParsingException("No more values to read", 0, mBinaryStreamReader.GetPosition());
	}

	bool CMsgPackStreamReader::ReadValue(std::nullptr_t&)
	{
		if (const auto byteCode = mBinaryStreamReader.PeekByte())
		{
			if (byteCode.value() == '\xC0')
			{
				mBinaryStreamReader.GotoNextByte();
				return true;
			}
			HandleMismatchedTypesPolicy(ByteCodeTable[static_cast<uint8_t>(*byteCode)].Type, mSerializationOptions.mismatchedTypesPolicy);
		}
		throw ParsingException("No more values to read", 0, mBinaryStreamReader.GetPosition());
	}

	bool CMsgPackStreamReader::ReadValue(bool& value)
	{
		return ReadInteger(mBinaryStreamReader, value, mSerializationOptions);
	}

	bool CMsgPackStreamReader::ReadValue(uint8_t& value)
	{
		return ReadInteger(mBinaryStreamReader, value, mSerializationOptions);
	}

	bool CMsgPackStreamReader::ReadValue(uint16_t& value)
	{
		return ReadInteger(mBinaryStreamReader, value, mSerializationOptions);
	}

	bool CMsgPackStreamReader::ReadValue(uint32_t& value)
	{
		return ReadInteger(mBinaryStreamReader, value, mSerializationOptions);
	}

	bool CMsgPackStreamReader::ReadValue(uint64_t& value)
	{
		return ReadInteger(mBinaryStreamReader, value, mSerializationOptions);
	}

	bool CMsgPackStreamReader::ReadValue(char& value)
	{
		return ReadInteger(mBinaryStreamReader, value, mSerializationOptions);
	}

	bool CMsgPackStreamReader::ReadValue(int8_t& value)
	{
		return ReadInteger(mBinaryStreamReader, value, mSerializationOptions);
	}

	bool CMsgPackStreamReader::ReadValue(int16_t& value)
	{
		return ReadInteger(mBinaryStreamReader, value, mSerializationOptions);
	}

	bool CMsgPackStreamReader::ReadValue(int32_t& value)
	{
		return ReadInteger(mBinaryStreamReader, value, mSerializationOptions);
	}

	bool CMsgPackStreamReader::ReadValue(int64_t& value)
	{
		return ReadInteger(mBinaryStreamReader, value, mSerializationOptions);
	}

	bool CMsgPackStreamReader::ReadValue(float& value)
	{
		if (const auto byteCode = mBinaryStreamReader.PeekByte())
		{
			if (*byteCode == '\xCA')
			{
				mBinaryStreamReader.GotoNextByte();
				uint32_t buf;
				GetValue(mBinaryStreamReader, buf);
				std::memcpy(&value, &buf, sizeof(uint32_t));
				return true;
			}
			if (*byteCode == '\xCB')
			{
				mBinaryStreamReader.GotoNextByte();
				uint64_t buf;
				GetValue(mBinaryStreamReader, buf);
				double temp;
				std::memcpy(&temp, &buf, sizeof(uint64_t));
				return BitSerializer::Detail::SafeNumberCast(temp, value, mSerializationOptions.overflowNumberPolicy);
			}

			HandleMismatchedTypesPolicy(ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, mBinaryStreamReader.GetPosition());
	}

	bool CMsgPackStreamReader::ReadValue(double& value)
	{
		if (const auto byteCode = mBinaryStreamReader.PeekByte())
		{
			if (*byteCode == '\xCB')
			{
				mBinaryStreamReader.GotoNextByte();
				uint64_t buf;
				GetValue(mBinaryStreamReader, buf);
				std::memcpy(&value, &buf, sizeof(uint64_t));
				return true;
			}
			if (*byteCode == '\xCA')
			{
				mBinaryStreamReader.GotoNextByte();
				uint32_t buf;
				GetValue(mBinaryStreamReader, buf);
				float temp;
				std::memcpy(&temp, &buf, sizeof(uint32_t));
				value = static_cast<double>(temp);
				return true;
			}

			HandleMismatchedTypesPolicy(ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, mBinaryStreamReader.GetPosition());
	}

	bool CMsgPackStreamReader::ReadValue(std::string_view& value)
	{
		if (const auto byteCode = mBinaryStreamReader.PeekByte())
		{
			size_t remainingSize;
			if ((static_cast<uint8_t>(*byteCode) & 0b11100000) == 0b10100000)
			{
				remainingSize = *byteCode & 0b00011111;
				mBinaryStreamReader.GotoNextByte();
			}
			else if (*byteCode == '\xD9')
			{
				mBinaryStreamReader.GotoNextByte();
				uint8_t sz8;
				GetValue(mBinaryStreamReader, sz8);
				remainingSize = sz8;
			}
			else if (*byteCode == '\xDA')
			{
				mBinaryStreamReader.GotoNextByte();
				uint16_t sz16;
				GetValue(mBinaryStreamReader, sz16);
				remainingSize = sz16;
			}
			else if (*byteCode == '\xDB')
			{
				mBinaryStreamReader.GotoNextByte();
				uint32_t sz32;
				GetValue(mBinaryStreamReader, sz32);
				remainingSize = sz32;
			}
			else
			{
				HandleMismatchedTypesPolicy(ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
				return false;
			}

			mBuffer.clear();
			while (remainingSize != 0)
			{
				if (std::string_view chunk = mBinaryStreamReader.ReadByChunks(remainingSize); !chunk.empty())
				{
					mBuffer += chunk;
					remainingSize -= chunk.size();
				}
				else
				{
					throw ParsingException("Unexpected end of input archive", 0, mBinaryStreamReader.GetPosition());
				}
			}
			value = mBuffer;
			return true;
		}
		throw ParsingException("No more values to read", 0, mBinaryStreamReader.GetPosition());
	}

	bool CMsgPackStreamReader::ReadArraySize(size_t& arraySize)
	{
		if (const auto byteCode = mBinaryStreamReader.PeekByte())
		{
			if ((static_cast<uint8_t>(*byteCode) & 0b11110000) == 0b10010000)
			{
				mBinaryStreamReader.GotoNextByte();
				arraySize = *byteCode & 0b00001111;
				return true;
			}
			if (*byteCode == '\xDC')
			{
				mBinaryStreamReader.GotoNextByte();
				uint16_t sz16;
				GetValue(mBinaryStreamReader, sz16);
				arraySize = sz16;
				return true;
			}
			if (*byteCode == '\xDD')
			{
				mBinaryStreamReader.GotoNextByte();
				uint32_t sz32;
				GetValue(mBinaryStreamReader, sz32);
				arraySize = sz32;
				return true;
			}

			HandleMismatchedTypesPolicy(ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, mBinaryStreamReader.GetPosition());
	}

	bool CMsgPackStreamReader::ReadMapSize(size_t& mapSize)
	{
		if (const auto byteCode = mBinaryStreamReader.PeekByte())
		{
			if ((static_cast<uint8_t>(*byteCode) & 0b11110000) == 0b10000000)
			{
				mBinaryStreamReader.GotoNextByte();
				mapSize = *byteCode & 0b00001111;
				return true;
			}
			if (*byteCode == '\xDE')
			{
				mBinaryStreamReader.GotoNextByte();
				uint16_t sz16;
				GetValue(mBinaryStreamReader, sz16);
				mapSize = sz16;
				return true;
			}
			if (*byteCode == '\xDF')
			{
				mBinaryStreamReader.GotoNextByte();
				uint32_t sz32;
				GetValue(mBinaryStreamReader, sz32);
				mapSize = sz32;
				return true;
			}

			HandleMismatchedTypesPolicy(ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, mBinaryStreamReader.GetPosition());
	}

	bool CMsgPackStreamReader::ReadBinarySize(size_t& binarySize)
	{
		if (const auto byteCode = mBinaryStreamReader.PeekByte())
		{
			if (*byteCode == '\xC4')
			{
				mBinaryStreamReader.GotoNextByte();
				uint8_t sz8;
				GetValue(mBinaryStreamReader, sz8);
				binarySize = sz8;
				return true;
			}
			if (*byteCode == '\xC5')
			{
				mBinaryStreamReader.GotoNextByte();
				uint16_t sz16;
				GetValue(mBinaryStreamReader, sz16);
				binarySize = sz16;
				return true;
			}
			if (*byteCode == '\xC6')
			{
				mBinaryStreamReader.GotoNextByte();
				uint32_t sz32;
				GetValue(mBinaryStreamReader, sz32);
				binarySize = sz32;
				return true;
			}
		}
		throw ParsingException("No more values to read", 0, mBinaryStreamReader.GetPosition());
	}

	char CMsgPackStreamReader::ReadBinary()
	{
		if (const auto byteCode = mBinaryStreamReader.ReadByte())
		{
			return *byteCode;
		}
		throw ParsingException("No more values to read", 0, mBinaryStreamReader.GetPosition());
	}

	void CMsgPackStreamReader::SkipValue()
	{
		if (const auto byteCode = mBinaryStreamReader.ReadByte())
		{
			const auto& byteCodeInfo = ByteCodeTable[static_cast<uint8_t>(*byteCode)];

			size_t size = byteCodeInfo.DataSize;
			uint32_t extSize = 0;
			if (byteCodeInfo.FixedSeq)
			{
				extSize += byteCodeInfo.FixedSeq;
			}
			else if (byteCodeInfo.ExtSize)
			{
				extSize = ReadExtSize(mBinaryStreamReader, byteCodeInfo.ExtSize);
			}

			if (byteCodeInfo.Type == ValueType::String)
			{
				size += extSize;
				extSize = 0;
			}

			if (mBinaryStreamReader.SetPosition(mBinaryStreamReader.GetPosition() + size))
			{
				if (extSize)
				{
					if (byteCodeInfo.Type == ValueType::Map)
					{
						for (uint32_t i = 0; i < extSize; ++i)
						{
							SkipValue();
							SkipValue();
						}
					}
					else if (byteCodeInfo.Type == ValueType::Array)
					{
						for (uint32_t i = 0; i < extSize; ++i)
						{
							SkipValue();
						}
					}
				}
				return;
			}
			throw ParsingException("Unexpected end of input archive", 0, mBinaryStreamReader.GetPosition());
		}
		throw ParsingException("No more values to read", 0, mBinaryStreamReader.GetPosition());
	}
}
