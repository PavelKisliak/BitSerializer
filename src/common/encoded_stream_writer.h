/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstddef>
#include <ostream>
#include <string>
#include <type_traits>
#include "bitserializer/conversion_detail/convert_utf.h"

namespace BitSerializer::Convert::Utf
{
	/**
	 * @brief Writes UTF-encoded data to a stream with optional BOM.
	 *
	 * Output is accumulated in an internal buffer and flushed to the stream in chunks, which
	 * avoids the per-call overhead of `std::ostream::write` (sentry + TLS) for small writes.
	 * Call `Flush()` to write the remaining buffered data at the end of serialization.
	 */
	class EncodedStreamWriter
	{
	public:
		EncodedStreamWriter(std::ostream& outputStream, UtfType targetUtfType, bool addBom,
			UtfEncodingErrorPolicy encodingErrorPolicy = UtfEncodingErrorPolicy::Skip);

		template <typename TCharType>
		UtfEncodingErrorCode Write(const std::basic_string_view<TCharType>& str)
		{
			switch (mTargetUtfType)
			{
			case UtfType::Utf8:
				return WriteEncoded<Utf8>(str, mUtf8Buffer);
			case UtfType::Utf16le:
				return WriteEncoded<Utf16Le>(str, mUtf16Buffer);
			case UtfType::Utf16be:
				return WriteEncoded<Utf16Be>(str, mUtf16Buffer);
			case UtfType::Utf32le:
				return WriteEncoded<Utf32Le>(str, mUtf32Buffer);
			case UtfType::Utf32be:
				return WriteEncoded<Utf32Be>(str, mUtf32Buffer);
			}
			return UtfEncodingErrorCode::Success;
		}

		template <typename TCharType, typename TAllocator>
		UtfEncodingErrorCode Write(const std::basic_string<TCharType, std::char_traits<TCharType>, TAllocator>& str)
		{
			return Write(std::basic_string_view<TCharType>(str.data(), str.size()));
		}

		template <typename TCharType, size_t ArraySize>
		UtfEncodingErrorCode Write(const TCharType(&str)[ArraySize])
		{
			return Write(std::basic_string_view<TCharType>(str, ArraySize - 1));
		}

		/**
		 * @brief Flushes the buffered output to the stream.
		 */
		void Flush();

	private:
		static constexpr size_t OutputBufferSize = 1024;

		template <typename TEncoder, typename TBuffer, typename TCharType>
		UtfEncodingErrorCode WriteEncoded(const std::basic_string_view<TCharType>& str, TBuffer& buffer)
		{
			if constexpr (sizeof(TCharType) == 1 && std::is_same_v<TBuffer, std::string>)
			{
				// UTF-8 to UTF-8: copy 'as is' without analysis.
				mByteBuffer.append(str.data(), str.size());
				FlushIfFull();
				return UtfEncodingErrorCode::Success;
			}
			else
			{
				buffer.clear();
				// NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage)
				const auto result = TEncoder::Encode(str.data(), str.data() + str.size(), buffer, mEncodingErrorPolicy);
				if (result)
				{
					mByteBuffer.append(reinterpret_cast<const char*>(buffer.data()),
						buffer.size() * sizeof(typename TBuffer::value_type));
					FlushIfFull();
					return UtfEncodingErrorCode::Success;
				}
				return result.ErrorCode;
			}
		}

		void FlushIfFull()
		{
			if (mByteBuffer.size() >= OutputBufferSize) {
				Flush();
			}
		}

		std::ostream& mOutputStream;
		UtfType mTargetUtfType;
		UtfEncodingErrorPolicy mEncodingErrorPolicy;
		std::string mByteBuffer;
		std::string mUtf8Buffer;
		std::u16string mUtf16Buffer;
		std::u32string mUtf32Buffer;
	};
}
