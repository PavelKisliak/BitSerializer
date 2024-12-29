/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <vector>
#include "bitserializer/csv_archive.h"

namespace BitSerializer::Csv::Detail
{
	struct CValueMeta
	{
		CValueMeta(size_t offset, size_t size, bool hasEscapedChars) noexcept
			: Offset(offset), Size(size), HasEscapedChars(hasEscapedChars)
		{ }

		size_t Offset;
		size_t Size;
		bool HasEscapedChars;
	};

	class CCsvStringReader final : public ICsvReader
	{
	public:
		CCsvStringReader(std::string_view inputString, bool withHeader, char separator = ',');

		[[nodiscard]] size_t GetCurrentIndex() const noexcept override { return mRowIndex; }
		[[nodiscard]] bool IsEnd() const noexcept override { return mCurrentPos >= mSourceString.size(); }
		bool ReadValue(std::string_view key, std::string_view& out_value) override;
		void ReadValue(std::string_view& out_value) override;
		bool ParseNextRow() override;
		[[nodiscard]] const std::vector<std::string>& GetHeaders() const noexcept override { return mHeaders; }

	private:
		bool ParseNextLine(std::vector<CValueMeta>& out_values);
		std::string_view UnescapeValue(std::string_view value);

		std::string_view mSourceString;
		const bool mWithHeader;
		const char mSeparator;

		std::vector<std::string> mHeaders;
		std::vector<CValueMeta> mRowValuesMeta;
		std::string mTempValueBuffer;
		size_t mCurrentPos = 0;
		size_t mLineNumber = 0;
		size_t mRowIndex = 0;
		size_t mValueIndex = 0;
		size_t mPrevValuesCount = 0;
	};

	class CCsvStreamReader final : public ICsvReader
	{
	public:
		CCsvStreamReader(std::istream& inputStream, bool withHeader, char separator = ',');

		[[nodiscard]] size_t GetCurrentIndex() const noexcept override { return mRowIndex; }
		[[nodiscard]] bool IsEnd() const override { return mCurrentPos >= mDecodedBuffer.size() && mEncodedStreamReader.IsEnd(); }
		bool ReadValue(std::string_view key, std::string_view& out_value) override;
		void ReadValue(std::string_view& out_value) override;
		bool ParseNextRow() override;
		[[nodiscard]] const std::vector<std::string>& GetHeaders() const noexcept override { return mHeaders; }

	private:
		bool ParseNextLine(std::vector<CValueMeta>& out_values);
		std::string_view UnescapeValue(char* beginIt, char* endIt);

		Convert::Utf::CEncodedStreamReader<char> mEncodedStreamReader;
		std::string mDecodedBuffer;
		const bool mWithHeader;
		const char mSeparator;

		std::vector<std::string> mHeaders;
		std::vector<CValueMeta> mRowValuesMeta;
		size_t mCurrentPos = 0;
		size_t mLineNumber = 0;
		size_t mRowIndex = 0;
		size_t mValueIndex = 0;
		size_t mPrevValuesCount = 0;
	};
}
