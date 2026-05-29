/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "encoded_stream_reader.h"
#include <algorithm>
#include <optional>

namespace BitSerializer::Convert::Utf
{
	template <typename TTargetCharType>
	EncodedStreamReader<TTargetCharType>::EncodedStreamReader(std::istream& inputStream, UtfEncodingErrorPolicy encodeErrorPolicy, const TTargetCharType* errorMark, size_t chunkSize)
		: mInputStream(inputStream)
		, mEncodingErrorPolicy(encodeErrorPolicy)
		, mErrorMark(errorMark)
		, mChunkSize(chunkSize)
	{
		if (mChunkSize % 4 != 0) {
			throw std::invalid_argument("Chunk size must be a multiple of 4");
		}
		if (mChunkSize < 32) {
			throw std::invalid_argument("Chunk size must be at least 32 bytes to correctly detect the encoding");
		}

		mRawBytes.resize(mChunkSize);
		mInputStream.read(mRawBytes.data(), static_cast<std::streamsize>(mChunkSize));
		const auto firstReadSize = mInputStream.gcount();
		mRawBytes.resize(static_cast<size_t>(firstReadSize));

		if (!mRawBytes.empty())
		{
			size_t bomSize = 0;
			mDetectedEncoding = DetectEncoding(std::string_view(mRawBytes.data(), mRawBytes.size()), bomSize);
			mStreamPos = bomSize;
			mConsumedRawBytes = bomSize;
		}

		mRawMode = std::is_same_v<TTargetCharType, char> && mDetectedEncoding == UtfType::Utf8;
	}

	template <typename TTargetCharType>
	std::basic_string_view<TTargetCharType> EncodedStreamReader<TTargetCharType>::PeekData(size_t minChars)
	{
		if (mRawMode)
		{
			EnsureRawDataAvailable();
			const size_t avail = (mStreamPos < mRawBytes.size()) ? (mRawBytes.size() - mStreamPos) : 0;
			return std::basic_string_view<TTargetCharType>(
				reinterpret_cast<const TTargetCharType*>(mRawBytes.data() + mStreamPos), avail);
		}
		while (mDecodedBuf.size() - mDecodedPos < minChars && !IsEnd())
		{
			DecodeNextBatch();
		}
		const size_t avail = mDecodedBuf.size() - mDecodedPos;
		return std::basic_string_view<TTargetCharType>(mDecodedBuf.data() + mDecodedPos, avail);
	}

	template <typename TTargetCharType>
	void EncodedStreamReader<TTargetCharType>::SkipChars(size_t count)
	{
		if (mRawMode)
		{
			mStreamPos += count;
			return;
		}
		mDecodedPos += count;
		TrimDecodedBuf();
	}

	template <typename TTargetCharType>
	bool EncodedStreamReader<TTargetCharType>::IsEnd() const noexcept
	{
		if (mRawMode) {
			return mStreamPos >= mRawBytes.size() && mInputStream.eof();
		}
		return mDecodedPos >= mDecodedBuf.size() && mStreamPos >= mRawBytes.size() && mInputStream.eof();
	}

	template <typename TTargetCharType>
	UtfType EncodedStreamReader<TTargetCharType>::GetSourceUtfType() const noexcept
	{
		return mDetectedEncoding;
	}

	template <typename TTargetCharType>
	size_t EncodedStreamReader<TTargetCharType>::GetPosition() const noexcept
	{
		if (mRawMode) {
			return mStreamPos;
		}
		return mConsumedRawBytes + CountRawBytes(mDecodedPos, mConsumedRawBytes);
	}

