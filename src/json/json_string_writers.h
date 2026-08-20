/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/json_archive.h"

namespace BitSerializer::Json::Detail
{
	class CJsonStringWriter final : public IJsonWriter
	{
	public:
		explicit CJsonStringWriter(std::string& outputString);

		void WriteValue(std::nullptr_t) override {
			mOutputString.append("null");
		}

		void WriteValue(bool value) override {
			mOutputString.append(value ? "true" : "false");
		}

		void WriteValue(uint8_t value) override {
			Convert::Detail::To(value, mOutputString);
		}
		void WriteValue(uint16_t value) override {
			Convert::Detail::To(value, mOutputString);
		}
		void WriteValue(uint32_t value) override {
			Convert::Detail::To(value, mOutputString);
		}
		void WriteValue(uint64_t value) override {
			Convert::Detail::To(value, mOutputString);
		}

		void WriteValue(int8_t value) override {
			Convert::Detail::To(value, mOutputString);
		}
		void WriteValue(int16_t value) override {
			Convert::Detail::To(value, mOutputString);
		}
		void WriteValue(int32_t value) override {
			Convert::Detail::To(value, mOutputString);
		}
		void WriteValue(int64_t value) override {
			Convert::Detail::To(value, mOutputString);
		}

		void WriteValue(float value) override {
			Convert::Detail::To(value, mOutputString);
		}
		void WriteValue(double value) override {
			Convert::Detail::To(value, mOutputString);
		}
		void WriteValue(long double value) override {
			Convert::Detail::To(value, mOutputString);
		}

		void WriteValue(const char* value) override {
			WriteValue(std::string_view(value));
		}
		void WriteValue(std::string_view value) override;

		void WriteValue(JsonArchiveTraits::raw_type& value) override {
			mOutputString.append(value.Get());
		}

		void WriteValueSeparator() override {
			mOutputString.push_back(',');
		}

		void BeginArray() override {
			mOutputString.push_back('[');
		}
		void EndArray(bool /*hasElements*/) override {
			mOutputString.push_back(']');
		}

		void BeginObject() override {
			mOutputString.push_back('{');
		}
		void WriteKey(std::string_view key) override
		{
			WriteValue(key);
			mOutputString.push_back(':');
		}
		void EndObject(bool /*hasElements*/) override {
			mOutputString.push_back('}');
		}

	private:
		std::string& mOutputString;
	};

	class CJsonStringPrettyWriter final : public IJsonWriter
	{
	public:
		explicit CJsonStringPrettyWriter(std::string& outputString, char paddingChar = '\t', uint16_t paddingCharNum = 1);

		void WriteValue(std::nullptr_t) override
		{
			WriteIndent();
			mOutputString.append("null");
		}

		void WriteValue(bool value) override
		{
			WriteIndent();
			mOutputString.append(value ? "true" : "false");
		}

		void WriteValue(uint8_t value) override
		{
			WriteIndent();
			Convert::Detail::To(value, mOutputString);
		}
		void WriteValue(uint16_t value) override
		{
			WriteIndent();
			Convert::Detail::To(value, mOutputString);
		}
		void WriteValue(uint32_t value) override
		{
			WriteIndent();
			Convert::Detail::To(value, mOutputString);
		}
		void WriteValue(uint64_t value) override
		{
			WriteIndent();
			Convert::Detail::To(value, mOutputString);
		}

		void WriteValue(int8_t value) override
		{
			WriteIndent();
			Convert::Detail::To(value, mOutputString);
		}

		void WriteValue(int16_t value) override
		{
			WriteIndent();
			Convert::Detail::To(value, mOutputString);
		}

		void WriteValue(int32_t value) override
		{
			WriteIndent();
			Convert::Detail::To(value, mOutputString);
		}

		void WriteValue(int64_t value) override
		{
			WriteIndent();
			Convert::Detail::To(value, mOutputString);
		}

		void WriteValue(float value) override
		{
			WriteIndent();
			Convert::Detail::To(value, mOutputString);
		}

		void WriteValue(double value) override
		{
			WriteIndent();
			Convert::Detail::To(value, mOutputString);
		}

		void WriteValue(long double value) override
		{
			WriteIndent();
			Convert::Detail::To(value, mOutputString);
		}

		void WriteValue(const char* value) override { WriteValue(std::string_view(value)); }
		void WriteValue(std::string_view value) override;

		void WriteValue(JsonArchiveTraits::raw_type& value) override
		{
			WriteIndent();
			mOutputString.append(value.Get());
		}

		void WriteValueSeparator() override
		{
			mOutputString.push_back(',');
			mPadding = true;
		}

		void BeginArray() override
		{
			WriteIndent();
			mOutputString.push_back('[');
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
			mOutputString.push_back(']');
		}

		void BeginObject() override
		{
			WriteIndent();
			mOutputString.append("{");
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
			mOutputString.push_back('}');
		}

		void WriteKey(std::string_view key) override
		{
			WriteValue(key);
			mOutputString.append(": ");
			mPadding = false;
		}

	private:
		void WriteIndent() const
		{
			if (mPadding)
			{
				mOutputString.push_back('\n');
				if (mCurrentPadding > 0)
				{
					mOutputString.append(mCurrentPadding, mPaddingChar);
				}
			}
		}

		std::string& mOutputString;
		uint32_t mCurrentPadding = 0;
		uint16_t mPaddingCharNum;
		char mPaddingChar;
		bool mPadding = false;
	};
}
