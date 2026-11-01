/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "json_writer_common.h"
#include "bitserializer/json_archive.h"

namespace BitSerializer::Json::Detail
{
	/**
	 * @brief JSON writer that serializes into a caller-provided string.
	 *
	 * This is a one-shot writer: it owns the output segment until Flush(). Call Flush()
	 * (or destroy the writer) once; do not reuse the instance for another document.
	 * The destructor finalizes the output if Flush() was not called.
	 */
	class JsonStringWriter final : public IJsonWriter
	{
	public:
		explicit JsonStringWriter(std::string& outputString);
		~JsonStringWriter() override;

		void WriteValue(std::nullptr_t) override {
			mBuffer.AppendLiteral("null");
		}

		void WriteValue(bool value) override {
			if (value) {
				mBuffer.AppendLiteral("true");
			}
			else {
				mBuffer.AppendLiteral("false");
			}
		}

		void WriteValue(uint8_t value) override {
			WriteNumber(value);
		}
		void WriteValue(uint16_t value) override {
			WriteNumber(value);
		}
		void WriteValue(uint32_t value) override {
			WriteNumber(value);
		}
		void WriteValue(uint64_t value) override {
			WriteNumber(value);
		}

		void WriteValue(int8_t value) override {
			WriteNumber(value);
		}
		void WriteValue(int16_t value) override {
			WriteNumber(value);
		}
		void WriteValue(int32_t value) override {
			WriteNumber(value);
		}
		void WriteValue(int64_t value) override {
			WriteNumber(value);
		}

		void WriteValue(float value) override {
			WriteNumber(value);
		}
		void WriteValue(double value) override {
			WriteNumber(value);
		}
		void WriteValue(long double value) override {
			WriteNumber(value);
		}

		void WriteValue(const char* value) override {
			WriteValue(std::string_view(value));
		}
		void WriteValue(std::string_view value) override;

		void WriteRawValue(std::string_view value) override {
			mBuffer.Append(value);
		}

		void WriteValueSeparator() override {
			mBuffer.PushUnsafe(',');
		}

		void BeginArray() override {
			mBuffer.Push('[');
		}
		void EndArray(bool /*hasElements*/) override {
			mBuffer.Push(']');
		}

		void BeginObject() override {
			mBuffer.Push('{');
		}
		void WriteKey(std::string_view key) override
		{
			WriteValue(key);
			mBuffer.PushUnsafe(':');
		}
		void WriteRawKey(std::string_view key) override
		{
			const size_t size = key.size() + 3;
			mBuffer.Ensure(size);
			char* out = mBuffer.Tail();
			*out++ = '"';
			if (!key.empty()) {
				std::memcpy(out, key.data(), key.size());
			}
			out += key.size();
			*out++ = '"';
			*out++ = ':';
			mBuffer.Commit(size);
		}
		void EndObject(bool /*hasElements*/) override {
			mBuffer.Push('}');
		}

		void Flush() override
		{
			mBuffer.Finalize();
			mFlushed = true;
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

			WriteNumberTo(mBuffer, value);
		}

		StringBuffer mBuffer;
		bool mFlushed = false;
	};

	/**
	 * @brief Pretty-printing JSON writer that serializes into a caller-provided string.
	 *
	 * Like JsonStringWriter, this is a one-shot writer. Flush() finalizes the output;
	 * the destructor does it if Flush() was not called. Recreate the writer for the next
	 * document.
	 */
	class JsonStringPrettyWriter final : public IJsonWriter
	{
	public:
		explicit JsonStringPrettyWriter(std::string& outputString, char paddingChar = '\t', uint16_t paddingCharNum = 1);
		~JsonStringPrettyWriter() override;

		void WriteValue(std::nullptr_t) override
		{
			WriteIndent();
			mBuffer.AppendLiteral("null");
		}

		void WriteValue(bool value) override
		{
			WriteIndent();
			if (value) {
				mBuffer.AppendLiteral("true");
			}
			else {
				mBuffer.AppendLiteral("false");
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

		void WriteValue(const char* value) override { WriteValue(std::string_view(value)); }
		void WriteValue(std::string_view value) override;

		void WriteRawValue(std::string_view value) override
		{
			WriteIndent();
			mBuffer.Append(value);
		}

		void WriteValueSeparator() override
		{
			mBuffer.Push(',');
			mPadding = true;
		}

		void BeginArray() override
		{
			WriteIndent();
			mBuffer.Push('[');
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
			mBuffer.Push(']');
		}

		void BeginObject() override
		{
			WriteIndent();
			mBuffer.Push('{');
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
			mBuffer.Push('}');
		}

		void WriteKey(std::string_view key) override
		{
			WriteValue(key);
			mBuffer.AppendLiteral(": ");
			mPadding = false;
		}
		void WriteRawKey(std::string_view key) override
		{
			WriteIndent();
			mBuffer.AppendLiteral("\"");
			mBuffer.Append(key);
			mBuffer.AppendLiteral("\": ");
			mPadding = false;
		}

		void Flush() override {
			mBuffer.Finalize();
			mFlushed = true;
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

			WriteNumberTo(mBuffer, value);
		}

		void WriteIndent()
		{
			if (mPadding)
			{
				mBuffer.Push('\n');
				if (mCurrentPadding > 0) {
					mBuffer.Append(mPaddingChar, mCurrentPadding);
				}
			}
		}

		StringBuffer mBuffer;
		bool mFlushed = false;
		uint32_t mCurrentPadding = 0;
		uint16_t mPaddingCharNum;
		char mPaddingChar;
		bool mPadding = false;
	};
}
