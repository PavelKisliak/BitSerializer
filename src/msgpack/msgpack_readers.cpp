/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "msgpack_readers.h"
#include "bitserializer/conversion_detail/memory_utils.h"

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
		constexpr ByteCodeMetaInfo(ValueType InType, uint_fast8_t InFixedSeq = 0, uint_fast8_t InDataSize = 0, uint_fast8_t InExtSize = 0)
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

	const ByteCodeMetaInfo ByteCodeTable[256] =
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
		{ ValueType::BinaryArray, 0, 0, 1 },
		// Binary 16 (0xc5)
		{ ValueType::BinaryArray, 0, 0, 2 },
		// Binary 32 (0xc6)
		{ ValueType::BinaryArray, 0, 0, 4 },

		// ext 8 (0xc7)
		{ ValueType::Ext, 0, 1, 1 },
		// ext 16 (0xc8)
		{ ValueType::Ext, 0, 1, 2 },
		// ext 32 (0xc9)
		{ ValueType::Ext, 0, 1, 4 },

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
		{ ValueType::Ext, 1, 1 },
		// fixext 2 (0xd5)
		{ ValueType::Ext, 2, 1 },
		// fixext 4 (0xd6)
		{ ValueType::Ext, 4, 1 },
		// fixext 8 (0xd7)
		{ ValueType::Ext, 8, 1 },
		// fixext 16 (0xd8)
		{ ValueType::Ext, 16, 1 },

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

	struct ExtTypeInfo
	{
		MsgPack::Detail::ValueType ValueType = ValueType::Ext;
		uint8_t DataOffset = 0;
		uint32_t Size = 0;
		char ByteCode = 0;
		char ExtTypeCode = 0;
	};
}


//-----------------------------------------------------------------------------
// CMsgPackStringReader
//-----------------------------------------------------------------------------
namespace
{
	template <typename T, std::enable_if_t<sizeof(T) == 1 && std::is_integral_v<T>, int> = 0>
	void GetValue(std::string_view inputData, size_t& pos, T& outValue)
	{
		if (pos < inputData.size())
		{
			outValue = static_cast<unsigned char>(inputData[pos++]);
		}
		else {
			throw ParsingException("Unexpected end of input archive", 0, pos);
		}
	}

	template <typename T, std::enable_if_t<sizeof(T) >= 2 && std::is_integral_v<T>, int> = 0>
	void GetValue(std::string_view inputData, size_t& pos, T& outValue)
	{
		if (pos + sizeof(T) <= inputData.size())
		{
			outValue = Memory::BigEndianToNative(*reinterpret_cast<const T*>(inputData.data() + pos));
			pos += sizeof(T);
		}
		else {
			throw ParsingException("Unexpected end of input archive", 0, pos);
		}
	}

	uint32_t ReadExtSize(uint_fast8_t extSizeBytesNum, std::string_view inputData, size_t pos)
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

	void SkipValueImpl(std::string_view inputData, size_t& pos)
	{
		if (pos < inputData.size())
		{
			const auto& byteCodeInfo = ByteCodeTable[static_cast<uint_fast8_t>(inputData[pos++])];

			size_t size = byteCodeInfo.DataSize;
			uint32_t extSize = 0;
			if (byteCodeInfo.FixedSeq)
			{
				extSize += byteCodeInfo.FixedSeq;
			}
			else if (byteCodeInfo.ExtSize)
			{
				size += byteCodeInfo.ExtSize;
				extSize = ReadExtSize(byteCodeInfo.ExtSize, inputData, pos);
			}

			if (byteCodeInfo.Type == ValueType::String || byteCodeInfo.Type == ValueType::BinaryArray || byteCodeInfo.Type == ValueType::Ext)
			{
				size += extSize;
				extSize = 0;
			}

			if (pos + size <= inputData.size())
			{
				pos += size;
				if (extSize)
				{
					if (byteCodeInfo.Type == ValueType::Map)
					{
						for (uint32_t i = 0; i < extSize; ++i)
						{
							SkipValueImpl(inputData, pos);
							SkipValueImpl(inputData, pos);
						}
					}
					else if (byteCodeInfo.Type == ValueType::Array)
					{
						for (uint32_t i = 0; i < extSize; ++i)
						{
							SkipValueImpl(inputData, pos);
						}
					}
				}
				return;
			}
			throw ParsingException("Unexpected end of input archive", 0, pos);
		}
		throw ParsingException("No more values to read", 0, pos);
	}

