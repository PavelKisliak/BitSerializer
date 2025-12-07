/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <vector>
#include "bitserializer/csv_archive.h"

namespace BitSerializer::Csv::Detail
{
	class CCsvStringReader final : public ICsvReader
	{
		struct CValueMeta
		{
			CValueMeta(size_t offset, size_t size, bool inOriginalData) noexcept
				: Offset(offset), Size(size), InOriginalData(inOriginalData)
			{
			}

			size_t Offset;
			size_t Size;
			// Indicates where the decoded value is located (in the original data or in a local buffer)
			bool InOriginalData;
		};

	public:
		CCsvStringReader(std::string_view inputString, bool withHeader, char separator = ',');

		[[nodiscard]] size_t GetCurrentLine() const noexcept override { return mLineNumber; }
		[[nodiscard]] size_t GetCurrentIndex() const noexcept override { return mRowIndex; }
		[[nodiscard]] bool IsEnd() const noexcept override { return mCurrentPos >= mSourceString.size(); }

		[[nodiscard]] size_t GetHeadersCount() const noexcept override { return mHeaders.size(); }
		[[nodiscard]] bool SeekToHeader(size_t headerIndex, std::string_view& out_header) noexcept override;

		bool ReadValue(std::string_view key, std::string_view& out_value) noexcept override;
		void ReadValue(std::string_view& out_value) override;
		bool ParseNextRow() override;

	private:
		bool ParseNextLine();
		void UnescapeValue(std::string_view value);

		std::string_view mSourceString;
		const bool mWithHeader;
		const char mSeparator;

		std::vector<std::string_view> mHeaders;
		std::vector<CValueMeta> mRowValuesMeta;
		std::vector<std::string::value_type> mTempValueBuffer;
		size_t mLockedBufferSize = 0;
		size_t mCurrentPos = 0;
		size_t mLineNumber = 0;
		size_t mRowIndex = 0;
		size_t mValueIndex = 0;
		size_t mPrevValuesCount = 0;
	};

	class CCsvStreamReader final : public ICsvReader
	{
		struct CValueMeta
		{
			CValueMeta(size_t offset, size_t size) noexcept
				: Offset(offset), Size(size)
			{
			}

			size_t Offset;
			size_t Size;
		};

	public:
		CCsvStreamReader(std::istream& inputStream, bool withHeader, char separator = ',');

		[[nodiscard]] size_t GetCurrentLine() const noexcept override { return mLineNumber; }
		[[nodiscard]] size_t GetCurrentIndex() const noexcept override { return mRowIndex; }
		[[nodiscard]] bool IsEnd() const override { return mCurrentPos >= mDecodedBuffer.size() && mEncodedStreamReader.IsEnd(); }

		[[nodiscard]] size_t GetHeadersCount() const noexcept override { return mHeaders.size(); }
		[[nodiscard]] bool SeekToHeader(size_t headerIndex, std::string_view& out_header) noexcept override;

		bool ReadValue(std::string_view key, std::string_view& out_value) noexcept override;
		void ReadValue(std::string_view& out_value) override;
		bool ParseNextRow() override;

	private:
		bool ParseNextLine();
		void UnescapeValue(char* beginIt, const char* endIt);

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
