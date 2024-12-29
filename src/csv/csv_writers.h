/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/csv_archive.h"

namespace BitSerializer::Csv::Detail
{
	class CCsvStringWriter final : public ICsvWriter
	{
	public:
		CCsvStringWriter(std::string& outputString, bool withHeader, char separator = ',');

		void SetEstimatedSize(size_t size) override;
		void WriteValue(const std::string_view& key, std::string_view value) override;
		void NextLine() override;
		[[nodiscard]] size_t GetCurrentIndex() const noexcept override { return mRowIndex; }

	private:
		std::string& mOutputString;
		const bool mWithHeader;
		const char mSeparator;

		std::string mCurrentRow;
		size_t mRowIndex = 0;
		size_t mValueIndex = 0;
		size_t mEstimatedSize = 0;
		size_t mPrevValuesCount = 0;
	};

	class CCsvStreamWriter final : public ICsvWriter
	{
	public:
		CCsvStreamWriter(std::ostream& outputStream, bool withHeader, char separator = ',', const StreamOptions& streamOptions = {});

		void SetEstimatedSize(size_t size) noexcept override { /* Not required for stream */ }
		void WriteValue(const std::string_view& key, std::string_view value) override;
		void NextLine() override;
		[[nodiscard]] size_t GetCurrentIndex() const noexcept override { return mRowIndex; }

	private:
		Convert::Utf::CEncodedStreamWriter mEncodedStream;
		const bool mWithHeader;
		const char mSeparator;

		std::string mCsvHeader;
		std::string mCurrentRow;
		size_t mRowIndex = 0;
		size_t mValueIndex = 0;
		size_t mPrevValuesCount = 0;
	};
}
