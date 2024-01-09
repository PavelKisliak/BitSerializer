/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "msgpack_writers.h"

namespace
{
	using namespace BitSerializer;

	template <typename T, std::enable_if_t<sizeof T == 1 && std::is_integral_v<T>, int> = 0>
	void PushValue(std::string& outputString, uint8_t code, T value)
	{
		outputString.push_back(static_cast<char>(code));
		outputString.push_back(static_cast<char>(value));
	}

	template <typename T, std::enable_if_t<sizeof T == 2 && std::is_integral_v<T>, int> = 0>
	void PushValue(std::string& outputString, uint8_t code, T value)
	{
		outputString.push_back(static_cast<char>(code));
		const char* valPtr = reinterpret_cast<const char*>(&value);
		outputString.push_back(valPtr[1]);
		outputString.push_back(valPtr[0]);
	}

	template <typename T, std::enable_if_t<sizeof T == 4 && std::is_integral_v<T>, int> = 0>
	void PushValue(std::string& outputString, uint8_t code, T value)
	{
		outputString.push_back(static_cast<char>(code));
		const char* valPtr = reinterpret_cast<const char*>(&value);
		outputString.push_back(valPtr[3]);
		outputString.push_back(valPtr[2]);
		outputString.push_back(valPtr[1]);
		outputString.push_back(valPtr[0]);
	}

	template <typename T, std::enable_if_t<sizeof T == 8 && std::is_integral_v<T>, int> = 0>
	void PushValue(std::string& outputString, uint8_t code, T value)
	{
		outputString.push_back(static_cast<char>(code));
		const char* valPtr = reinterpret_cast<const char*>(&value);
		outputString.push_back(valPtr[7]);
		outputString.push_back(valPtr[6]);
		outputString.push_back(valPtr[5]);
		outputString.push_back(valPtr[4]);
		outputString.push_back(valPtr[3]);
		outputString.push_back(valPtr[2]);
		outputString.push_back(valPtr[1]);
		outputString.push_back(valPtr[0]);
	}
}

namespace BitSerializer::MsgPack::Detail
{
	CMsgPackStringWriter::CMsgPackStringWriter(std::string& outputString)
		: mOutputString(outputString)
	{ }

	void CMsgPackStringWriter::WriteValue(std::nullptr_t)
	{
		mOutputString.push_back('\xC0');
	}

	void CMsgPackStringWriter::WriteValue(bool value)
	{
		mOutputString.push_back(value ? '\xC3' : '\xC2');
	}

	void CMsgPackStringWriter::WriteValue(uint8_t value)
	{
		if (value >= 128) {
			mOutputString.push_back('\xCC');
		}
		mOutputString.push_back(static_cast<char>(value));
	}

	void CMsgPackStringWriter::WriteValue(uint16_t value)
	{
		if (value > std::numeric_limits<uint8_t>::max()) {
			PushValue(mOutputString, '\xCD', value);
		}
		else {
			return WriteValue(static_cast<uint8_t>(value));
		}
	}

	void CMsgPackStringWriter::WriteValue(uint32_t value)
	{
		if (value > std::numeric_limits<uint16_t>::max()) {
			PushValue(mOutputString, '\xCE', value);
		}
		else {
			return WriteValue(static_cast<uint16_t>(value));
		}
	}

	void CMsgPackStringWriter::WriteValue(uint64_t value)
	{
		if (value > std::numeric_limits<uint32_t>::max()) {
			PushValue(mOutputString, '\xCF', value);
		}
		else {
			return WriteValue(static_cast<uint32_t>(value));
		}
	}

	void CMsgPackStringWriter::WriteValue(int8_t value)
	{
		if (value >= -32) {
			mOutputString.push_back(value);
		}
		else {
			PushValue(mOutputString, '\xD0', value);
		}
	}

	void CMsgPackStringWriter::WriteValue(int16_t value)
	{
		if (value < std::numeric_limits<int8_t>::min() || value > std::numeric_limits<int8_t>::max()) {
			PushValue(mOutputString, '\xD1', value);
		}
		else {
			return WriteValue(static_cast<int8_t>(value));
		}
	}