	void HandleMismatchedTypesPolicy(std::string_view inputData, size_t& pos, ValueType actualType, MismatchedTypesPolicy mismatchedTypesPolicy)
	{
		// Null value is excluded from MismatchedTypesPolicy processing
		if (actualType != ValueType::Nil && mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
		{
			throw SerializationException(SerializationErrorCode::MismatchedTypes,
				"The type of target field does not match the value being loaded");
		}
		SkipValueImpl(inputData, pos);
	}

	template <typename T>
	bool ReadInteger(std::string_view inputData, size_t& pos, T& outValue, const SerializationOptions& serializationOptions)
	{
		if (pos < inputData.size())
		{
			const char byteCode = inputData[pos];
			if (byteCode >= -32)
			{
				++pos;
				return Detail::SafeNumberCast(byteCode, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xCC')
			{
				++pos;
				uint8_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xCD')
			{
				++pos;
				uint16_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xCE')
			{
				++pos;
				uint32_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xCF')
			{
				++pos;
				uint64_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xD0')
			{
				++pos;
				int8_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xD1')
			{
				++pos;
				int16_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xD2')
			{
				++pos;
				int32_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xD3')
			{
				++pos;
				int64_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, serializationOptions.overflowNumberPolicy);
			}
			// Read from boolean
			if (byteCode == '\xC2')
			{
				++pos;
				return Detail::SafeNumberCast(0, outValue, serializationOptions.overflowNumberPolicy);
			}
			if (byteCode == '\xC3')
			{
				++pos;
				return Detail::SafeNumberCast(1, outValue, serializationOptions.overflowNumberPolicy);
			}
			HandleMismatchedTypesPolicy(inputData, pos, ByteCodeTable[static_cast<uint_fast8_t>(byteCode)].Type, serializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, pos);
	}

	bool ReadExtFamilyType(std::string_view inputData, size_t pos, ExtTypeInfo& extTypeInfo)
	{
		if (pos < inputData.size())
		{
			extTypeInfo.ByteCode = inputData[pos];
			const auto& metaInfo = ByteCodeTable[static_cast<uint8_t>(extTypeInfo.ByteCode)];
			if (metaInfo.Type != ValueType::Ext) {
				return false;
			}

			// Ext format family with fixed data size
			if (metaInfo.FixedSeq)
			{
				extTypeInfo.Size = metaInfo.FixedSeq;
				extTypeInfo.DataOffset = 1 + metaInfo.DataSize;
				if (pos + extTypeInfo.DataOffset < inputData.size())
				{
					extTypeInfo.ExtTypeCode = inputData[pos + 1];
					// Currently only timestamp is specified as extension type
					if (extTypeInfo.ExtTypeCode == -1) {
						extTypeInfo.ValueType = ValueType::Timestamp;
					}
					return true;
				}
				throw ParsingException("Unexpected end of input archive", 0, pos);
			}

			// Ext format family with specified size
			if (metaInfo.ExtSize)
			{
				extTypeInfo.Size = ReadExtSize(metaInfo.ExtSize, inputData, pos + 1);
				extTypeInfo.DataOffset = 1 + metaInfo.DataSize + metaInfo.ExtSize;
				if (pos + extTypeInfo.DataOffset < inputData.size())
				{
					extTypeInfo.ExtTypeCode = inputData[pos + 1 + metaInfo.DataSize];
					// Currently only timestamp is specified as extension type
					if (extTypeInfo.ExtTypeCode == -1) {
						extTypeInfo.ValueType = ValueType::Timestamp;
					}
					return true;
				}
				throw ParsingException("Unexpected end of input archive", 0, pos);
			}
			throw std::runtime_error("Internal error: invalid external type descriptor");
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
		if (mPos < mInputData.size())
		{
			const char byteCode = mInputData[mPos];
			const auto& metaInfo = ByteCodeTable[static_cast<uint8_t>(byteCode)];
			if (metaInfo.Type == ValueType::Ext)
			{
				ExtTypeInfo extTypeInfo;
				ReadExtFamilyType(mInputData, mPos, extTypeInfo);
				return extTypeInfo.ValueType;
			}
			return metaInfo.Type;
		}
		throw ParsingException("No more values to read", 0, mPos);
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

			HandleMismatchedTypesPolicy(mInputData, mPos, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
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

			HandleMismatchedTypesPolicy(mInputData, mPos, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
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

			HandleMismatchedTypesPolicy(mInputData, mPos, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
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
				HandleMismatchedTypesPolicy(mInputData, mPos, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
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

	bool CMsgPackStringReader::ReadValue(CBinTimestamp& timestamp)
	{
		ExtTypeInfo extTypeInfo;
		if (ReadExtFamilyType(mInputData, mPos, extTypeInfo) && extTypeInfo.ExtTypeCode == -1)
		{
			mPos += extTypeInfo.DataOffset;
			if (extTypeInfo.Size == 4)
			{
				uint32_t data32;
				GetValue(mInputData, mPos, data32);
				timestamp.Seconds = data32;
				timestamp.Nanoseconds = 0;
				return true;
			}
			if (extTypeInfo.Size == 8)
			{
				uint64_t data64;
				GetValue(mInputData, mPos, data64);
				timestamp.Seconds = static_cast<int64_t>(data64 & 0x00000003ffffffffL);
				timestamp.Nanoseconds = static_cast<uint32_t>(data64 >> 34);
				return true;
			}
			if (extTypeInfo.Size == 12)
			{
				GetValue(mInputData, mPos, timestamp.Seconds);
				GetValue(mInputData, mPos, timestamp.Nanoseconds);
				return true;
			}
			throw SerializationException(SerializationErrorCode::ParsingError,
				"Invalid size of timestamp: " + Convert::ToString(extTypeInfo.Size));
		}
		HandleMismatchedTypesPolicy(mInputData, mPos, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
		return false;
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

			HandleMismatchedTypesPolicy(mInputData, mPos, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
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

			HandleMismatchedTypesPolicy(mInputData, mPos, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
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
		SkipValueImpl(mInputData, mPos);
	}
}


//-----------------------------------------------------------------------------
// CMsgPackStreamReader
//-----------------------------------------------------------------------------
namespace
{
	template <typename T, std::enable_if_t<sizeof(T) == 1 && std::is_integral_v<T>, int> = 0>
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

	template <typename T, std::enable_if_t<sizeof(T) >= 2 && std::is_integral_v<T>, int> = 0>
	void GetValue(Detail::CBinaryStreamReader& binaryStreamReader, T& outValue)
	{
		if (const auto data = binaryStreamReader.ReadSolidBlock(sizeof(T)); !data.empty())
		{
			outValue = Memory::BigEndianToNative(*reinterpret_cast<const T*>(data.data()));
		}
		else {
			throw ParsingException("Unexpected end of input archive", 0, binaryStreamReader.GetPosition());
		}
	}

	uint32_t ReadExtSize(Detail::CBinaryStreamReader& binaryStreamReader, uint_fast8_t extSizeBytesNum)
	{
		if (extSizeBytesNum == 1)
		{
			uint8_t sz8;
			GetValue(binaryStreamReader, sz8);
			return sz8;
		}
		if (extSizeBytesNum == 2)
		{
			uint16_t sz16;
			GetValue(binaryStreamReader, sz16);
			return sz16;
		}
		if (extSizeBytesNum == 4)
		{
			uint32_t sz32;
			GetValue(binaryStreamReader, sz32);
			return sz32;
		}
		throw std::invalid_argument("Internal error: invalid range of 'extSizeBytesNum'");
	}

	void SkipValueImpl(Detail::CBinaryStreamReader& binaryStreamReader)
	{
		if (const auto byteCode = binaryStreamReader.ReadByte())
		{
			const auto& byteCodeInfo = ByteCodeTable[static_cast<uint_fast8_t>(*byteCode)];

			size_t size = byteCodeInfo.DataSize;
			uint32_t extSize = 0;
			if (byteCodeInfo.FixedSeq)
			{
				extSize += byteCodeInfo.FixedSeq;
			}
			else if (byteCodeInfo.ExtSize)
			{
				extSize = ReadExtSize(binaryStreamReader, byteCodeInfo.ExtSize);
			}

			if (byteCodeInfo.Type == ValueType::String || byteCodeInfo.Type == ValueType::BinaryArray || byteCodeInfo.Type == ValueType::Ext)
			{
				size += extSize;
				extSize = 0;
			}

			if (size == 0 || binaryStreamReader.SetPosition(binaryStreamReader.GetPosition() + size))
			{
				if (extSize)
				{
					if (byteCodeInfo.Type == ValueType::Map)
					{
						for (uint32_t i = 0; i < extSize; ++i)
						{
							SkipValueImpl(binaryStreamReader);
							SkipValueImpl(binaryStreamReader);
						}
					}
					else if (byteCodeInfo.Type == ValueType::Array)
					{
						for (uint32_t i = 0; i < extSize; ++i)
						{
							SkipValueImpl(binaryStreamReader);
						}
					}
				}
				return;
			}
			throw ParsingException("Unexpected end of input archive", 0, binaryStreamReader.GetPosition());
		}
		throw ParsingException("No more values to read", 0, binaryStreamReader.GetPosition());
	}

	void HandleMismatchedTypesPolicy(Detail::CBinaryStreamReader& binaryStreamReader, ValueType actualType, MismatchedTypesPolicy mismatchedTypesPolicy)
	{
		// Null value is excluded from MismatchedTypesPolicy processing
		if (actualType != ValueType::Nil && mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
		{
			throw SerializationException(SerializationErrorCode::MismatchedTypes,
				"The type of target field does not match the value being loaded");
		}
		SkipValueImpl(binaryStreamReader);
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
			HandleMismatchedTypesPolicy(binaryStreamReader, ByteCodeTable[static_cast<uint8_t>(*byteCode)].Type, serializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", 0, binaryStreamReader.GetPosition());
	}

	bool ReadExtFamilyType(Detail::CBinaryStreamReader& binaryStreamReader, ExtTypeInfo& extTypeInfo)
	{
		if (const auto byteCode = binaryStreamReader.PeekByte())
		{
			extTypeInfo.ByteCode = *byteCode;
			const auto& metaInfo = ByteCodeTable[static_cast<uint8_t>(extTypeInfo.ByteCode)];
			if (metaInfo.Type != ValueType::Ext) {
				return false;
			}

			// Ext format family with fixed data size
			const auto prevPos = binaryStreamReader.GetPosition();
			binaryStreamReader.GotoNextByte();
			if (metaInfo.FixedSeq)
			{
				extTypeInfo.Size = metaInfo.FixedSeq;
				extTypeInfo.DataOffset = 1 + metaInfo.DataSize;
				if (const auto extCode = binaryStreamReader.ReadByte())
				{
					extTypeInfo.ExtTypeCode = *extCode;
					// Currently only timestamp is specified as extension type
					if (extTypeInfo.ExtTypeCode == -1) {
						extTypeInfo.ValueType = ValueType::Timestamp;
					}
					binaryStreamReader.SetPosition(prevPos);
					return true;
				}
				throw ParsingException("Unexpected end of input archive", 0, binaryStreamReader.GetPosition());
			}

			// Ext format family with specified size
			if (metaInfo.ExtSize)
			{
				extTypeInfo.Size = ReadExtSize(binaryStreamReader, metaInfo.ExtSize);
				extTypeInfo.DataOffset = 1 + metaInfo.DataSize + metaInfo.ExtSize;
				if (const auto extCode = binaryStreamReader.ReadByte())
				{
					extTypeInfo.ExtTypeCode = *extCode;
					// Currently only timestamp is specified as extension type
					if (extTypeInfo.ExtTypeCode == -1) {
						extTypeInfo.ValueType = ValueType::Timestamp;
					}
					binaryStreamReader.SetPosition(prevPos);
					return true;
				}
				throw ParsingException("Unexpected end of input archive", 0, binaryStreamReader.GetPosition());
			}
			throw std::runtime_error("Internal error: invalid external type descriptor");
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
			const auto& metaInfo = ByteCodeTable[static_cast<uint8_t>(*byteCode)];
			if (metaInfo.Type == ValueType::Ext)
			{
				ExtTypeInfo extTypeInfo;
				ReadExtFamilyType(mBinaryStreamReader, extTypeInfo);
				return extTypeInfo.ValueType;
			}
			return metaInfo.Type;
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
			HandleMismatchedTypesPolicy(mBinaryStreamReader, ByteCodeTable[static_cast<uint8_t>(*byteCode)].Type, mSerializationOptions.mismatchedTypesPolicy);
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

			HandleMismatchedTypesPolicy(mBinaryStreamReader, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
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

			HandleMismatchedTypesPolicy(mBinaryStreamReader, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
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
				HandleMismatchedTypesPolicy(mBinaryStreamReader, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
				return false;
			}

			mBuffer.clear();
			mBuffer.reserve(remainingSize);
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

	bool CMsgPackStreamReader::ReadValue(CBinTimestamp& timestamp)
	{
		ExtTypeInfo extTypeInfo;
		if (ReadExtFamilyType(mBinaryStreamReader, extTypeInfo) && extTypeInfo.ExtTypeCode == -1)
		{
			mBinaryStreamReader.SetPosition(mBinaryStreamReader.GetPosition() + extTypeInfo.DataOffset);
			if (extTypeInfo.Size == 4)
			{
				uint32_t data32;
				GetValue(mBinaryStreamReader, data32);
				timestamp.Seconds = data32;
				timestamp.Nanoseconds = 0;
				return true;
			}
			if (extTypeInfo.Size == 8)
			{
				uint64_t data64;
				GetValue(mBinaryStreamReader, data64);
				timestamp.Seconds = static_cast<int64_t>(data64 & 0x00000003ffffffffL);
				timestamp.Nanoseconds = static_cast<uint32_t>(data64 >> 34);
				return true;
			}
			if (extTypeInfo.Size == 12)
			{
				GetValue(mBinaryStreamReader, timestamp.Seconds);
				GetValue(mBinaryStreamReader, timestamp.Nanoseconds);
				return true;
			}
			throw SerializationException(SerializationErrorCode::ParsingError,
				"Invalid size of timestamp: " + Convert::ToString(extTypeInfo.Size));
		}
		HandleMismatchedTypesPolicy(mBinaryStreamReader, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
		return false;
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

			HandleMismatchedTypesPolicy(mBinaryStreamReader, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
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

			HandleMismatchedTypesPolicy(mBinaryStreamReader, ReadValueType(), mSerializationOptions.mismatchedTypesPolicy);
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
		SkipValueImpl(mBinaryStreamReader);
	}
}
