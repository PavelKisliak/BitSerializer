/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/csv_archive.h"
#include "common/encoded_stream_writer.h"

namespace BitSerializer::Csv::Detail
{
	class CsvStringWriter final : public ICsvWriter
	{
	public:
		CsvStringWriter(std::string& outputString, bool withHeader, char separator = ',');

		void SetEstimatedSize(size_t size) noexcept override { mEstimatedSize = size; }

		void WriteValue(std::string_view key, std::nullptr_t) override;
		void WriteValue(std::string_view key, bool value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, uint8_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, uint16_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, uint32_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, uint64_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, int8_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, int16_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, int32_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, int64_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, float value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, double value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, long double value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, const char* value) override { WriteValue(key, std::string_view(value)); }
		void WriteValue(std::string_view key, std::string_view value) override;

		void NextLine() override;
		[[nodiscard]] size_t GetCurrentIndex() const noexcept override { return mRowIndex; }

	private:
		void BeginValue(std::string_view key);
		void EndValue() noexcept { ++mValueIndex; }

		template <typename T>
		void WriteNumericValue(std::string_view key, T value)
		{
			BeginValue(key);
			Convert::Detail::To(value, mCurrentRow);
			EndValue();
		}

		std::string& mOutputString;
		const bool mWithHeader;
		const char mSeparator;

		std::string mCurrentRow;
		size_t mRowIndex = 0;
		size_t mValueIndex = 0;
		size_t mEstimatedSize = 0;
		size_t mPrevValuesCount = 0;
	};

	class CsvStreamWriter final : public ICsvWriter
	{
	public:
		CsvStreamWriter(std::ostream& outputStream, bool withHeader, char separator = ',',
			Convert::Utf::UtfEncodingErrorPolicy utfEncodingErrorPolicy = Convert::Utf::UtfEncodingErrorPolicy::Skip, const StreamOptions& streamOptions = {});

		void SetEstimatedSize(size_t) noexcept override { /* Not required for stream */ }

		void WriteValue(std::string_view key, std::nullptr_t) override;
		void WriteValue(std::string_view key, bool value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, uint8_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, uint16_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, uint32_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, uint64_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, int8_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, int16_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, int32_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, int64_t value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, float value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, double value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, long double value) override { WriteNumericValue(key, value); }
		void WriteValue(std::string_view key, const char* value) override { WriteValue(key, std::string_view(value)); }
		void WriteValue(std::string_view key, std::string_view value) override;

		void NextLine() override;
		[[nodiscard]] size_t GetCurrentIndex() const noexcept override { return mRowIndex; }
		void Flush() override { mEncodedStream.Flush(); }

	private:
		void BeginValue(std::string_view key);
		void EndValue() noexcept { ++mValueIndex; }

		template <typename T>
		void WriteNumericValue(std::string_view key, T value)
		{
			BeginValue(key);
			Convert::Detail::To(value, mCurrentRow);
			EndValue();
		}

		Convert::Utf::EncodedStreamWriter mEncodedStream;
		const bool mWithHeader;
		const char mSeparator;

		std::string mCsvHeader;
		std::string mCurrentRow;
		size_t mRowIndex = 0;
		size_t mValueIndex = 0;
		size_t mPrevValuesCount = 0;
	};
}
