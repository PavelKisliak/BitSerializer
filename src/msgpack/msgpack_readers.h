/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/msgpack_archive.h"

namespace BitSerializer::MsgPack::Detail
{
	class CMsgPackStringReader final : public IMsgPackReader
	{
	public:
		CMsgPackStringReader(std::string_view inputData, OverflowNumberPolicy overflowNumberPolicy, MismatchedTypesPolicy mismatchedTypesPolicy);

		[[nodiscard]] size_t GetPosition() const noexcept override { return mPos; }
		void SetPosition(size_t pos) override;
		[[nodiscard]] ValueType ReadValueType() const override;
		[[nodiscard]] bool IsEnd() const override { return mPos == mInputData.size(); }

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

		bool ReadArraySize(size_t& arraySize) override;
		bool ReadMapSize(size_t& arraySize) override;

		void SkipValue() override;

	private:
		size_t mPos = 0;
		std::string_view mInputData;
		OverflowNumberPolicy mOverflowNumberPolicy;
		MismatchedTypesPolicy mMismatchedTypesPolicy;
	};
}
