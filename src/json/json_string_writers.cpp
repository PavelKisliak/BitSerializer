/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "json_string_writers.h"

namespace
{
	using namespace BitSerializer;

	void WriteString(std::string_view source, std::string& target)
	{
		size_t extra = 0;
		bool has_escapes = false;

		for (char c : source)
		{
			unsigned char uc = static_cast<unsigned char>(c);
			if (uc == '"' || uc == '\\')
			{
				extra += 1;
				has_escapes = true;
			}
			else if (uc <= 0x1F)
			{
				has_escapes = true;
				switch (uc) {
				case '\b': case '\f': case '\n': case '\r': case '\t':
					extra += 1;  // 2-char escape vs 1 char
					break;
				default:
					extra += 5;  // \uXXXX (6 chars vs 1 char)
					break;
				}
			}
		}

		const size_t orig_size = target.size();
		target.reserve(orig_size + 2 + source.size() + extra);

		target.push_back('"');

		// FAST PATH: No escapes needed (common case for keys/values)
		if (!has_escapes) {
			target.append(source);
			target.push_back('"');
			return;
		}

		// SLOW PATH: Handle escapes with branch-predictor friendly loop
		static constexpr char hex[] = "0123456789ABCDEF";
		for (char c : source)
		{
			unsigned char uc = static_cast<unsigned char>(c);
			switch (uc) {
			case '"':  target.append("\\\""); break;
			case '\\': target.append("\\\\"); break;
			case '\b': target.append("\\b"); break;
			case '\f': target.append("\\f"); break;
			case '\n': target.append("\\n"); break;
			case '\r': target.append("\\r"); break;
			case '\t': target.append("\\t"); break;
			default:
				if (uc <= 0x1F) {
					char buf[7] = { '\\', 'u', '0', '0', hex[uc >> 4], hex[uc & 0x0F], '\0' };
					target.append(buf, 6);
				}
				else {
					target.push_back(c);
				}
				break;
			}
		}

		target.push_back('"');
	}
}

namespace BitSerializer::Json::Detail
{
	CJsonStringWriter::CJsonStringWriter(std::string& outputString)
		: mOutputString(outputString)
	{
	}

	void CJsonStringWriter::WriteValue(std::string_view value)
	{
		WriteString(value, mOutputString);

		//mOutputString.push_back('"');

		//static constexpr char hex[] = "0123456789ABCDEF";
		//auto endPtr = value.data() + value.size();
		//for (auto chPtr = value.data(); chPtr < endPtr; ++chPtr)
		//{
		//	char c = *chPtr;
		//	unsigned char uc = static_cast<unsigned char>(c);
		//	if (uc <= 0x1F)
		//	{
		//		switch (c)
		//		{
		//		case '\n': mOutputString.append("\\n"); break;
		//		case '\r': mOutputString.append("\\r"); break;
		//		case '\t': mOutputString.append("\\t"); break;
		//		case '\b': mOutputString.append("\\b"); break;
		//		case '\f': mOutputString.append("\\f"); break;
		//		default:
		//			mOutputString.append({ '\\', 'u', '0', '0', hex[(uc >> 4) & 0x0F], hex[uc & 0x0F] });
		//		}
		//	}
		//	else if (c == '"') {
		//		mOutputString.append("\\\"");
		//	}
		//	else if (c == '\\') {
		//		mOutputString.append("\\\\");
		//	}
		//	else {
		//		mOutputString.push_back(c);
		//	}
		//}

		//mOutputString.push_back('"');
	}

	//------------------------------------------------------------------------------
	CJsonStringPrettyWriter::CJsonStringPrettyWriter(std::string& outputString, char paddingChar, uint16_t paddingCharNum)
		: mOutputString(outputString)
		, mPaddingCharNum(paddingCharNum)
		, mPaddingChar(paddingChar)
	{
	}

	void CJsonStringPrettyWriter::WriteValue(std::string_view value)
	{
		WriteIndent();
		WriteString(value, mOutputString);
	}

	//------------------------------------------------------------------------------

