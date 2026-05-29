/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>
#include "bitserializer/conversion_detail/convert_utf.h"

namespace BitSerializer::Convert::Utf
{
	/**
	 * @brief Reads UTF-encoded data from a stream with automatic encoding detection.
	 */
	template <typename TTargetCharType>
	class EncodedStreamReader
	{
	public:
		static constexpr size_t DefaultChunkSize = 1024;

		EncodedStreamReader(const EncodedStreamReader&) = delete;
		EncodedStreamReader(EncodedStreamReader&&) = delete;
		EncodedStreamReader& operator=(const EncodedStreamReader&) = delete;
		EncodedStreamReader& operator=(EncodedStreamReader&&) = delete;
		~EncodedStreamReader() = default;

		explicit EncodedStreamReader(std::istream& inputStream, UtfEncodingErrorPolicy encodeErrorPolicy = UtfEncodingErrorPolicy::Skip,
			const TTargetCharType* errorMark = Detail::GetDefaultErrorMark<TTargetCharType>(), size_t chunkSize = DefaultChunkSize);

		[[nodiscard]] std::basic_string_view<TTargetCharType> PeekData(size_t minChars);
		void SkipChars(size_t count);
		[[nodiscard]] bool IsEnd() const noexcept;
		[[nodiscard]] UtfType GetSourceUtfType() const noexcept;
		[[nodiscard]] size_t GetPosition() const noexcept;
		void SetPosition(size_t pos);
		[[nodiscard]] std::optional<TTargetCharType> PeekChar();
		[[nodiscard]] std::optional<TTargetCharType> ReadChar();

	private:
		[[nodiscard]] size_t CountRawBytes(size_t numChars, size_t rawOffset) const noexcept;
		void EnsureRawDataAvailable();
		void TrimDecodedBuf();
		void DecodeNextBatch();

		UtfType mDetectedEncoding = UtfType::Utf8;
		std::istream& mInputStream;
		UtfEncodingErrorPolicy mEncodingErrorPolicy;
		const TTargetCharType* mErrorMark;
		size_t mChunkSize;
		std::vector<char> mRawBytes;
		size_t mStreamPos = 0;
		bool mRawMode = true;
		std::basic_string<TTargetCharType> mDecodedBuf;
		size_t mDecodedPos = 0;
		size_t mConsumedRawBytes = 0;
	};

	extern template class EncodedStreamReader<char>;
	extern template class EncodedStreamReader<char16_t>;
	extern template class EncodedStreamReader<char32_t>;
	extern template class EncodedStreamReader<wchar_t>;
}
