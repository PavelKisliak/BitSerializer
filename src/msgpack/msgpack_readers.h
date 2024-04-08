/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/msgpack_archive.h"
#include "common/binary_stream_reader.h"


namespace BitSerializer::MsgPack::Detail
{
	class CMsgPackStringReader final : public IMsgPackReader
	{
	public:
		CMsgPackStringReader(std::string_view inputData, const SerializationOptions& serializationOptions) noexcept;

		[[nodiscard]] size_t GetPosition() const noexcept override { return mPos; }
		void SetPosition(size_t pos) override;
		[[nodiscard]] ValueType ReadValueType() override;
		[[nodiscard]] bool IsEnd() const noexcept override { return mPos == mInputData.size(); }

		bool ReadValue(std::nullptr_t&) override;
		bool ReadValue(bool& value) override;

		bool ReadValue(uint8_t& value) override;
		bool ReadValue(uint16_t& value) override;
		bool ReadValue(uint32_t& value) override;
		bool ReadValue(uint64_t& value) override;

		bool ReadValue(char& value) override;
		bool ReadValue(int8_t& value) override;
		bool ReadValue(int16_t& value) override;
		bool ReadValue(int32_t& value) override;
		bool ReadValue(int64_t& value) override;

		bool ReadValue(float& value) override;
		bool ReadValue(double& value) override;

		bool ReadValue(std::string_view& value) override;

		bool ReadValue(CBinTimestamp& timestamp) override;

		bool ReadArraySize(size_t& arraySize) override;
		bool ReadMapSize(size_t& mapSize) override;

		bool ReadBinarySize(size_t& binarySize) override;
		char ReadBinary() override;

		void SkipValue() override;

	private:
		size_t mPos = 0;
		std::string_view mInputData;
		const SerializationOptions& mSerializationOptions;
	};

	class CMsgPackStreamReader final : public IMsgPackReader
	{
	public:
		CMsgPackStreamReader(std::istream& inputStream, const SerializationOptions& serializationOptions) noexcept;

		[[nodiscard]] size_t GetPosition() const noexcept override {
			return mBinaryStreamReader.GetPosition();
		}
		void SetPosition(size_t pos) override {
			mBinaryStreamReader.SetPosition(pos);
		}
		[[nodiscard]] bool IsEnd() const noexcept override {
			return mBinaryStreamReader.IsEnd();
		}

		[[nodiscard]] ValueType ReadValueType() override;

		bool ReadValue(std::nullptr_t&) override;
		bool ReadValue(bool& value) override;

		bool ReadValue(uint8_t& value) override;
		bool ReadValue(uint16_t& value) override;
		bool ReadValue(uint32_t& value) override;
		bool ReadValue(uint64_t& value) override;

		bool ReadValue(char& value) override;
		bool ReadValue(int8_t& value) override;
		bool ReadValue(int16_t& value) override;
		bool ReadValue(int32_t& value) override;
		bool ReadValue(int64_t& value) override;

		bool ReadValue(float& value) override;
		bool ReadValue(double& value) override;

		bool ReadValue(std::string_view& value) override;

		bool ReadValue(CBinTimestamp& timestamp) override;

		bool ReadArraySize(size_t& arraySize) override;
		bool ReadMapSize(size_t& mapSize) override;

		bool ReadBinarySize(size_t& binarySize) override;
		char ReadBinary() override;

		void SkipValue() override;

	private:
		BitSerializer::Detail::CBinaryStreamReader mBinaryStreamReader;
		const SerializationOptions& mSerializationOptions;
		std::string mBuffer;
	};
}
