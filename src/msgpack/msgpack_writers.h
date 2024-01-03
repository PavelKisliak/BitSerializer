/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/msgpack_archive.h"

namespace BitSerializer::MsgPack::Detail
{
	class CMsgPackStringWriter final : public IMsgPackWriter
	{
	public:
		CMsgPackStringWriter(std::string& outputString);
		CMsgPackStringWriter(std::string& outputString, size_t arraySize);

		void WriteValue(std::nullptr_t) override;

		void WriteValue(bool value) override;

		void WriteValue(uint8_t value) override;
		void WriteValue(uint16_t value) override;
		void WriteValue(uint32_t value) override;
		void WriteValue(uint64_t value) override;

		void WriteValue(int8_t value) override;
		void WriteValue(int16_t value) override;
		void WriteValue(int32_t value) override;
		void WriteValue(int64_t value) override;

		void WriteValue(float value) override;
		void WriteValue(double value) override;

		void WriteValue(const char* value) override { WriteValue(std::string_view(value)); }
		void WriteValue(std::string_view value) override;

		void BeginArray(size_t arraySize) override;
		void BeginMap(size_t mapSize) override;

	private:
		std::string& mOutputString;
		size_t mArraySize = 0;
		size_t mMapSize = 0;
	};
}
