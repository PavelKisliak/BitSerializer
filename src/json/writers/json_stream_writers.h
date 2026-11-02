/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "json_writer_common.h"
#include "bitserializer/json_archive.h"
#include "bitserializer/conversion_detail/convert_utf.h"
#include "common/encoded_stream_writer.h"

namespace BitSerializer::Json::Detail
{
	class JsonStreamWriter final : public IJsonWriter
	{
	public:
		JsonStreamWriter(std::ostream& outputStream, const StreamOptions& streamOptions,
			Convert::Utf::UtfEncodingErrorPolicy encodingErrorPolicy = Convert::Utf::UtfEncodingErrorPolicy::Skip);

		void WriteValue(std::nullptr_t) override {
			mEncodedStream.Write("null");
		}

		void WriteValue(bool value) override
		{
			if (value) {
				mEncodedStream.Write("true");
			}
			else {
				mEncodedStream.Write("false");
			}
		}

		void WriteValue(uint8_t value) override { WriteNumber(value); }
		void WriteValue(uint16_t value) override { WriteNumber(value); }
		void WriteValue(uint32_t value) override { WriteNumber(value); }
		void WriteValue(uint64_t value) override { WriteNumber(value); }

		void WriteValue(int8_t value) override { WriteNumber(value); }
		void WriteValue(int16_t value) override { WriteNumber(value); }
		void WriteValue(int32_t value) override { WriteNumber(value); }
		void WriteValue(int64_t value) override { WriteNumber(value); }

		void WriteValue(float value) override { WriteNumber(value); }
		void WriteValue(double value) override { WriteNumber(value); }
		void WriteValue(long double value) override { WriteNumber(value); }

		void WriteValue(const char* value) override { WriteValue(std::string_view(value)); }
		void WriteValue(std::string_view value) override;

		void WriteRawValue(std::string_view value) override {
			mEncodedStream.Write(value);
		}

		void WriteValueSeparator() override {
			mEncodedStream.Write(",");
		}

		void BeginArray() override {
			mEncodedStream.Write("[");
		}
		void EndArray(bool /*hasElements*/) override {
			mEncodedStream.Write("]");
		}

		void BeginObject() override {
			mEncodedStream.Write("{");
		}
		void WriteKey(std::string_view key) override {
			WriteValue(key);
			mEncodedStream.Write(":");
		}
		void WriteRawKey(std::string_view key) override {
			mEncodedStream.Write("\"");
			mEncodedStream.Write(key);
			mEncodedStream.Write("\":");
		}
		void EndObject(bool /*hasElements*/) override {
			mEncodedStream.Write("}");
		}

		void Flush() override { mEncodedStream.Flush(); }

	private:
		template <typename T>
		void WriteNumber(T value)
		{
			if constexpr (std::is_floating_point_v<T>)
			{
				if (IsNonFinite(value))
				{
					throw SerializationException(SerializationErrorCode::OutOfRange,
						"JSON does not support non-finite numbers (NaN or Infinity)");
				}
			}
			char buffer[GetMaxNumberLength<T>()];
			const size_t length = FormatNumber(buffer, value);
			mEncodedStream.Write(std::string_view(buffer, length));
		}

		Convert::Utf::EncodedStreamWriter mEncodedStream;
	};

	class JsonStreamPrettyWriter final : public IJsonWriter
	{
	public:
		JsonStreamPrettyWriter(std::ostream& outputStream, const StreamOptions& streamOptions, char paddingChar = '\t',
			uint16_t paddingCharNum = 1, Convert::Utf::UtfEncodingErrorPolicy encodingErrorPolicy = Convert::Utf::UtfEncodingErrorPolicy::Skip);

		void WriteValue(std::nullptr_t) override
		{
			WriteIndent();
			mEncodedStream.Write("null");
		}

		void WriteValue(bool value) override
		{
			WriteIndent();
			if (value) {
				mEncodedStream.Write("true");
			}
			else {
				mEncodedStream.Write("false");
			}
		}

