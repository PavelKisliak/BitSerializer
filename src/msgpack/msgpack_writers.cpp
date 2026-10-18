/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "msgpack_writers.h"
#include "bitserializer/common/memory.h"

namespace
{
	using namespace BitSerializer;

	template <typename T, std::enable_if_t<sizeof(T) == 1 && std::is_integral_v<T>, int> = 0>
	void PushValue(std::string& outputString, char code, T value)
	{
		outputString.push_back(static_cast<char>(static_cast<uint8_t>(code)));
		outputString.push_back(static_cast<char>(value));
	}

	template <typename T, std::enable_if_t<sizeof(T) >= 2 && std::is_integral_v<T>, int> = 0>
	void PushValue(std::string& outputString, char code, T value)
	{
		outputString.push_back(static_cast<char>(static_cast<uint8_t>(code)));
		const T networkVal = Memory::NativeToBigEndian(value);
		outputString.append(reinterpret_cast<const char*>(&networkVal), sizeof(T));
	}

	template <typename T, std::enable_if_t<sizeof(T) >= 2 && std::is_integral_v<T>, int> = 0>
	void PushValue(std::string& outputString, T value)
	{
		const T networkVal = Memory::NativeToBigEndian(value);
		outputString.append(reinterpret_cast<const char*>(&networkVal), sizeof(T));
	}
}

namespace BitSerializer::MsgPack::Detail
{
	MsgPackStringWriter::MsgPackStringWriter(std::string& outputString)
		: mOutputString(outputString)
	{ }

	void MsgPackStringWriter::WriteValue(std::nullptr_t)
	{
		mOutputString.push_back('\xC0');
	}

	void MsgPackStringWriter::WriteValue(bool value)
	{
		mOutputString.push_back(value ? '\xC3' : '\xC2');
	}

	void MsgPackStringWriter::WriteValue(uint8_t value)
	{
		if (value >= 128u) {
			mOutputString.push_back('\xCC');
		}
		mOutputString.push_back(static_cast<char>(value));
	}

	void MsgPackStringWriter::WriteValue(uint16_t value)
	{
		if (value > std::numeric_limits<uint8_t>::max()) {
			PushValue(mOutputString, '\xCD', value);
		}
		else {
			WriteValue(static_cast<uint8_t>(value));
		}
	}

	void MsgPackStringWriter::WriteValue(uint32_t value)
	{
		if (value > std::numeric_limits<uint16_t>::max()) {
			PushValue(mOutputString, '\xCE', value);
		}
		else {
			WriteValue(static_cast<uint16_t>(value));
		}
	}

	void MsgPackStringWriter::WriteValue(uint64_t value)
	{
		if (value > std::numeric_limits<uint32_t>::max()) {
			PushValue(mOutputString, '\xCF', value);
		}
		else {
			WriteValue(static_cast<uint32_t>(value));
		}
	}

	void MsgPackStringWriter::WriteValue(int8_t value)
	{
		if (value >= -32) {
			mOutputString.push_back(value);
		}
		else {
			PushValue(mOutputString, '\xD0', value);
		}
	}

	void MsgPackStringWriter::WriteValue(int16_t value)
	{
		if (value < std::numeric_limits<int8_t>::min() || value > std::numeric_limits<int8_t>::max()) {
			PushValue(mOutputString, '\xD1', value);
		}
		else {
			WriteValue(static_cast<int8_t>(value));
		}
	}

	void MsgPackStringWriter::WriteValue(int32_t value)
	{
		if (value < std::numeric_limits<int16_t>::min() || value > std::numeric_limits<int16_t>::max()) {
			PushValue(mOutputString, '\xD2', value);
		}
		else {
			WriteValue(static_cast<int16_t>(value));
		}
	}

	void MsgPackStringWriter::WriteValue(int64_t value)
	{
		if (value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max()) {
			PushValue(mOutputString, '\xD3', value);
		}
		else {
			WriteValue(static_cast<int32_t>(value));
		}
	}

	void MsgPackStringWriter::WriteValue(float value)
	{
		uint32_t buf;
		std::memcpy(&buf, &value, sizeof(uint32_t));
		PushValue(mOutputString, '\xCA', buf);
	}

