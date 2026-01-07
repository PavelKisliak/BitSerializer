/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdint>
#include <cassert>
#include <istream>
#include <iterator>
#include <string>
#include <cstring>
#include <variant>
#include "bitserializer/common/memory.h"
#include "bitserializer/conversion_detail/convert_enum.h"

namespace BitSerializer::Convert::Utf
{
	/**
	 * @brief UTF encoding types supported by the library.
	 */
	enum class UtfType
	{
		Utf8,
		Utf16le,
		Utf16be,
		Utf32le,
		Utf32be
	};

	BITSERIALIZER_REGISTER_ENUM(UtfType, {
		{ UtfType::Utf8, "UTF-8" },
		{ UtfType::Utf16le, "UTF-16LE" },
		{ UtfType::Utf16be, "UTF-16BE" },
		{ UtfType::Utf32le, "UTF-32LE" },
		{ UtfType::Utf32be, "UTF-32BE" }
	})
	BITSERIALIZER_DECLARE_ENUM_STREAM_OPS(BitSerializer::Convert::Utf::UtfType)

	/**
	 * @brief Error handling policy for UTF encoding operations.
	 */
	enum class UtfEncodingErrorPolicy
	{
		Skip,		///< Skip invalid UTF sequences (replace with an error mark).
		ThrowError	///< Throw a `std::invalid_argument` on invalid sequence.
	};

	/**
	 * @brief Error codes returned during UTF encoding or decoding.
	 */
	enum class UtfEncodingErrorCode
	{
		Success = 0,
		InvalidSequence,
		UnexpectedEnd
	};

	/**
	 * @brief Result type returned by UTF encoding/decoding functions.
	 *
	 * Contains:
	 * - An error code.
	 * - The iterator pointing to the next unprocessed character.
	 * - A count of invalid sequences handled.
	 */
	template <typename TIterator>
	class UtfEncodingResult
	{
	public:
		UtfEncodingResult(UtfEncodingErrorCode errorCode, TIterator it, size_t invalidSequencesCount) noexcept
			: ErrorCode(errorCode), Iterator(std::move(it)), InvalidSequencesCount(invalidSequencesCount)
		{
		}

		template <typename TOtherIt, std::enable_if_t<std::is_convertible_v<TOtherIt, TIterator>, int> = 0>
		UtfEncodingResult(const UtfEncodingResult<TOtherIt>& otherResult) noexcept(std::is_nothrow_constructible_v<TIterator, TOtherIt>)
			: ErrorCode(otherResult.ErrorCode), Iterator(otherResult.Iterator), InvalidSequencesCount(otherResult.InvalidSequencesCount)
		{
		}

		[[nodiscard]] operator bool() const noexcept {
			return ErrorCode == UtfEncodingErrorCode::Success;
		}

		UtfEncodingErrorCode ErrorCode;	///< Result status.
		TIterator Iterator;				///< Points to the first unprocessed character.
		size_t InvalidSequencesCount;	///< Count of replaced or skipped invalid sequences.
	};

	namespace UnicodeTraits
	{
		static constexpr uint16_t HighSurrogatesStart = 0xD800;
		static constexpr uint16_t HighSurrogatesEnd = 0xDBFF;
		static constexpr uint16_t LowSurrogatesStart = 0xDC00;
		static constexpr uint16_t LowSurrogatesEnd = 0xDFFF;

		constexpr bool IsInSurrogatesRange(const char32_t sym) noexcept
		{
			return sym >= HighSurrogatesStart && sym <= LowSurrogatesEnd;
		}
	}

	namespace Detail
	{
		/**
		 * @brief Handles invalid UTF sequences based on the selected error policy.
		 *
		 * @param outStr Output string where replacement may be written.
		 * @param encodingPolicy Error handling policy.
		 * @param errorMark Optional custom replacement character.
		 * @return true if operation should continue, false if it should fail.
		 */
		template<typename TChar, typename TAllocator>
		[[nodiscard]] bool HandleEncodingError(std::basic_string<TChar, std::char_traits<TChar>, TAllocator>& outStr, UtfEncodingErrorPolicy encodingPolicy, const TChar* errorMark)
		{
			switch (encodingPolicy)
			{
			case UtfEncodingErrorPolicy::ThrowError:
				return false;
			case UtfEncodingErrorPolicy::Skip:
				if (errorMark) {
					outStr.append(errorMark);
				}
				break;
			}
			return true;
		}

		/**
		 * @brief Returns a default Unicode replacement character for the given character type.
		 */
		template <typename TChar>
		constexpr const TChar* GetDefaultErrorMark() noexcept
		{
			static_assert(sizeof(TChar) <= sizeof(char32_t), "Unsupported character type");

			if constexpr (sizeof(TChar) == sizeof(char)) {
				return reinterpret_cast<const TChar*>(u8"☐");
			}
			if constexpr (sizeof(TChar) == sizeof(char16_t)) {
				return reinterpret_cast<const TChar*>(u"☐");
			}
			if constexpr (sizeof(TChar) == sizeof(char32_t))
			{
				return reinterpret_cast<const TChar*>(U"☐");
			}
		}
	} // namespace Detail

	//-----------------------------------------------------------------------------
	// UTF-8 Implementation
	//-----------------------------------------------------------------------------

	class Utf8
	{
	public:
		using char_type = char;
		static constexpr UtfType utfType = UtfType::Utf8;
		static constexpr char bom[] = { '\xEF', '\xBB', '\xBF' };
		static constexpr Memory::Endian endianness = Memory::Endian::native;

