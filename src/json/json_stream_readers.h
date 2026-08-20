/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/json_archive.h"
#include "common/encoded_stream_reader.h"


namespace BitSerializer::Json::Detail
{
	class CJsonStreamReader final : public IJsonReader
	{
	public:
		CJsonStreamReader(std::istream& inputStream, const SerializationOptions& serializationOptions);

		[[nodiscard]] size_t GetPosition() const noexcept override { return mEncodedStreamReader.GetPosition(); }
		void SetPosition(size_t pos) override;
		[[nodiscard]] size_t GetLineNumber() const noexcept override { return mLineNumber; }
		[[nodiscard]] bool IsEnd() const override { return mEncodedStreamReader.IsEnd(); }

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

		void ReadValueSeparator() override;

		bool OpenArray() override;
		bool IsArrayEnd() override;
		void CloseArray(bool expectedComma) override;

		bool OpenObject() override;
		bool IsObjectEnd() override;
		void CloseObject(bool expectedComma) override;

	private:
		ValueType ReadValueTypeImpl();
		bool ReadStringImpl(std::string& outValue);
		template <typename T>
		bool ReadNumberImpl(T& value);
		void SkipValueImpl();
		void HandleMismatchedTypesPolicyStream();

		Convert::Utf::EncodedStreamReader<char> mEncodedStreamReader;
		std::string mKeyBuffer;
		std::string mValueBuffer;
		size_t mLineNumber = 1;
		const SerializationOptions& mSerializationOptions;
	};
}