	template <typename TTargetCharType>
	void EncodedStreamReader<TTargetCharType>::SetPosition(size_t pos)
	{
		if (pos > mRawBytes.size()) {
			throw std::out_of_range("Position is out of range");
		}

		if (mRawMode)
		{
			mStreamPos = pos;
			return;
		}

		// Try to find position within already-decoded data
		if (pos >= mConsumedRawBytes && !mDecodedBuf.empty())
		{
			const size_t totalDecodedRaw = CountRawBytes(mDecodedBuf.size(), mConsumedRawBytes);
			if (pos < mConsumedRawBytes + totalDecodedRaw)
			{
				size_t cumRaw = mConsumedRawBytes;
				for (size_t i = 0; i < mDecodedBuf.size(); ++i)
				{
					if (cumRaw >= pos)
					{
						mDecodedPos = i;
						TrimDecodedBuf();
						return;
					}
					cumRaw += CountRawBytes(1, cumRaw);
				}
			}
		}

		// Fallback: clear and reset to raw position
		mStreamPos = pos;
		mDecodedBuf.clear();
		mDecodedPos = 0;
		mConsumedRawBytes = 0;
	}

	template <typename TTargetCharType>
	std::optional<TTargetCharType> EncodedStreamReader<TTargetCharType>::PeekChar()
	{
		if (mRawMode)
		{
			if (mStreamPos >= mRawBytes.size())
			{
				EnsureRawDataAvailable();
				if (mStreamPos >= mRawBytes.size()) {
					return std::nullopt;
				}
			}
			return static_cast<TTargetCharType>(mRawBytes[mStreamPos]);
		}
		if (mDecodedPos >= mDecodedBuf.size()) {
			DecodeNextBatch();
		}
		if (mDecodedPos >= mDecodedBuf.size()) {
			return std::nullopt;
		}
		return mDecodedBuf[mDecodedPos];
	}

	template <typename TTargetCharType>
	std::optional<TTargetCharType> EncodedStreamReader<TTargetCharType>::ReadChar()
	{
		if (mRawMode)
		{
			if (mStreamPos >= mRawBytes.size())
			{
				EnsureRawDataAvailable();
				if (mStreamPos >= mRawBytes.size()) {
					return std::nullopt;
				}
			}
			return static_cast<TTargetCharType>(mRawBytes[mStreamPos++]);
		}
		auto result = PeekChar();
		if (result) {
			++mDecodedPos;
		}
		return result;
	}

	template <typename TTargetCharType>
	size_t EncodedStreamReader<TTargetCharType>::CountRawBytes(size_t numChars, size_t rawOffset) const noexcept
	{
		size_t pos = rawOffset;
		for (size_t i = 0; i < numChars && pos < mRawBytes.size(); ++i)
		{
			switch (mDetectedEncoding)
			{
			case UtfType::Utf8:
			{
				const auto b = static_cast<uint8_t>(mRawBytes[pos]);
				if (b < 0xC0) {
					pos += 1;
				}
				else if (b < 0xE0) {
					pos += 2;
				}
				else if (b < 0xF0) {
					pos += 3;
				}
				else {
					pos += 4;
				}
				break;
			}
			case UtfType::Utf16le:
			{
				if constexpr (std::is_same_v<TTargetCharType, char32_t>)
				{
					if (pos + 4 <= mRawBytes.size())
					{
						const auto ch = static_cast<uint8_t>(mRawBytes[pos]) | (static_cast<uint8_t>(mRawBytes[pos + 1]) << 8);
						if (ch >= UnicodeTraits::HighSurrogatesStart && ch <= UnicodeTraits::HighSurrogatesEnd)
						{
							pos += 4;
							continue;
						}
					}
				}
				pos += 2;
				break;
			}
			case UtfType::Utf16be:
			{
				if constexpr (std::is_same_v<TTargetCharType, char32_t>)
				{
					if (pos + 4 <= mRawBytes.size())
					{
						const auto ch = (static_cast<uint8_t>(mRawBytes[pos]) << 8) | static_cast<uint8_t>(mRawBytes[pos + 1]);
						if (ch >= UnicodeTraits::HighSurrogatesStart && ch <= UnicodeTraits::HighSurrogatesEnd)
						{
							pos += 4;
							continue;
						}
					}
				}
				pos += 2;
				break;
			}
			case UtfType::Utf32le:
			case UtfType::Utf32be:
				pos += 4;
				break;
			}
		}
		return pos - rawOffset;
	}