	void CMsgPackStringWriter::WriteValue(int32_t value)
	{
		if (value < std::numeric_limits<int16_t>::min() || value > std::numeric_limits<int16_t>::max()) {
			PushValue(mOutputString, '\xD2', value);
		}
		else {
			return WriteValue(static_cast<int16_t>(value));
		}
	}

	void CMsgPackStringWriter::WriteValue(int64_t value)
	{
		if (value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max()) {
			PushValue(mOutputString, '\xD3', value);
		}
		else {
			return WriteValue(static_cast<int32_t>(value));
		}
	}

	void CMsgPackStringWriter::WriteValue(float value)
	{
		uint32_t buf;
		std::memcpy(&buf, &value, sizeof(uint32_t));
		PushValue(mOutputString, '\xCA', buf);
	}

	void CMsgPackStringWriter::WriteValue(double value)
	{
		uint64_t buf;
		std::memcpy(&buf, &value, sizeof(uint64_t));
		PushValue(mOutputString, '\xCB', buf);
	}

	void CMsgPackStringWriter::WriteValue(std::string_view value)
	{
		if (value.size() < 32) {
			mOutputString.push_back(static_cast<char>(static_cast<uint8_t>(value.size()) | 0b10100000));
		}
		else
		{
			if (value.size() <= std::numeric_limits<uint8_t>::max()) {
				PushValue(mOutputString, '\xD9', static_cast<uint8_t>(value.size()));
			}
			else if (value.size() <= std::numeric_limits<uint16_t>::max()) {
				PushValue(mOutputString, '\xDA', static_cast<uint16_t>(value.size()));
			}
			else if (value.size() <= std::numeric_limits<uint32_t>::max()) {
				PushValue(mOutputString, '\xDB', static_cast<uint32_t>(value.size()));
			}
			else {
				throw SerializationException(SerializationErrorCode::OutOfRange, "String size is too large");
			}
		}
		mOutputString.append(value);
	}

	void CMsgPackStringWriter::BeginArray(size_t arraySize)
	{
		if (arraySize < 16) {
			mOutputString.push_back(static_cast<char>(static_cast<uint8_t>(arraySize) | 0b10010000));
		}
		else
		{
			if (arraySize <= std::numeric_limits<uint16_t>::max()) {
				PushValue(mOutputString, '\xDC', static_cast<uint16_t>(arraySize));
			}
			else if (arraySize <= std::numeric_limits<uint32_t>::max()) {
				PushValue(mOutputString, '\xDD', static_cast<uint32_t>(arraySize));
			}
			else {
				throw SerializationException(SerializationErrorCode::OutOfRange, "Array size is too large");
			}
		}
	}

	void CMsgPackStringWriter::BeginMap(size_t mapSize)
	{
		if (mapSize < 16) {
			mOutputString.push_back(static_cast<char>(static_cast<uint8_t>(mapSize) | 0b10000000));
		}
		else
		{
			if (mapSize <= std::numeric_limits<uint16_t>::max()) {
				PushValue(mOutputString, '\xDE', static_cast<uint16_t>(mapSize));
			}
			else if (mapSize <= std::numeric_limits<uint32_t>::max()) {
				PushValue(mOutputString, '\xDF', static_cast<uint32_t>(mapSize));
			}
			else {
				throw SerializationException(SerializationErrorCode::OutOfRange, "Map size is too large");
			}
		}
	}

	void CMsgPackStringWriter::BeginBinary(size_t binarySize)
	{
		if (binarySize <= std::numeric_limits<uint8_t>::max()) {
			PushValue(mOutputString, '\xC4', static_cast<uint8_t>(binarySize));
		}
		else if (binarySize <= std::numeric_limits<uint16_t>::max()) {
			PushValue(mOutputString, '\xC5', static_cast<uint16_t>(binarySize));
		}
		else if (binarySize <= std::numeric_limits<uint32_t>::max()) {
			PushValue(mOutputString, '\xC6', static_cast<uint32_t>(binarySize));
		}
		else {
			throw SerializationException(SerializationErrorCode::OutOfRange, "Binary size is too large");
		}
	}
}