	//CJsonStreamWriter::CJsonStreamWriter(std::ostream& outputStream)
	//	: mOutputStream(outputStream)
	//{
	//}

	//void CJsonStreamWriter::WriteValue(std::nullptr_t)
	//{
	//	mOutputStream.put('\xC0');
	//}

	//void CJsonStreamWriter::WriteValue(bool value)
	//{
	//	mOutputStream.put(value ? '\xC3' : '\xC2');
	//}

	//void CJsonStreamWriter::WriteValue(uint8_t value)
	//{
	//	if (value >= 128u) {
	//		mOutputStream.put('\xCC');
	//	}
	//	mOutputStream.put(static_cast<char>(value));
	//}

	//void CJsonStreamWriter::WriteValue(uint16_t value)
	//{
	//	if (value > std::numeric_limits<uint8_t>::max()) {
	//		PushValue(mOutputStream, '\xCD', value);
	//	}
	//	else {
	//		WriteValue(static_cast<uint8_t>(value));
	//	}
	//}

	//void CJsonStreamWriter::WriteValue(uint32_t value)
	//{
	//	if (value > std::numeric_limits<uint16_t>::max()) {
	//		PushValue(mOutputStream, '\xCE', value);
	//	}
	//	else {
	//		WriteValue(static_cast<uint16_t>(value));
	//	}
	//}

	//void CJsonStreamWriter::WriteValue(uint64_t value)
	//{
	//	if (value > std::numeric_limits<uint32_t>::max()) {
	//		PushValue(mOutputStream, '\xCF', value);
	//	}
	//	else {
	//		WriteValue(static_cast<uint32_t>(value));
	//	}
	//}

	//void CJsonStreamWriter::WriteValue(int8_t value)
	//{
	//	if (value >= -32) {
	//		mOutputStream.put(value);
	//	}
	//	else {
	//		PushValue(mOutputStream, '\xD0', value);
	//	}
	//}

	//void CJsonStreamWriter::WriteValue(int16_t value)
	//{
	//	if (value < std::numeric_limits<int8_t>::min() || value > std::numeric_limits<int8_t>::max()) {
	//		PushValue(mOutputStream, '\xD1', value);
	//	}
	//	else {
	//		WriteValue(static_cast<int8_t>(value));
	//	}
	//}

	//void CJsonStreamWriter::WriteValue(int32_t value)
	//{
	//	if (value < std::numeric_limits<int16_t>::min() || value > std::numeric_limits<int16_t>::max()) {
	//		PushValue(mOutputStream, '\xD2', value);
	//	}
	//	else {
	//		WriteValue(static_cast<int16_t>(value));
	//	}
	//}

	//void CJsonStreamWriter::WriteValue(int64_t value)
	//{
	//	if (value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max()) {
	//		PushValue(mOutputStream, '\xD3', value);
	//	}
	//	else {
	//		WriteValue(static_cast<int32_t>(value));
	//	}
	//}

	//void CJsonStreamWriter::WriteValue(float value)
	//{
	//	uint32_t buf;
	//	std::memcpy(&buf, &value, sizeof(uint32_t));
	//	PushValue(mOutputStream, '\xCA', buf);
	//}

	//void CJsonStreamWriter::WriteValue(double value)
	//{
	//	uint64_t buf;
	//	std::memcpy(&buf, &value, sizeof(uint64_t));
	//	PushValue(mOutputStream, '\xCB', buf);
	//}

	//void CJsonStreamWriter::WriteValue(std::string_view value)
	//{
	//	if (value.size() < 32u) {
	//		mOutputStream.put(static_cast<char>(static_cast<uint8_t>(value.size()) | 0b10100000u));
	//	}
	//	else
	//	{
	//		if (value.size() <= std::numeric_limits<uint8_t>::max()) {
	//			PushValue(mOutputStream, '\xD9', static_cast<uint8_t>(value.size()));
	//		}
	//		else if (value.size() <= std::numeric_limits<uint16_t>::max()) {
	//			PushValue(mOutputStream, '\xDA', static_cast<uint16_t>(value.size()));
	//		}
	//		else if (value.size() <= std::numeric_limits<uint32_t>::max()) {
	//			PushValue(mOutputStream, '\xDB', static_cast<uint32_t>(value.size()));
	//		}
	//		else {
	//			throw SerializationException(SerializationErrorCode::OutOfRange, "String size is too large");
	//		}
	//	}
	//	mOutputStream.write(value.data(), static_cast<std::streamsize>(value.size()));
	//}