	template <typename TTargetCharType>
	void EncodedStreamReader<TTargetCharType>::TrimDecodedBuf()
	{
		if (mDecodedBuf.empty() || mDecodedPos == 0) {
			return;
		}
		if (mDecodedPos < mChunkSize) {
			return;
		}

		const size_t trimCount = (std::min)(mDecodedPos, mDecodedBuf.size());
		mConsumedRawBytes += CountRawBytes(trimCount, mConsumedRawBytes);
		mDecodedBuf.erase(0, trimCount);
		mDecodedPos = 0;
	}

	template <typename TTargetCharType>
	void EncodedStreamReader<TTargetCharType>::EnsureRawDataAvailable()
	{
		while (mStreamPos + mChunkSize > mRawBytes.size() && !mInputStream.eof())
		{
			const auto oldSize = mRawBytes.size();
			mRawBytes.resize(oldSize + mChunkSize);
			mInputStream.read(mRawBytes.data() + oldSize, static_cast<std::streamsize>(mChunkSize));
			const auto n = mInputStream.gcount();
			mRawBytes.resize(oldSize + static_cast<size_t>(n));
		}
	}

	template <typename TTargetCharType>
	void EncodedStreamReader<TTargetCharType>::DecodeNextBatch()
	{
		if (IsEnd()) {
			return;
		}
		EnsureRawDataAvailable();
		if (mStreamPos >= mRawBytes.size()) {
			return;
		}

		const char* data = mRawBytes.data() + mStreamPos;
		const size_t available = mRawBytes.size() - mStreamPos;

		const auto handleDecodeResult = [&](auto result) -> void
		{
			mStreamPos += static_cast<size_t>(reinterpret_cast<const char*>(result.Iterator) - data);
			if (result.ErrorCode != UtfEncodingErrorCode::Success && mEncodingErrorPolicy == UtfEncodingErrorPolicy::ThrowError)
			{
				throw std::invalid_argument("Invalid UTF sequence detected");
			}
		};

		switch (mDetectedEncoding)
		{
		case UtfType::Utf8:
			if constexpr (!std::is_same_v<TTargetCharType, char>)
			{
				const auto result = Utf8::Decode(data, data + available, mDecodedBuf, mEncodingErrorPolicy, mErrorMark);
				handleDecodeResult(result);
			}
			break;
		case UtfType::Utf16le:
		{
			const auto* start = reinterpret_cast<const char16_t*>(data);
			const auto result = Utf16Le::Decode(start, start + available / sizeof(char16_t), mDecodedBuf, mEncodingErrorPolicy, mErrorMark);
			handleDecodeResult(result);
			break;
		}
		case UtfType::Utf16be:
		{
			const auto* start = reinterpret_cast<const char16_t*>(data);
			const auto result = Utf16Be::Decode(start, start + available / sizeof(char16_t), mDecodedBuf, mEncodingErrorPolicy, mErrorMark);
			handleDecodeResult(result);
			break;
		}
		case UtfType::Utf32le:
		{
			const auto* start = reinterpret_cast<const char32_t*>(data);
			const auto result = Utf32Le::Decode(start, start + available / sizeof(char32_t), mDecodedBuf, mEncodingErrorPolicy, mErrorMark);
			handleDecodeResult(result);
			break;
		}
		case UtfType::Utf32be:
		{
			const auto* start = reinterpret_cast<const char32_t*>(data);
			const auto result = Utf32Be::Decode(start, start + available / sizeof(char32_t), mDecodedBuf, mEncodingErrorPolicy, mErrorMark);
			handleDecodeResult(result);
			break;
		}
		}
	}

	template class EncodedStreamReader<char>;
	template class EncodedStreamReader<char16_t>;
	template class EncodedStreamReader<char32_t>;
	template class EncodedStreamReader<wchar_t>;
}
