/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "msgpack_readers.h"

/*
-----------------------------------------------------------
Format name 	First byte(in binary) 	First byte(in hex)
-----------------------------------------------------------
positive		fixint 	0xxxxxxx		0x00 - 0x7f
fixmap 			1000xxxx 	0x80		0x8f
fixarray		1001xxxx 	0x90		0x9f
fixstr 			101xxxxx 	0xa0		0xbf
nil 			11000000 	0xc0
(never used)	11000001 	0xc1
false 			11000010 	0xc2
true 			11000011 	0xc3
bin 8 			11000100 	0xc4
bin 16 			11000101 	0xc5
bin 32 			11000110 	0xc6
ext 8 			11000111 	0xc7
ext 16 			11001000 	0xc8
ext 32 			11001001 	0xc9
float 32		11001010 	0xca
float 64		11001011 	0xcb
uint 8 			11001100 	0xcc
uint 16			11001101 	0xcd
uint 32			11001110 	0xce
uint 64			11001111 	0xcf
int 8 			11010000 	0xd0
int 16 			11010001 	0xd1
int 32 			11010010 	0xd2
int 64 			11010011 	0xd3
fixext 1		11010100 	0xd4
fixext 2		11010101 	0xd5
fixext 4		11010110 	0xd6
fixext 8		11010111 	0xd7
fixext 16 		11011000 	0xd8
str 8 			11011001 	0xd9
str 16 			11011010 	0xda
str 32 			11011011 	0xdb
array 16		11011100 	0xdc
array 32		11011101 	0xdd
map 16 			11011110 	0xde
map 32 			11011111 	0xdf
negative fixint 111xxxxx	0xe0		0xff
*/

namespace
{
	using namespace BitSerializer;

	MsgPack::Detail::ValueType ReadValueTypeImpl(const std::string_view& inputData, const size_t& pos)
	{
		if (pos < inputData.size())
		{
			const auto ch = inputData[pos];
			if ((static_cast<uint8_t>(ch) & 0b11100000) == 0b10100000 || ch == '\xD9' || ch == '\xDA' || ch == '\xDB') {
				return MsgPack::Detail::ValueType::String;
			}
			if (ch >= 0 || ch == '\xCC' || ch == '\xCD' || ch == '\xCE' || ch == '\xCF') {
				return MsgPack::Detail::ValueType::UnsignedInteger;
			}
			if (ch >= -32 || ch == '\xD0' || ch == '\xD1' || ch == '\xD2' || ch == '\xD3') {
				return MsgPack::Detail::ValueType::SignedInteger;
			}
			if (ch == '\xC2' || ch == '\xC3') {
				return MsgPack::Detail::ValueType::Boolean;
			}
			if (ch == '\xC0') {
				return MsgPack::Detail::ValueType::Nil;
			}
			if (ch == '\xCA') {
				return MsgPack::Detail::ValueType::Float;
			}
			if (ch == '\xCB') {
				return MsgPack::Detail::ValueType::Double;
			}
			if ((static_cast<uint8_t>(ch) & 0b11110000) == 0b10000000 || ch == '\xDE' || ch == '\xDF') {
				return MsgPack::Detail::ValueType::Map;
			}
			if ((static_cast<uint8_t>(ch) & 0b11110000) == 0b10010000 || ch == '\xDC' || ch == '\xDD') {
				return MsgPack::Detail::ValueType::Array;
			}
			return MsgPack::Detail::ValueType::Unknown;
		}
		throw ParsingException("No more values to read", pos);
	}

	template <typename T, std::enable_if_t<sizeof T == 1 && std::is_integral_v<T>, int> = 0>
	void GetValue(std::string_view& inputData, size_t& pos, T& outValue)
	{
		if (pos < inputData.size())
		{
			outValue = static_cast<unsigned char>(inputData[pos++]);
		}
		else {
			throw ParsingException("Unexpected end of input archive", pos);
		}
	}

	template <typename T, std::enable_if_t<sizeof T == 2 && std::is_integral_v<T>, int> = 0>
	void GetValue(std::string_view& inputData, size_t& pos, T& outValue)
	{
		if (pos + sizeof(T) <= inputData.size())
		{
			char* valPtr = reinterpret_cast<char*>(&outValue);
			valPtr[1] = inputData[pos++];
			valPtr[0] = inputData[pos++];
		}
		else {
			throw ParsingException("Unexpected end of input archive", pos);
		}
	}

