/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/json_archive.h"


namespace BitSerializer::Json::Detail
{
	class CJsonStringReader final : public IJsonReader
	{
	public:
		CJsonStringReader(std::string_view inputData, const SerializationOptions& serializationOptions) noexcept;

		[[nodiscard]] size_t GetPosition() const noexcept override { return mPos; }
		void SetPosition(size_t pos) override;
		size_t GetLineNumber() const noexcept override { return mLineNumber; }
		[[nodiscard]] bool IsEnd() const override { return mPos == mInputData.size(); }

		void ReadKey(std::string_view& key) override;

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
		bool ReadValue(long double& value) override;

		bool ReadValue(std::string_view& value) override;

		bool ReadValue(JsonArchiveTraits::raw_type& value) override;

		[[nodiscard]] ValueType ReadValueType() override;

		void SkipValue() override;

		bool TryConsumeComma() noexcept override;

		bool OpenArray() override;
		bool IsArrayEnd() override;
		void CloseArray() override;

		bool OpenObject() override;
		bool IsObjectEnd() override;
		void CloseObject() override;

	private:
		size_t mPos = 0;
		size_t mLineNumber = 1;
		std::string_view mInputData;
		std::string mBuffer;
		const SerializationOptions& mSerializationOptions;
	};
}
