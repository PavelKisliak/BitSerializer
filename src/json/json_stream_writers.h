/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/json_archive.h"
#include "bitserializer/conversion_detail/convert_utf.h"
#include "common/encoded_stream_writer.h"

namespace BitSerializer::Json::Detail
{
	class CJsonStreamWriter final : public IJsonWriter
	{
	public:
		CJsonStreamWriter(std::ostream& outputStream, const StreamOptions& streamOptions,
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

		void WriteValue(JsonArchiveTraits::raw_type& value) override {
			mEncodedStream.Write(value.Get());
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
		void EndObject(bool /*hasElements*/) override {
			mEncodedStream.Write("}");
		}

	private:
		template <typename T>
		void WriteNumber(T value)
		{
			std::string buf;
			Convert::Detail::To(value, buf);
			mEncodedStream.Write(buf);
		}

		Convert::Utf::EncodedStreamWriter mEncodedStream;
		std::string mStringBuffer;
	};

	class CJsonStreamPrettyWriter final : public IJsonWriter
	{
	public:
		CJsonStreamPrettyWriter(std::ostream& outputStream, const StreamOptions& streamOptions, char paddingChar = '\t',
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

		void WriteValue(JsonArchiveTraits::raw_type& value) override
		{
			WriteIndent();
			mEncodedStream.Write(value.Get());
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

		void WriteKey(std::string_view key) override
		{
			WriteValue(key);
			mEncodedStream.Write(": ");
			mPadding = false;
		}

	private:
		template <typename T>
		void WriteNumber(T value)
		{
			std::string buf;
			Convert::Detail::To(value, buf);
			mEncodedStream.Write(buf);
		}

		void WriteIndent()
		{
			if (mPadding)
			{
				std::string indent;
				indent.push_back('\n');
				if (mCurrentPadding > 0) {
					indent.append(mCurrentPadding, mPaddingChar);
				}
				mEncodedStream.Write(indent);
			}
		}

		Convert::Utf::EncodedStreamWriter mEncodedStream;
		std::string mStringBuffer;
		uint32_t mCurrentPadding = 0;
		uint16_t mPaddingCharNum;
		char mPaddingChar;
		bool mPadding = false;
	};
}