	void MsgPackStringWriter::WriteValue(double value)
	{
		uint64_t buf;
		std::memcpy(&buf, &value, sizeof(uint64_t));
		PushValue(mOutputString, '\xCB', buf);
	}

	void MsgPackStringWriter::WriteValue(std::string_view value)
	{
		if (value.size() < 32u) {
			mOutputString.push_back(static_cast<char>(static_cast<uint8_t>(value.size()) | 0b10100000u));
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

	void MsgPackStringWriter::WriteValue(const BinTimestamp& timestamp)
	{
		if (static_cast<uint64_t>(timestamp.seconds) >> 34u == 0u)
		{
			const uint64_t data64 = (static_cast<uint64_t>(timestamp.nanoseconds) << 34u) | static_cast<uint64_t>(timestamp.seconds);
			if ((data64 & 0xFFFFFFFF00000000ul) == 0u)
			{
				// timestamp 32
				mOutputString.push_back('\xD6');
				PushValue(mOutputString, -1, static_cast<uint32_t>(data64));
			}
			else
			{
				// timestamp 64
				mOutputString.push_back('\xD7');
				PushValue(mOutputString, -1, data64);
			}
		}
		else
		{
			mOutputString.push_back('\xC7');
			mOutputString.push_back(12);
			// timestamp 96
			PushValue(mOutputString, -1, timestamp.seconds);
			PushValue(mOutputString, timestamp.nanoseconds);
		}
	}

	void MsgPackStringWriter::WriteValue(MsgPackArchiveTraits::raw_type& value)
	{
		const auto& bytes = value.Get();
		mOutputString.append(reinterpret_cast<const char*>(bytes.data()), bytes.size());
	}

	void MsgPackStringWriter::BeginArray(size_t arraySize)
	{
		if (arraySize < 16u) {
			mOutputString.push_back(static_cast<char>(static_cast<uint8_t>(arraySize) | 0b10010000u));
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

	void MsgPackStringWriter::BeginMap(size_t mapSize)
	{
		if (mapSize < 16u) {
			mOutputString.push_back(static_cast<char>(static_cast<uint8_t>(mapSize) | 0b10000000u));
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

	void MsgPackStringWriter::BeginBinary(size_t binarySize)
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

	//------------------------------------------------------------------------------

	MsgPackStreamWriter::MsgPackStreamWriter(std::ostream& outputStream)
		: mOutputStream(outputStream)
		, mStringWriter(mByteBuffer)
	{
		mByteBuffer.reserve(OutputBufferSize);
	}

	void MsgPackStreamWriter::WriteValue(std::nullptr_t value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(bool value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(uint8_t value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(uint16_t value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(uint32_t value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(uint64_t value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(int8_t value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(int16_t value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(int32_t value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(int64_t value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(float value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(double value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(std::string_view value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(const BinTimestamp& timestamp)
	{
		mStringWriter.WriteValue(timestamp);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteValue(MsgPackArchiveTraits::raw_type& value)
	{
		mStringWriter.WriteValue(value);
		FlushIfFull();
	}

	void MsgPackStreamWriter::BeginArray(size_t arraySize)
	{
		mStringWriter.BeginArray(arraySize);
		FlushIfFull();
	}

	void MsgPackStreamWriter::BeginMap(size_t mapSize)
	{
		mStringWriter.BeginMap(mapSize);
		FlushIfFull();
	}

	void MsgPackStreamWriter::BeginBinary(size_t binarySize)
	{
		mStringWriter.BeginBinary(binarySize);
		FlushIfFull();
	}

	void MsgPackStreamWriter::WriteBinary(char byte)
	{
		mStringWriter.WriteBinary(byte);
		FlushIfFull();
	}

	void MsgPackStreamWriter::Flush()
	{
		if (!mByteBuffer.empty())
		{
			mOutputStream.write(mByteBuffer.data(), static_cast<std::streamsize>(mByteBuffer.size()));
			mByteBuffer.clear();
		}
	}
}