	template <typename T, std::enable_if_t<sizeof T == 4 && std::is_integral_v<T>, int> = 0>
	void GetValue(std::string_view& inputData, size_t& pos, T& outValue)
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
			throw ParsingException("Unexpected end of input archive", pos);
		}
	}

	template <typename T, std::enable_if_t<sizeof T == 8 && std::is_integral_v<T>, int> = 0>
	void GetValue(std::string_view& inputData, size_t& pos, T& outValue)
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
			throw ParsingException("Unexpected end of input archive", pos);
		}
	}

	void HandleMismatchedTypesPolicy(MsgPack::Detail::ValueType actualType, MismatchedTypesPolicy mismatchedTypesPolicy)
	{
		// Null value is excluded from MismatchedTypesPolicy processing
		if (actualType != MsgPack::Detail::ValueType::Nil && mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
		{
			throw SerializationException(SerializationErrorCode::MismatchedTypes,
				"The type of target field does not match the value being loaded");
		}
	}

	template <typename T>
	bool ReadInteger(std::string_view& inputData, size_t& pos, T& outValue, OverflowNumberPolicy overflowNumberPolicy, MismatchedTypesPolicy mismatchedTypesPolicy)
	{
		if (pos < inputData.size())
		{
			const char ch = inputData[pos];
			if (ch >= -32)
			{
				++pos;
				return Detail::SafeNumberCast(ch, outValue, overflowNumberPolicy);
			}
			if (ch == '\xCC')
			{
				++pos;
				uint8_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, overflowNumberPolicy);
			}
			if (ch == '\xCD')
			{
				++pos;
				uint16_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, overflowNumberPolicy);
			}
			if (ch == '\xCE')
			{
				++pos;
				uint32_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, overflowNumberPolicy);
			}
			if (ch == '\xCF')
			{
				++pos;
				uint64_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, overflowNumberPolicy);
			}
			if (ch == '\xD0')
			{
				++pos;
				int8_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, overflowNumberPolicy);
			}
			if (ch == '\xD1')
			{
				++pos;
				int16_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, overflowNumberPolicy);
			}
			if (ch == '\xD2')
			{
				++pos;
				int32_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, overflowNumberPolicy);
			}
			if (ch == '\xD3')
			{
				++pos;
				int64_t val;
				GetValue(inputData, pos, val);
				return Detail::SafeNumberCast(val, outValue, overflowNumberPolicy);
			}
			// Read from boolean
			if (ch == '\xC2')
			{
				++pos;
				return Detail::SafeNumberCast(0, outValue, overflowNumberPolicy);
			}
			if (ch == '\xC3')
			{
				++pos;
				return Detail::SafeNumberCast(1, outValue, overflowNumberPolicy);
			}
			HandleMismatchedTypesPolicy(ReadValueTypeImpl(inputData, pos), mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", pos);
	}
}

//-----------------------------------------------------------------------------

namespace BitSerializer::MsgPack::Detail
{
	CMsgPackStringReader::CMsgPackStringReader(std::string_view inputData, OverflowNumberPolicy overflowNumberPolicy, MismatchedTypesPolicy mismatchedTypesPolicy)
		: mInputData(inputData)
		, mOverflowNumberPolicy(overflowNumberPolicy)
		, mMismatchedTypesPolicy(mismatchedTypesPolicy)
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

	ValueType CMsgPackStringReader::ReadValueType() const
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

			HandleMismatchedTypesPolicy(ReadValueTypeImpl(mInputData, mPos), mMismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", mPos);
	}

	bool CMsgPackStringReader::ReadValue(bool& value)
	{
		return ReadInteger(mInputData, mPos, value, mOverflowNumberPolicy, mMismatchedTypesPolicy);
	}

	bool CMsgPackStringReader::ReadValue(uint8_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mOverflowNumberPolicy, mMismatchedTypesPolicy);
	}

	bool CMsgPackStringReader::ReadValue(uint16_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mOverflowNumberPolicy, mMismatchedTypesPolicy);
	}

	bool CMsgPackStringReader::ReadValue(uint32_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mOverflowNumberPolicy, mMismatchedTypesPolicy);
	}

	bool CMsgPackStringReader::ReadValue(uint64_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mOverflowNumberPolicy, mMismatchedTypesPolicy);
	}

	bool CMsgPackStringReader::ReadValue(char& value)
	{
		return ReadInteger(mInputData, mPos, value, mOverflowNumberPolicy, mMismatchedTypesPolicy);
	}

	bool CMsgPackStringReader::ReadValue(int8_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mOverflowNumberPolicy, mMismatchedTypesPolicy);
	}

	bool CMsgPackStringReader::ReadValue(int16_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mOverflowNumberPolicy, mMismatchedTypesPolicy);
	}

	bool CMsgPackStringReader::ReadValue(int32_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mOverflowNumberPolicy, mMismatchedTypesPolicy);
	}

	bool CMsgPackStringReader::ReadValue(int64_t& value)
	{
		return ReadInteger(mInputData, mPos, value, mOverflowNumberPolicy, mMismatchedTypesPolicy);
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
				return BitSerializer::Detail::SafeNumberCast(temp, value, mOverflowNumberPolicy);
			}

			HandleMismatchedTypesPolicy(ReadValueTypeImpl(mInputData, mPos), mMismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", mPos);
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

			HandleMismatchedTypesPolicy(ReadValueTypeImpl(mInputData, mPos), mMismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", mPos);
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
				HandleMismatchedTypesPolicy(ReadValueTypeImpl(mInputData, mPos), mMismatchedTypesPolicy);
				return false;
			}

			if (mPos + size <= mInputData.size())
			{
				value = std::string_view(mInputData.data() + mPos, size);
				mPos += size;
				return true;
			}
			throw ParsingException("Unexpected end of input archive", mPos);
		}
		throw ParsingException("No more values to read", mPos);
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
			return false;
		}
		throw ParsingException("No more values to read", mPos);
	}

	bool CMsgPackStringReader::ReadMapSize(size_t& arraySize)
	{
		if (mPos < mInputData.size())
		{
			const auto ch = mInputData[mPos];
			if ((static_cast<uint8_t>(ch) & 0b11110000) == 0b10000000)
			{
				++mPos;
				arraySize = ch & 0b00001111;
				return true;
			}
			if (ch == '\xDE')
			{
				++mPos;
				uint16_t sz16;
				GetValue(mInputData, mPos, sz16);
				arraySize = sz16;
				return true;
			}
			if (ch == '\xDF')
			{
				++mPos;
				uint32_t sz32;
				GetValue(mInputData, mPos, sz32);
				arraySize = sz32;
				return true;
			}
			return false;
		}
		throw ParsingException("No more values to read", mPos);
	}

	void CMsgPackStringReader::SkipValue()
	{

	}
}
