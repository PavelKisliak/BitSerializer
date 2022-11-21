/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <vector>
#include "bitserializer/csv_archive.h"

namespace BitSerializer::Csv::Detail
{
	class CCsvStringReader final : public ICsvReader
	{
	public:
		CCsvStringReader(std::string_view inputString, bool withHeader, char separator = ',');

		[[nodiscard]] size_t GetCurrentIndex() const noexcept override { return mRowIndex; }
		[[nodiscard]] bool IsEnd() const override;
		bool ReadValue(std::string_view key, std::string& value) override;
		void ReadValue(std::string& value) override;
		bool ParseNextRow() override;

	private:
		bool ParseLine(std::vector<std::string>& out_values);

		std::string_view mSourceString;
		const bool mWithHeader;
		const char mSeparator;

		std::vector<std::string> mHeader;
		std::vector<std::string> mRowValues;
		size_t mCurrentPos = 0;
		size_t mLineNumber = 1;
		size_t mRowIndex = 0;
		size_t mValueIndex = 0;
		size_t mPrevValuesCount = 0;
	};

	class CCsvStreamReader final : public ICsvReader
	{
	public:
		CCsvStreamReader(std::istream& inputStream, bool withHeader, char separator = ',');

		[[nodiscard]] size_t GetCurrentIndex() const noexcept override { return mRowIndex; }
		[[nodiscard]] bool IsEnd() const override;
		bool ReadValue(std::string_view key, std::string& value) override;
		void ReadValue(std::string& value) override;
		bool ParseNextRow() override;

	private:
		bool ParseLine(std::vector<std::string>& out_values);
		void RemoveParsedStringPart();

		Convert::CEncodedStreamReader<Convert::Utf8> mEncodedStreamReader;
		std::string mDecodedBuffer;
		const bool mWithHeader;
		const char mSeparator;

		std::vector<std::string> mHeader;
		std::vector<std::string> mRowValues;
		size_t mCurrentPos = 0;
		size_t mLineNumber = 1;
		size_t mRowIndex = 0;
		size_t mValueIndex = 0;
		size_t mPrevValuesCount = 0;
	};
}
