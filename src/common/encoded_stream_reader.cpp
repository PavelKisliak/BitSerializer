/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "encoded_stream_reader.h"

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
			mRawBytesPos = bomSize;
			mConsumedRawBytes = bomSize;
		}

		mRawMode = std::is_same_v<TTargetCharType, char> && mDetectedEncoding == UtfType::Utf8;
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
			return mRawBytesPos;
		}
		return mConsumedRawBytes + mDecodedRawPos;
	}

	template <typename TTargetCharType>
	void EncodedStreamReader<TTargetCharType>::SetPosition(size_t pos)
	{
		// Re-read from stream if position was trimmed
		if (pos < mStreamOffset)
		{
			mInputStream.clear();
			mInputStream.seekg(static_cast<std::streamoff>(pos), std::ios::beg);
			if (mInputStream.fail())
			{
				mInputStream.clear();
				throw std::out_of_range("Cannot seek to position " + std::to_string(pos) + " in the stream");
			}

			mRawBytes.clear();
			mRawBytes.resize(mChunkSize);
			mInputStream.read(mRawBytes.data(), static_cast<std::streamsize>(mChunkSize));
			const auto n = mInputStream.gcount();
			mRawBytes.resize(static_cast<size_t>(n));

			mDecodedBuf.clear();
			mDecodedPos = 0;
			mDecodedRawPos = 0;
			mConsumedRawBytes = pos;
			mStreamOffset = pos;
			mRawBytesPos = 0;
			return;
		}

		// Try to fetch more data if position is beyond current buffer
		if (pos > mStreamOffset + mRawBytes.size())
		{
			mRawBytesPos = pos - mStreamOffset;
			EnsureRawDataAvailable();
			if (pos > mStreamOffset + mRawBytes.size()) {
				throw std::out_of_range("Position is out of range");
			}
		}

		if (mRawMode)
		{
			mRawBytesPos = pos - mStreamOffset;
			return;
		}

		// Decoded mode: position is within buffered data
		const size_t bufPos = pos - mStreamOffset;

		// Try to find position within already-decoded data
		if (pos >= mConsumedRawBytes && !mDecodedBuf.empty())
		{
			const size_t decodedBufStartRaw = mConsumedRawBytes - mStreamOffset;
			size_t currentRawPos = decodedBufStartRaw;
			for (size_t i = 0; i < mDecodedBuf.size(); ++i)
			{
				if (currentRawPos >= bufPos)
				{
					mDecodedPos = i;
					mDecodedRawPos = currentRawPos - decodedBufStartRaw;
					return;
				}
				currentRawPos += CountRawBytesPerChar(currentRawPos);
			}
		}

		// Fallback: clear decoded state and reset to raw position
		mRawBytesPos = bufPos;
		mDecodedBuf.clear();
		mDecodedPos = 0;
		mDecodedRawPos = 0;
		mConsumedRawBytes = pos;
	}

	template <typename TTargetCharType>
	std::optional<TTargetCharType> EncodedStreamReader<TTargetCharType>::PeekChar()
	{
		if (mRawMode)
		{
			if (mRawBytesPos >= mRawBytes.size())
			{
				EnsureRawDataAvailable();
				if (mRawBytesPos >= mRawBytes.size()) {
					return std::nullopt;
				}
			}
			return static_cast<TTargetCharType>(mRawBytes[mRawBytesPos]);
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
			if (mRawBytesPos >= mRawBytes.size())
			{
				EnsureRawDataAvailable();
				if (mRawBytesPos >= mRawBytes.size()) {
					return std::nullopt;
				}
			}
			return static_cast<TTargetCharType>(mRawBytes[mRawBytesPos++]);
		}

		auto result = PeekChar();
		if (result)
		{
			mDecodedRawPos += CountRawBytesPerChar((mConsumedRawBytes + mDecodedRawPos) - mStreamOffset);
			++mDecodedPos;
		}
		return result;
	}

	template <typename TTargetCharType>
	std::basic_string_view<TTargetCharType> EncodedStreamReader<TTargetCharType>::PeekChars(size_t minChars)
	{
		if (mRawMode)
		{
			if (mRawBytesPos + minChars > mRawBytes.size()) {
				EnsureRawDataAvailable();
			}
			const size_t avail = (mRawBytesPos < mRawBytes.size()) ? (mRawBytes.size() - mRawBytesPos) : 0;
			return std::basic_string_view<TTargetCharType>(reinterpret_cast<const TTargetCharType*>(mRawBytes.data() + mRawBytesPos), avail);
		}

		while (mDecodedBuf.size() - mDecodedPos < minChars)
		{
			const auto prevSize = mDecodedBuf.size();
			DecodeNextBatch();
			if (mDecodedBuf.size() == prevSize) {
				break;
			}
		}
		const size_t avail = mDecodedBuf.size() - mDecodedPos;
		return std::basic_string_view<TTargetCharType>(mDecodedBuf.data() + mDecodedPos, avail);
	}

	template <typename TTargetCharType>
	void EncodedStreamReader<TTargetCharType>::SkipChars(size_t count)
	{
		if (mRawMode)
		{
			mRawBytesPos += count;
			return;
		}
		mDecodedRawPos += CountRawBytesForChars(count, (mConsumedRawBytes + mDecodedRawPos) - mStreamOffset);
		mDecodedPos += count;
	}

	template <typename TTargetCharType>
	bool EncodedStreamReader<TTargetCharType>::IsEnd() const noexcept
	{
		if (mRawMode) {
			return mRawBytesPos >= mRawBytes.size() && mInputStream.eof();
		}
		return mDecodedPos >= mDecodedBuf.size() && mRawBytesPos >= mRawBytes.size() && mInputStream.eof();
	}

	template <typename TTargetCharType>
	size_t EncodedStreamReader<TTargetCharType>::CountRawBytesPerChar(size_t rawOffset) const noexcept
	{
		switch (mDetectedEncoding)
		{
		case UtfType::Utf8:
		{
			const auto b = static_cast<uint8_t>(mRawBytes[rawOffset]);
			if (b < 0xC0) {
				return 1;
			}
			if (b < 0xE0) {
				return 2;
			}
			if (b < 0xF0) {
				return 3;
			}
			return 4;
		}
		case UtfType::Utf16le:
		{
			if constexpr (std::is_same_v<TTargetCharType, char32_t>)
			{
				if (rawOffset + 4 <= mRawBytes.size())
				{
					const auto ch = static_cast<uint8_t>(mRawBytes[rawOffset])
						| (static_cast<uint8_t>(mRawBytes[rawOffset + 1]) << 8);
					if (ch >= UnicodeTraits::HighSurrogatesStart && ch <= UnicodeTraits::HighSurrogatesEnd)
					{
						return 4;
					}
				}
			}
			return 2;
		}
		case UtfType::Utf16be:
		{
			if constexpr (std::is_same_v<TTargetCharType, char32_t>)
			{
				if (rawOffset + 4 <= mRawBytes.size())
				{
					const auto ch = (static_cast<uint8_t>(mRawBytes[rawOffset]) << 8)
						| static_cast<uint8_t>(mRawBytes[rawOffset + 1]);
					if (ch >= UnicodeTraits::HighSurrogatesStart && ch <= UnicodeTraits::HighSurrogatesEnd)
					{
						return 4;
					}
				}
			}
			return 2;
		}
		case UtfType::Utf32le:
		case UtfType::Utf32be:
			return 4;
		}
		return 0;
	}

	template <typename TTargetCharType>
	size_t EncodedStreamReader<TTargetCharType>::CountRawBytesForChars(size_t numChars, size_t rawOffset) const noexcept
	{
		size_t pos = rawOffset;
		for (size_t i = 0; i < numChars && pos < mRawBytes.size(); ++i) {
			pos += CountRawBytesPerChar(pos);
		}
		return pos - rawOffset;
	}

	template <typename TTargetCharType>
	void EncodedStreamReader<TTargetCharType>::TrimConsumedData()
	{
		if (mDecodedBuf.empty() || mDecodedPos == 0) {
			return;
		}
		if (mDecodedPos < mChunkSize) {
			return;
		}

		const size_t trimCount = (std::min)(mDecodedPos, mDecodedBuf.size());
		mConsumedRawBytes += mDecodedRawPos;
		mDecodedBuf.erase(0, trimCount);
		mDecodedPos = 0;
		mDecodedRawPos = 0;

		const size_t shift = mConsumedRawBytes - mStreamOffset;
		if (shift >= mChunkSize)
		{
			mRawBytes.erase(mRawBytes.begin(), mRawBytes.begin() + shift);
			mRawBytesPos -= shift;
			mStreamOffset += shift;
		}
	}

	template <typename TTargetCharType>
	void EncodedStreamReader<TTargetCharType>::EnsureRawDataAvailable()
	{
		if (mInputStream.eof()) {
			return;
		}
		while (mRawBytesPos + mChunkSize > mRawBytes.size() && !mInputStream.eof())
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
		TrimConsumedData();
		EnsureRawDataAvailable();
		if (mRawBytesPos >= mRawBytes.size()) {
			return;
		}

		const char* data = mRawBytes.data() + mRawBytesPos;
		const size_t available = mRawBytes.size() - mRawBytesPos;

		const auto handleDecodeResult = [&](auto result) -> void
		{
			mRawBytesPos += static_cast<size_t>(reinterpret_cast<const char*>(result.Iterator) - data);
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