		void WriteValue(uint8_t value) override
		{
			WriteIndent();
			WriteNumber(value);
		}
		void WriteValue(uint16_t value) override
		{
			WriteIndent();
			WriteNumber(value);
		}
		void WriteValue(uint32_t value) override
		{
			WriteIndent();
			WriteNumber(value);
		}
		void WriteValue(uint64_t value) override
		{
			WriteIndent();
			WriteNumber(value);
		}

		void WriteValue(int8_t value) override
		{
			WriteIndent();
			WriteNumber(value);
		}
		void WriteValue(int16_t value) override
		{
			WriteIndent();
			WriteNumber(value);
		}
		void WriteValue(int32_t value) override
		{
			WriteIndent();
			WriteNumber(value);
		}
		void WriteValue(int64_t value) override
		{
			WriteIndent();
			WriteNumber(value);
		}

		void WriteValue(float value) override
		{
			WriteIndent();
			WriteNumber(value);
		}
		void WriteValue(double value) override
		{
			WriteIndent();
			WriteNumber(value);
		}
		void WriteValue(long double value) override
		{
			WriteIndent();
			WriteNumber(value);
		}

		void WriteValue(const char* value) override
		{
			WriteValue(std::string_view(value));
		}
		void WriteValue(std::string_view value) override;

		void WriteRawValue(std::string_view value) override
		{
			WriteIndent();
			mEncodedStream.Write(value);
		}

		void WriteValueSeparator() override
		{
			mEncodedStream.Write(",");
			mPadding = true;
		}

		void BeginArray() override
		{
			WriteIndent();
			mEncodedStream.Write("[");
			mCurrentPadding += mPaddingCharNum;
			mPadding = true;
		}

		void EndArray(bool hasElements) override
		{
			assert(mCurrentPadding >= mPaddingCharNum);
			mCurrentPadding -= mPaddingCharNum;
			if (hasElements)
			{
				mPadding = true;
				WriteIndent();
			}
			mEncodedStream.Write("]");
		}

		void BeginObject() override
		{
			WriteIndent();
			mEncodedStream.Write("{");
			mCurrentPadding += mPaddingCharNum;
			mPadding = true;
		}

		void EndObject(bool hasElements) override
		{
			assert(mCurrentPadding >= mPaddingCharNum);
			mCurrentPadding -= mPaddingCharNum;
			if (hasElements)
			{
				mPadding = true;
				WriteIndent();
			}
			mEncodedStream.Write("}");
		}

		void Flush() override { mEncodedStream.Flush(); }

		void WriteKey(std::string_view key) override
		{
			WriteValue(key);
			mEncodedStream.Write(": ");
			mPadding = false;
		}
		void WriteRawKey(std::string_view key) override
		{
			WriteIndent();
			mEncodedStream.Write("\"");
			mEncodedStream.Write(key);
			mEncodedStream.Write("\": ");
			mPadding = false;
		}

	private:
		template <typename T>
		void WriteNumber(T value)
		{
			if constexpr (std::is_floating_point_v<T>)
			{
				if (IsNonFinite(value))
				{
					throw SerializationException(SerializationErrorCode::OutOfRange,
						"JSON does not support non-finite numbers (NaN or Infinity)");
				}
			}
			char buffer[GetMaxNumberLength<T>()];
			const size_t length = FormatNumber(buffer, value);
			mEncodedStream.Write(std::string_view(buffer, length));
		}

		void WriteIndent()
		{
			if (mPadding)
			{
				mEncodedStream.Write("\n");
				if (mCurrentPadding > 0)
				{
					static constexpr std::string_view tabs = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
					static constexpr std::string_view spaces = "                ";
					const std::string_view padding = mPaddingChar == '\t' ? tabs : spaces;
					uint32_t remaining = mCurrentPadding;
					while (remaining > 0)
					{
						const size_t chunkSize = remaining < padding.size() ? remaining : padding.size();
						mEncodedStream.Write(padding.substr(0, chunkSize));
						remaining -= static_cast<uint32_t>(chunkSize);
					}
				}
			}
		}

		Convert::Utf::EncodedStreamWriter mEncodedStream;
		uint32_t mCurrentPadding = 0;
		uint16_t mPaddingCharNum;
		char mPaddingChar;
		bool mPadding = false;
	};
}