		/**
		 * @brief Decodes UTF-8 into UTF-16 or UTF-32.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Decode(TInIt in, TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(decltype(*in)) == sizeof(char_type), "The input sequence must be 8-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char16_t) || sizeof(TOutChar) == sizeof(char32_t), "Output string should have at least 16-bit characters");

			size_t invalidSequencesCount = 0;
			while (in != end)
			{
				TInIt startTailPos = in;
				uint32_t sym = static_cast<unsigned char>(*in);		// NOLINT(clang-analyzer-core.uninitialized.Assign)
				++in;
				if ((sym & 0b10000000) == 0)
				{
					outStr.push_back(static_cast<TOutChar>(sym));
					continue;
				}

				int tails = 0;
				bool isWrongSeq = false;
				if ((sym & 0b11100000) == 0b11000000) { tails = 2; sym &= 0b00011111; }
				else if ((sym & 0b11110000) == 0b11100000) { tails = 3; sym &= 0b00001111; }
				else if ((sym & 0b11111000) == 0b11110000) { tails = 4; sym &= 0b00000111; }
				// Overlong sequence (was prohibited in the RFC 3629 since November 2003)
				else if ((sym & 0b11111100) == 0b11111000) { isWrongSeq = true; tails = 5; }
				else if ((sym & 0b11111110) == 0b11111100) { isWrongSeq = true; tails = 6; }
				// Invalid start code
				else {
					isWrongSeq = true;
				}

				// Decode following tails
				for (; tails > 1; --tails)
				{
					if (in == end) {
						return UtfEncodingResult(UtfEncodingErrorCode::UnexpectedEnd, startTailPos, invalidSequencesCount);
					}

					if (!isWrongSeq)
					{
						const auto nextTail = static_cast<uint8_t>(*in);	// NOLINT(clang-analyzer-core.uninitialized.Assign)
						if ((nextTail & 0b11000000) == 0b10000000)
						{
							sym <<= 6;
							sym |= nextTail & 0b00111111;
						}
						// When tail has bad signature
						else {
							isWrongSeq = true;
						}
					}
					++in;
				}

				// Error handling when wrong sequence or when surrogate pair (prohibited in the UTF-8)
				if (isWrongSeq || UnicodeTraits::IsInSurrogatesRange(sym))
				{
					++invalidSequencesCount;
					if (!Detail::HandleEncodingError(outStr, errorPolicy, errorMark)) {
						return UtfEncodingResult(UtfEncodingErrorCode::InvalidSequence, startTailPos, invalidSequencesCount);
					}
				}
				else
				{
					// Decode as surrogate pair when character exceeds UTF-16 range
					if (sym > 0xFFFF && sizeof(TOutChar) == 2)
					{
						sym -= 0x10000;
						outStr.push_back(static_cast<TOutChar>(UnicodeTraits::HighSurrogatesStart | ((sym >> 10) & 0x3FF)));
						outStr.push_back(static_cast<TOutChar>(UnicodeTraits::LowSurrogatesStart | (sym & 0x3FF)));
					}
					else {
						outStr.push_back(static_cast<TOutChar>(sym));
					}
				}
			}
			return UtfEncodingResult(UtfEncodingErrorCode::Success, in, invalidSequencesCount);
		}

		/**
		 * @brief Encodes UTF-16 or UTF-32 into UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Encode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(decltype(*in)) > sizeof(char_type), "The input sequence must be at least 16-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char), "Output string must be 8-bit characters (e.g. std::string)");

			using InCharType = decltype(*in);
			size_t invalidSequencesCount = 0;
			while (in != end)
			{
				TInIt startTailPos = in;
				uint32_t sym = *in;
				++in;
				if (sym < 0x80)
				{
					outStr.push_back(static_cast<char>(sym));
					continue;
				}

				// Handle surrogates for UTF-16 (decode before encoding to UTF-8)
				if constexpr (sizeof(InCharType) == sizeof(char16_t))
				{
					if (UnicodeTraits::IsInSurrogatesRange(sym))
					{
						// Low surrogate character cannot be first and should present second part
						if (sym >= UnicodeTraits::LowSurrogatesStart)
						{
							++invalidSequencesCount;
							if (!Detail::HandleEncodingError(outStr, errorPolicy, errorMark)) {
								return UtfEncodingResult(UtfEncodingErrorCode::InvalidSequence, startTailPos, invalidSequencesCount);
							}
							continue;
						}
						// Check if it's end of input string
						if (in == end)
						{
							// Should return iterator to first character in the surrogate pair
							return UtfEncodingResult(UtfEncodingErrorCode::UnexpectedEnd, startTailPos, invalidSequencesCount);
						}
						// Surrogate characters are always written as pairs (low follows after high)
						const char16_t low = *in;
						if (low >= UnicodeTraits::LowSurrogatesStart && sym <= UnicodeTraits::LowSurrogatesEnd)
						{
							sym = 0x10000 + ((sym & 0x3FF) << 10 | (low & 0x3FF));
							++in;
						}
						else
						{
							++invalidSequencesCount;
							if (!Detail::HandleEncodingError(outStr, errorPolicy, errorMark)) {
								return UtfEncodingResult(UtfEncodingErrorCode::InvalidSequence, startTailPos, invalidSequencesCount);
							}
							continue;
						}
					}
				}

				if (sym < 0x800)
				{
					outStr.append({
						static_cast<TOutChar>(0b11000000 | (sym >> 6)),
						static_cast<TOutChar>(0b10000000 | (sym & 0b00111111))
						});
				}
				else if (sym < 0x10000)
				{
					outStr.append({
						static_cast<TOutChar>(0b11100000 | (sym >> 12)),
						static_cast<TOutChar>(0b10000000 | ((sym >> 6) & 0b00111111)),
						static_cast<TOutChar>(0b10000000 | ((sym & 0b00111111)))
						});
				}
				else
				{
					outStr.append({
						static_cast<TOutChar>(0b11110000 | (sym >> 18)),
						static_cast<TOutChar>(0b10000000 | ((sym >> 12) & 0b00111111)),
						static_cast<TOutChar>(0b10000000 | ((sym >> 6) & 0b00111111)),
						static_cast<TOutChar>(0b10000000 | ((sym & 0b00111111)))
						});
				}
			}
			return UtfEncodingResult(UtfEncodingErrorCode::Success, in, invalidSequencesCount);
		}
	};

	//-----------------------------------------------------------------------------
	// UTF-16 / UTF-16LE / UTF-16BE Implementations
	//-----------------------------------------------------------------------------

	class Utf16
	{
	public:
		using char_type = char16_t;
		static constexpr UtfType utfType = Memory::Endian::native == Memory::Endian::little ? UtfType::Utf16le : UtfType::Utf16be;
		static constexpr char bom[] = { '\xFF', '\xFE' };
		static constexpr Memory::Endian endianness = Memory::Endian::native;

		/**
		 * @brief Decodes UTF-16 to UTF-32, UTF-16 (copies 'as is') or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Decode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(decltype(*in)) == sizeof(char_type), "The input sequence must be 16-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char) || sizeof(TOutChar) == sizeof(char16_t) || sizeof(TOutChar) == sizeof(char32_t), "Output string should have 8, 16 or 32-bit characters");

			if constexpr (sizeof(TOutChar) == sizeof(char))
			{
				return Utf8::Encode(in, end, outStr, errorPolicy, errorMark);
			}
			else
			{
				size_t invalidSequencesCount = 0;
				while (in != end)
				{
					TInIt startTailPos = in;
					TOutChar sym = *in;
					++in;

					if constexpr (sizeof(TOutChar) == sizeof(char16_t))
					{
						// Do not copy only first part of surrogate pair
						if (in == end && (sym >= UnicodeTraits::HighSurrogatesStart && sym < UnicodeTraits::HighSurrogatesEnd)) {
							return UtfEncodingResult(UtfEncodingErrorCode::UnexpectedEnd, startTailPos, 0);
						}
					}
					else if constexpr (sizeof(TOutChar) == sizeof(char32_t))
					{
						// Handle surrogates
						if (UnicodeTraits::IsInSurrogatesRange(sym))
						{
							// Low surrogate character cannot be first and should present second part
							if (sym >= UnicodeTraits::LowSurrogatesStart)
							{
								++invalidSequencesCount;
								if (!Detail::HandleEncodingError(outStr, errorPolicy, errorMark)) {
									return UtfEncodingResult(UtfEncodingErrorCode::InvalidSequence, startTailPos, invalidSequencesCount);
								}
								continue;
							}
							// Check if it's end of input string
							if (in == end)
							{
								// Should return iterator to first character in surrogate pair
								return UtfEncodingResult(UtfEncodingErrorCode::UnexpectedEnd, startTailPos, 0);
							}
							// Surrogate characters are always written as pairs (low follows after high)
							const char16_t low = *in;
							if (low >= UnicodeTraits::LowSurrogatesStart && sym <= UnicodeTraits::LowSurrogatesEnd)
							{
								sym = 0x10000 + ((sym & 0x3FF) << 10 | (low & 0x3FF));
								++in;
							}
							else
							{
								++invalidSequencesCount;
								if (!Detail::HandleEncodingError(outStr, errorPolicy, errorMark)) {
									return UtfEncodingResult(UtfEncodingErrorCode::InvalidSequence, startTailPos, invalidSequencesCount);
								}
								continue;
							}
						}
					}

					outStr.push_back(sym);
				}
				return UtfEncodingResult(UtfEncodingErrorCode::Success, in, invalidSequencesCount);
			}
		}

		/**
		 * @brief Encodes to UTF-16 from UTF-32, UTF-16 (copies 'as is') or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Encode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(TOutChar) == sizeof(char16_t), "Output string must be 16-bit characters (e.g. std::u16string)");

			using TInCharType = decltype(*in);
			if constexpr (sizeof(TInCharType) == sizeof(char))
			{
				return Utf8::Decode(in, end, outStr, errorPolicy, errorMark);
			}
			else if constexpr (sizeof(TInCharType) == sizeof(char16_t))
			{
				while (in != end)
				{
					TInIt startTailPos = in;
					TOutChar sym = *in;
					++in;
					// Do not copy only first part of surrogate pair
					if (in == end && (sym >= UnicodeTraits::HighSurrogatesStart && sym < UnicodeTraits::HighSurrogatesEnd)) {
						return UtfEncodingResult(UtfEncodingErrorCode::UnexpectedEnd, startTailPos, 0);
					}
					outStr.push_back(sym);
				}
				return UtfEncodingResult(UtfEncodingErrorCode::Success, in, 0);
			}
			else if constexpr (sizeof(TInCharType) == sizeof(char32_t))
			{
				while (in != end)
				{
					uint32_t sym = *in;
					++in;
					if (sym < 0x10000)
					{
						outStr.push_back(static_cast<TOutChar>(sym));
					}
					else
					{
						// Encode as surrogate pair
						sym -= 0x10000;
						outStr.push_back(static_cast<TOutChar>(UnicodeTraits::HighSurrogatesStart | (sym >> 10)));
						outStr.push_back(static_cast<TOutChar>(UnicodeTraits::LowSurrogatesStart | (sym & 0x3FF)));
					}
				}
				return UtfEncodingResult(UtfEncodingErrorCode::Success, in, 0);
			}
			else
			{
				// Just for suppress warnings, should never be reached
				return UtfEncodingResult(UtfEncodingErrorCode::Success, in, 0);
			}
		}
	};


	class Utf16Le
	{
	public:
		using char_type = char16_t;
		static constexpr UtfType utfType = UtfType::Utf16le;
		static constexpr char bom[] = { '\xFF', '\xFE' };
		static constexpr Memory::Endian endianness = Memory::Endian::little;

		/**
		 * @brief Decodes UTF-16LE to UTF-32, UTF-16 or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Decode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			return Utf16::Decode(Memory::MakeIteratorAdapter<endianness>(in), Memory::MakeIteratorAdapter<endianness>(end), outStr, errorPolicy, errorMark);
		}

		/**
		 * @brief Encodes to UTF-16LE from UTF-32, UTF-16 or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Encode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(TOutChar) == sizeof(char16_t), "Output string must be 16-bit characters (e.g. std::u16string)");

			const size_t startOutPos = outStr.size();
			auto result = Utf16::Encode(in, end, outStr, errorPolicy, errorMark);
			if constexpr (endianness != Utf16::endianness) {
				Memory::Reverse(outStr.begin() + startOutPos, outStr.end());
			}
			return result;
		}
	};


	class Utf16Be
	{
	public:
		using char_type = char16_t;
		static constexpr UtfType utfType = UtfType::Utf16be;
		static constexpr char bom[] = { '\xFE', '\xFF' };
		static constexpr Memory::Endian endianness = Memory::Endian::big;

		/**
		 * @brief Decodes UTF-16BE to UTF-32, UTF-16 or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Decode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			return Utf16::Decode(Memory::MakeIteratorAdapter<endianness>(in), Memory::MakeIteratorAdapter<endianness>(end), outStr, errorPolicy, errorMark);
		}

		/**
		 * @brief Encodes UTF-16BE from UTF-32, UTF-16 or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Encode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(TOutChar) == sizeof(char16_t), "Output string must be 16-bit characters (e.g. std::u16string)");

			const size_t startOutPos = outStr.size();
			auto result = Utf16::Encode(in, end, outStr, errorPolicy, errorMark);
			if constexpr (endianness != Utf16::endianness) {
				Memory::Reverse(outStr.begin() + startOutPos, outStr.end());
			}
			return result;
		}
	};

	//-----------------------------------------------------------------------------
	// UTF-32 / UTF-32LE / UTF-32BE Implementations
	//-----------------------------------------------------------------------------

	class Utf32
	{
	public:
		using char_type = char32_t;
		static constexpr UtfType utfType = UtfType::Utf32le;
		static constexpr char bom[] = { '\xFF', '\xFE', '\x00', '\x00' };
		static constexpr Memory::Endian endianness = Memory::Endian::native;

		/**
		 * @brief Decodes UTF-32 to UTF-32 (copies 'as is'), UTF-16 or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Decode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(decltype(*in)) == sizeof(char_type), "The input sequence must be 32-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char) || sizeof(TOutChar) == sizeof(char16_t) || sizeof(TOutChar) == sizeof(char32_t), "Output string should have 8, 16 or 32-bit characters");

			if constexpr (sizeof(TOutChar) == sizeof(char_type))
			{
				for (; in != end; ++in) {
					outStr.push_back(static_cast<TOutChar>(*in));
				}
				return UtfEncodingResult(UtfEncodingErrorCode::Success, in, 0);
			}
			else if constexpr (sizeof(TOutChar) == sizeof(Utf16Le::char_type))
			{
				return Utf16::Encode(in, end, outStr, errorPolicy, errorMark);
			}
			else if constexpr (sizeof(TOutChar) == sizeof(Utf8::char_type))
			{
				return Utf8::Encode(in, end, outStr, errorPolicy, errorMark);
			}
			else
			{
				// Just for suppress warnings, should never be reached
				return UtfEncodingResult(UtfEncodingErrorCode::Success, in, 0);
			}
		}

		/**
		 * @brief Encodes UTF-32 from UTF-32 (copies 'as is'), UTF-16 or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Encode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(TOutChar) == sizeof(char32_t), "Output string must be 32-bit characters (e.g. std::u32string)");

			using TInCharType = typename std::iterator_traits<TInIt>::value_type;
			if constexpr (sizeof(TInCharType) == sizeof(char_type))
			{
				for (; in != end; ++in) {
					outStr.push_back(static_cast<TOutChar>(*in));
				}
				return UtfEncodingResult(UtfEncodingErrorCode::Success, in, 0);
			}
			else if constexpr (sizeof(TInCharType) == sizeof(Utf16Le::char_type))
			{
				return Utf16::Decode(in, end, outStr, errorPolicy, errorMark);
			}
			else if constexpr (sizeof(TInCharType) == sizeof(Utf8::char_type))
			{
				return Utf8::Decode(in, end, outStr, errorPolicy, errorMark);
			}
			else
			{
				// Just for suppress warnings, should never be reached
				return UtfEncodingResult(UtfEncodingErrorCode::Success, in, 0);
			}
		}
	};


	class Utf32Le
	{
	public:
		using char_type = char32_t;
		static constexpr UtfType utfType = UtfType::Utf32le;
		static constexpr char bom[] = { '\xFF', '\xFE', '\x00', '\x00' };
		static constexpr Memory::Endian endianness = Memory::Endian::little;

		/**
		 * @brief Decodes UTF-32LE to UTF-32, UTF-16 or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Decode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			return Utf32::Decode(Memory::MakeIteratorAdapter<endianness>(in), Memory::MakeIteratorAdapter<endianness>(end), outStr, errorPolicy, errorMark);
		}

		/**
		 * @brief Encodes UTF-32LE from UTF-32, UTF-16 or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Encode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			const size_t startOutPos = outStr.size();
			auto result = Utf32::Encode(in, end, outStr, errorPolicy, errorMark);
			if constexpr (endianness != Utf32::endianness) {
				Memory::Reverse(outStr.begin() + startOutPos, outStr.end());
			}
			return result;
		}
	};


	class Utf32Be
	{
	public:
		using char_type = char32_t;
		static constexpr UtfType utfType = UtfType::Utf32be;
		static constexpr char bom[] = { '\x00', '\x00', '\xFE', '\xFF' };
		static constexpr Memory::Endian endianness = Memory::Endian::big;

		/**
		 * @brief Decodes UTF-32BE to UTF-32, UTF-16 or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Decode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			return Utf32::Decode(Memory::MakeIteratorAdapter<endianness>(in), Memory::MakeIteratorAdapter<endianness>(end), outStr, errorPolicy, errorMark);
		}

		/**
		 * @brief Encodes UTF-32BE from UTF-32, UTF-16 or UTF-8.
		 */
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static UtfEncodingResult<TInIt> Encode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			const size_t startOutPos = outStr.size();
			auto result = Utf32::Encode(in, end, outStr, errorPolicy, errorMark);
			if constexpr (endianness != Utf32::endianness) {
				Memory::Reverse(outStr.begin() + startOutPos, outStr.end());
			}
			return result;
		}
	};

	//-----------------------------------------------------------------------------
	// Utility Functions
	//-----------------------------------------------------------------------------

	/**
	 * @brief Transcodes input UTF string to another UTF format (in byte order of the current platform).
	 */
	template<typename TInIt, typename TOutChar, typename TAllocator>
	UtfEncodingResult<TInIt> Transcode(TInIt in, const TInIt& end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
		UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
	{
		using TInCharType = typename std::iterator_traits<TInIt>::value_type;
		if constexpr (sizeof(TInCharType) == sizeof(TOutChar))
		{
			outStr.append(in, end);
			return UtfEncodingResult(UtfEncodingErrorCode::Success, end, 0);
		}
		else if constexpr (sizeof(TOutChar) == 1) {
			return Utf8::Encode(in, end, outStr, errorPolicy, errorMark);
		}
		else if constexpr (sizeof(TOutChar) == 2) {
			return Utf16::Encode(in, end, outStr, errorPolicy, errorMark);
		}
		else {
			return Utf32::Encode(in, end, outStr, errorPolicy, errorMark);
		}
	}

	/**
	 * @brief Transcodes a UTF string view to another UTF encoding.
	 *
	 * This function transcodes the entire content of a `std::basic_string_view` from one UTF encoding to another.
	 * It is a convenience wrapper around the iterator-based version of `Transcode`.
	 *
	 * @tparam TInChar Character type of the input string view (UTF-8, UTF-16, or UTF-32).
	 * @tparam TOutChar Character type of the output string (UTF-8, UTF-16, or UTF-32).
	 * @tparam TAllocator Allocator type used by the output string.
	 * @param[in] sourceStr Input string view to transcode.
	 * @param[out] outStr Output string where transcoded data will be appended.
	 * @param[in] errorPolicy Policy for handling invalid UTF sequences.
	 * @param[in] errorMark Optional replacement character for invalid sequences.
	 * @return A result object containing:
	 *   - Encoding status (`UtfEncodingErrorCode`)
	 *   - Iterator pointing to the first unprocessed character in the input
	 *   - Count of invalid sequences handled
	 */
	template<typename TInChar, typename TOutChar, typename TAllocator>
	UtfEncodingResult<typename std::basic_string_view<TInChar>::iterator> Transcode(const std::basic_string_view<TInChar> sourceStr, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
		UtfEncodingErrorPolicy errorPolicy = UtfEncodingErrorPolicy::Skip, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
	{
		return Transcode(sourceStr.cbegin(), sourceStr.cend(), outStr, errorPolicy, errorMark);
	}

	/**
	 * @brief Checks whether the given string starts with the Byte Order Mark (BOM) defined by the specified UTF traits class.
	 *
	 * This function compares the beginning of the input sequence with the BOM bytes defined in `TUtfTraits::bom`.
	 * It supports any container or string-like type that provides `std::cbegin()` and `std::cend()`.
	 *
	 * @tparam TUtfTraits Traits class defining the expected BOM sequence (e.g., `Utf8`, `Utf16Le`, etc.)
	 * @tparam T Container or string-like type supporting `std::cbegin()` and `std::cend()`.
	 * @param[in] inputString The input string or container to check.
	 * @return true if the input starts with the BOM; false otherwise.
	 */
	template<class TUtfTraits, class T>
	static bool StartsWithBom(const T& inputString)
	{
		auto it = std::cbegin(inputString);
		const auto endIt = std::cend(inputString);
		for (const char ch : TUtfTraits::bom)
		{
			if (it == endIt || *it != ch) {
				return false;
			}
			++it;
		}
		return true;
	}

	/**
	 * @brief Detects the UTF encoding of a byte stream.
	 */
	static UtfType DetectEncoding(std::string_view inputString, size_t& out_dataOffset)
	{
		if (inputString.empty())
		{
			out_dataOffset = 0;
			return UtfType::Utf8;
		}

		out_dataOffset = 0;
		UtfType utfType = UtfType::Utf8;
		if (StartsWithBom<Utf8>(inputString))
		{
			out_dataOffset = sizeof Utf8::bom;
			utfType = UtfType::Utf8;
		}
		else if (StartsWithBom<Utf32Le>(inputString))
		{
			out_dataOffset = sizeof Utf32Le::bom;
			utfType = UtfType::Utf32le;
		}
		else if (StartsWithBom<Utf32Be>(inputString))
		{
			out_dataOffset = sizeof Utf32Be::bom;
			utfType = UtfType::Utf32be;
		}
		else if (StartsWithBom<Utf16Le>(inputString))
		{
			out_dataOffset = sizeof Utf16Le::bom;
			utfType = UtfType::Utf16le;
		}
		else if (StartsWithBom<Utf16Be>(inputString))
		{
			out_dataOffset = sizeof Utf16Be::bom;
			utfType = UtfType::Utf16be;
		}
		else
		{
			// Analysis data
			for (size_t i = 0; i < inputString.size(); ++i)
			{
				// Detecting UTF-32 (LE/BE)
				if (i % sizeof(Utf32Le::char_type) == 0 && i + sizeof(Utf32Le::char_type) < inputString.size())
				{
					if (const uint32_t sym = Memory::NativeToLittleEndian(*reinterpret_cast<const uint32_t*>(&inputString[i])); sym != 0)
					{
						if ((sym & 0b11111111111111110000000000000000) == 0)
						{
							utfType = UtfType::Utf32le;
							break;
						}
						if ((sym & 0b00000000000000001111111111111111) == 0)
						{
							utfType = UtfType::Utf32be;
							break;
						}
					}
				}
				// Detecting UTF-16 (LE/BE)
				if (i % sizeof(Utf16Le::char_type) == 0 && i + sizeof(Utf16Le::char_type) < inputString.size())
				{
					if (const uint16_t sym = Memory::NativeToLittleEndian(*reinterpret_cast<const uint16_t*>(&inputString[i])); sym != 0)
					{
						if ((sym & 0b1111111100000000) == 0)
						{
							utfType = UtfType::Utf16le;
							break;
						}
						if ((sym & 0b0000000011111111) == 0)
						{
							utfType = UtfType::Utf16be;
							break;
						}
					}
				}
			}
		}
		return utfType;
	}

	/**
	 * @brief Detects UTF encoding from an input stream.
	 */
	[[maybe_unused]] static UtfType DetectEncoding(std::istream& inputStream, bool skipBomWhenFound = true)
	{
		// Read first characters to temporary buffer
		static constexpr size_t tempBufferSize = 128;
		char buffer[tempBufferSize];
		const auto origPos = inputStream.tellg();
		inputStream.read(buffer, tempBufferSize);
		const auto lastReadSize = static_cast<size_t>(inputStream.gcount());

		// Detect encoding
		size_t dataOffset = 0;
		const auto detectedUtf = DetectEncoding(std::string_view(buffer, lastReadSize), dataOffset);

		// Get back to stream position
		if (inputStream.eof())
		{
			// Need reset EOF for able to rewind stream
			inputStream.clear();
		}
		if (skipBomWhenFound)
		{
			if (lastReadSize != dataOffset)
			{
				inputStream.seekg(origPos + static_cast<std::streamoff>(dataOffset));
			}
		}
		else
		{
			inputStream.seekg(origPos);
		}
		return detectedUtf;
	}

	/**
	 * @brief Writes a BOM (Byte Order Mark) to an output stream.
	 */
	static void WriteBom(std::ostream& outputStream, UtfType encoding)
	{
		switch (encoding)
		{
		case UtfType::Utf8:
			outputStream.write(Utf8::bom, sizeof Utf8::bom);
			break;
		case UtfType::Utf16le:
			outputStream.write(Utf16Le::bom, sizeof Utf16Le::bom);
			break;
		case UtfType::Utf16be:
			outputStream.write(Utf16Be::bom, sizeof Utf16Be::bom);
			break;
		case UtfType::Utf32le:
			outputStream.write(Utf32Le::bom, sizeof Utf32Le::bom);
			break;
		case UtfType::Utf32be:
			outputStream.write(Utf32Be::bom, sizeof Utf32Be::bom);
			break;
		}
	}

	//-----------------------------------------------------------------------------
	// Stream Reader / Writer Classes
	//-----------------------------------------------------------------------------

	/**
	 * @brief Result of reading a chunk from an encoded stream.
	 */
	enum class EncodedStreamReadResult
	{
		Success = 0,
		DecodeError,
		EndFile
	};

	/**
	 * @brief Reads UTF-encoded data from a stream with automatic encoding detection.
	 */
	template <typename TTargetCharType, size_t ChunkSize = 256>
	class CEncodedStreamReader
	{
	public:
		static constexpr size_t chunk_size = ChunkSize;

		CEncodedStreamReader(const CEncodedStreamReader&) = delete;
		CEncodedStreamReader(CEncodedStreamReader&&) = delete;
		CEncodedStreamReader& operator=(const CEncodedStreamReader&) = delete;
		CEncodedStreamReader& operator=(CEncodedStreamReader&&) = delete;
		~CEncodedStreamReader() = default;

		explicit CEncodedStreamReader(std::istream& inputStream, UtfEncodingErrorPolicy encodeErrorPolicy = UtfEncodingErrorPolicy::Skip,
			const TTargetCharType* errorMark = Detail::GetDefaultErrorMark<TTargetCharType>())
			: mInputStream(inputStream)
			, mEncodingErrorPolicy(encodeErrorPolicy)
			, mErrorMark(errorMark)
		{
			static_assert((ChunkSize % 4) == 0, "Chunk size must be a multiple of 4");
			static_assert(ChunkSize >= 32, "Chunk size must be at least 32 bytes to correctly detect the encoding");

			if (ReadNextEncodedChunk())
			{
				size_t bomSize = 0;
				mDetectedEncoding = DetectEncoding(std::string_view(mStartDataPtr, mEndDataPtr - mStartDataPtr), bomSize);
				mStartDataPtr += bomSize;
			}
		}

		template<typename TAllocator>
		EncodedStreamReadResult ReadChunk(std::basic_string<TTargetCharType, std::char_traits<TTargetCharType>, TAllocator>& outStr)
		{
			if (IsEnd()) {
				return EncodedStreamReadResult::EndFile;
			}

			if (!ReadNextEncodedChunk() && mStartDataPtr == mEndDataPtr) {
				return EncodedStreamReadResult::EndFile;
			}

			switch (mDetectedEncoding)
			{
			case UtfType::Utf8:
				if constexpr (std::is_same_v<TTargetCharType, char>)
				{
					outStr.append(mStartDataPtr, mEndDataPtr);
					mStartDataPtr = mEndDataPtr = mRawBuffer;
					return EncodedStreamReadResult::Success;
				}
				else
				{
					return DecodeChunk<Utf8>(outStr);
				}
			case UtfType::Utf16le:
				return DecodeChunk<Utf16Le>(outStr);
			case UtfType::Utf16be:
				return DecodeChunk<Utf16Be>(outStr);
			case UtfType::Utf32le:
				return DecodeChunk<Utf32Le>(outStr);
			case UtfType::Utf32be:
				return DecodeChunk<Utf32Be>(outStr);
			}
			assert(false);
			return EncodedStreamReadResult::DecodeError;
		}

		[[nodiscard]] bool IsEnd() const noexcept {
			return mStartDataPtr == mEndDataPtr && mInputStream.eof();
		}

		[[nodiscard]] UtfType GetSourceUtfType() const noexcept {
			return mDetectedEncoding;
		}

	private:
		template <typename T>
		T* GetLastAlignedPosition() noexcept {
			return reinterpret_cast<T*>(mEndDataPtr - ((mEndDataPtr - mStartDataPtr) % sizeof(T)));
		}

		bool ReadNextEncodedChunk()
		{
			if (mStartDataPtr == mBufferEnd)
			{
				mStartDataPtr = mEndDataPtr = mRawBuffer;
			}
			else if (mStartDataPtr != mRawBuffer)
			{
				// Squeeze buffer
				std::memcpy(mRawBuffer, mStartDataPtr, mEndDataPtr - mStartDataPtr);
				mEndDataPtr -= mStartDataPtr - mRawBuffer;
				mStartDataPtr = mRawBuffer;
			}

			// Read next chunk
			mInputStream.read(mEndDataPtr, mBufferEnd - mEndDataPtr);
			const auto lastReadSize = mInputStream.gcount();
			mEndDataPtr += lastReadSize;
			assert(mStartDataPtr >= mRawBuffer && mStartDataPtr <= mEndDataPtr);
			return lastReadSize != 0;
		}

		template <typename TUtf, typename TAllocator>
		EncodedStreamReadResult DecodeChunk(std::basic_string<TTargetCharType, std::char_traits<TTargetCharType>, TAllocator>& outStr)
		{
			const auto result = TUtf::Decode(reinterpret_cast<typename TUtf::char_type*>(mStartDataPtr), GetLastAlignedPosition<typename TUtf::char_type>(), outStr, mEncodingErrorPolicy, mErrorMark);
			mStartDataPtr = reinterpret_cast<char*>(result.Iterator);
			assert(mStartDataPtr <= mEndDataPtr);
			if (mInputStream.eof())
			{
				// Handle incomplete sequence at end of file
				if (result.ErrorCode == UtfEncodingErrorCode::UnexpectedEnd && Detail::HandleEncodingError(outStr, mEncodingErrorPolicy, mErrorMark))
				{
					mStartDataPtr = mEndDataPtr = mRawBuffer;
					return EncodedStreamReadResult::Success;
				}
				return result.ErrorCode == UtfEncodingErrorCode::Success ? EncodedStreamReadResult::Success : EncodedStreamReadResult::DecodeError;
			}
			// Ignore error code `UnexpectedEnd` if it's not end of file
			return result.ErrorCode == UtfEncodingErrorCode::Success || result.ErrorCode == UtfEncodingErrorCode::UnexpectedEnd ? EncodedStreamReadResult::Success : EncodedStreamReadResult::DecodeError;
		}

		UtfType mDetectedEncoding;
		std::istream& mInputStream;
		UtfEncodingErrorPolicy mEncodingErrorPolicy;
		const TTargetCharType* mErrorMark;
		char mRawBuffer[ChunkSize];
		char* mBufferEnd = mRawBuffer + ChunkSize;
		char* mStartDataPtr = mRawBuffer;
		char* mEndDataPtr = mRawBuffer;
	};

	/**
	 * @brief Writes UTF-encoded data to a stream with optional BOM.
	 */
	class CEncodedStreamWriter
	{
	public:
		CEncodedStreamWriter(std::ostream& outputStream, UtfType targetUtfType, bool addBom, UtfEncodingErrorPolicy encodingErrorPolicy = UtfEncodingErrorPolicy::Skip)
			: mOutputStream(outputStream)
			, mEncodingErrorPolicy(encodingErrorPolicy)
		{
			switch (targetUtfType)
			{
			case UtfType::Utf8:
				mUtfToolset.emplace<std::pair<Utf8, std::string>>();
				break;
			case UtfType::Utf16le:
				mUtfToolset.emplace<std::pair<Utf16Le, std::u16string>>();
				break;
			case UtfType::Utf16be:
				mUtfToolset.emplace<std::pair<Utf16Be, std::u16string>>();
				break;
			case UtfType::Utf32le:
				mUtfToolset.emplace<std::pair<Utf32Le, std::u32string>>();
				break;
			case UtfType::Utf32be:
				mUtfToolset.emplace<std::pair<Utf32Be, std::u32string>>();
				break;
			}

			if (addBom)
			{
				WriteBom(mOutputStream, targetUtfType);
			}
		}

		template <typename TCharType>
		UtfEncodingErrorCode Write(const std::basic_string_view<TCharType>& str)
		{
			return std::visit([str, this](auto&& utfToolset) -> UtfEncodingErrorCode
			{
				if constexpr (sizeof(TCharType) == 1 && sizeof(decltype(utfToolset.second.front())) == 1)
				{
					// Write "as is", when source and output are UTF-8
					mOutputStream.write(reinterpret_cast<const char*>(str.data()), static_cast<std::streamsize>(str.size()));
					return UtfEncodingErrorCode::Success;
				}
				else
				{
					utfToolset.second.clear();
					if (auto result = utfToolset.first.Encode(str.data(), str.data() + str.size(), utfToolset.second, mEncodingErrorPolicy))	// NOLINT(bugprone-suspicious-stringview-data-usage)
					{
						mOutputStream.write(reinterpret_cast<const char*>(utfToolset.second.data()),
							static_cast<std::streamsize>(utfToolset.second.size() * sizeof(decltype(utfToolset.second.front()))));
						return UtfEncodingErrorCode::Success;
					}
					else {	// NOLINT(readability-else-after-return)
						return result.ErrorCode;
					}
				}
			}, mUtfToolset);
		}

		template <typename TCharType, typename TAllocator>
		UtfEncodingErrorCode Write(const std::basic_string<TCharType, std::char_traits<TCharType>, TAllocator>& str)
		{
			return Write(std::basic_string_view<TCharType>(str.data(), str.size()));
		}

		template <typename TCharType, size_t ArraySize>
		UtfEncodingErrorCode Write(const TCharType(&str)[ArraySize])
		{
			return Write(std::basic_string_view<TCharType>(std::cbegin(str), std::size(str)));
		}

	private:
		using UtfVariant = std::variant<std::pair<Utf8, std::string>, std::pair<Utf16Le, std::u16string>, std::pair<Utf16Be, std::u16string>, std::pair<Utf32Le, std::u32string>, std::pair<Utf32Be, std::u32string>>;
		std::ostream& mOutputStream;
		UtfVariant mUtfToolset;
		UtfEncodingErrorPolicy mEncodingErrorPolicy;
	};
}

// ToDo: remove in the future versions
namespace BitSerializer::Convert
{
	[[deprecated("Moved into sub-namespace `BitSerializer::Convert::Utf::UtfType`")]]
	typedef Utf::UtfType UtfType;
	[[deprecated("Moved into sub-namespace `BitSerializer::Convert::Utf::Utf8`")]]
	typedef Utf::Utf8 Utf8;
	[[deprecated("Moved into sub-namespace `BitSerializer::Convert::Utf::Utf16Le`")]]
	typedef Utf::Utf16Le Utf16Le;
	[[deprecated("Moved into sub-namespace `BitSerializer::Convert::Utf::Utf16Be`")]]
	typedef Utf::Utf16Be Utf16Be;
	[[deprecated("Moved into sub-namespace `BitSerializer::Convert::Utf::Utf32Le`")]]
	typedef Utf::Utf32Le Utf32Le;
	[[deprecated("Moved into sub-namespace `BitSerializer::Convert::Utf::Utf32Be`")]]
	typedef Utf::Utf32Be Utf32Be;
}