	//void CJsonStreamWriter::WriteValue(const CBinTimestamp& timestamp)
	//{
	//	if (static_cast<uint64_t>(timestamp.Seconds) >> 34u == 0u)
	//	{
	//		const uint64_t data64 = (static_cast<uint64_t>(timestamp.Nanoseconds) << 34u) | static_cast<uint64_t>(timestamp.Seconds);
	//		if ((data64 & 0xFFFFFFFF00000000ul) == 0)
	//		{
	//			// timestamp 32
	//			mOutputStream.put('\xD6');
	//			PushValue(mOutputStream, -1, static_cast<uint32_t>(data64));
	//		}
	//		else
	//		{
	//			// timestamp 64
	//			mOutputStream.put('\xD7');
	//			PushValue(mOutputStream, -1, data64);
	//		}
	//	}
	//	else
	//	{
	//		mOutputStream.put('\xC7');
	//		mOutputStream.put(12);
	//		// timestamp 96
	//		PushValue(mOutputStream, -1, timestamp.Seconds);
	//		PushValue(mOutputStream, timestamp.Nanoseconds);
	//	}
	//}

	//void CJsonStreamWriter::BeginArray(size_t arraySize)
	//{
	//	if (arraySize < 16u) {
	//		mOutputStream.put(static_cast<char>(static_cast<uint8_t>(arraySize) | 0b10010000u));
	//	}
	//	else
	//	{
	//		if (arraySize <= std::numeric_limits<uint16_t>::max()) {
	//			PushValue(mOutputStream, '\xDC', static_cast<uint16_t>(arraySize));
	//		}
	//		else if (arraySize <= std::numeric_limits<uint32_t>::max()) {
	//			PushValue(mOutputStream, '\xDD', static_cast<uint32_t>(arraySize));
	//		}
	//		else {
	//			throw SerializationException(SerializationErrorCode::OutOfRange, "Array size is too large");
	//		}
	//	}
	//}

	//void CJsonStreamWriter::BeginMap(size_t mapSize)
	//{
	//	if (mapSize < 16u) {
	//		mOutputStream.put(static_cast<char>(static_cast<uint8_t>(mapSize) | 0b10000000u));
	//	}
	//	else
	//	{
	//		if (mapSize <= std::numeric_limits<uint16_t>::max()) {
	//			PushValue(mOutputStream, '\xDE', static_cast<uint16_t>(mapSize));
	//		}
	//		else if (mapSize <= std::numeric_limits<uint32_t>::max()) {
	//			PushValue(mOutputStream, '\xDF', static_cast<uint32_t>(mapSize));
	//		}
	//		else {
	//			throw SerializationException(SerializationErrorCode::OutOfRange, "Object size is too large");
	//		}
	//	}
	//}

	//void CJsonStreamWriter::BeginBinary(size_t binarySize)
	//{
	//	if (binarySize <= std::numeric_limits<uint8_t>::max()) {
	//		PushValue(mOutputStream, '\xC4', static_cast<uint8_t>(binarySize));
	//	}
	//	else if (binarySize <= std::numeric_limits<uint16_t>::max()) {
	//		PushValue(mOutputStream, '\xC5', static_cast<uint16_t>(binarySize));
	//	}
	//	else if (binarySize <= std::numeric_limits<uint32_t>::max()) {
	//		PushValue(mOutputStream, '\xC6', static_cast<uint32_t>(binarySize));
	//	}
	//	else {
	//		throw SerializationException(SerializationErrorCode::OutOfRange, "Binary size is too large");
	//	}
	//}

	//void CJsonStreamWriter::WriteBinary(char byte)
	//{
	//	mOutputStream.put(byte);
	//}
}
