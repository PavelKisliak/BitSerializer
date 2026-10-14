/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>
#include "bitserializer/config.h"

#if BITSERIALIZER_HAS_SSE2
	#include <emmintrin.h>
	#if defined(_MSC_VER)
		#include <intrin.h>
	#endif
#elif BITSERIALIZER_HAS_NEON
	#if defined(_M_ARM64) || defined(_M_ARM64EC)
		#include <arm64_neon.h>
	#else
		#include <arm_neon.h>
	#endif
#endif

namespace BitSerializer::Simd
{
	namespace Detail
	{
		/**
		 * @brief Builds a 256-entry lookup table marking bytes that match the search criteria:
		 * `< TLimit` (unsigned) or equal to any of `TChars`. `TLimit == 0` disables the range test.
		 *
		 * Used by the scalar tail of the scanners, where a single table load is cheaper than
		 * several per-byte comparisons (JSON keys/values are usually shorter than one SIMD block).
		 */
		template <unsigned char TLimit, char... TChars>
		[[nodiscard]] constexpr std::array<bool, 256> MakeByteMatchTable() noexcept
		{
			std::array<bool, 256> table{};
			for (size_t c = 0; c < 256; ++c)
			{
				const unsigned char uc = static_cast<unsigned char>(c);
				table[c] = (uc < TLimit) || ((uc == static_cast<unsigned char>(TChars)) || ... || false);
			}
			return table;
		}

		/**
		 * @brief SWAR word type: 64-bit on 64-bit targets, 32-bit otherwise. On 32-bit x86 a `uint64_t`
		 * is emulated with two 32-bit ops, which makes an 8-byte SWAR slower than the 4-byte one.
		 */
		using swar_word_t = uintptr_t;

		/**
		 * @brief Returns a mask whose high bit is set for every byte of `word` equal to any of `TChars`
		 * (SWAR, no SIMD). Used as a cheap pre-check for short inputs before the scalar tail.
		 */
		template <typename TWord, char... TChars>
		[[nodiscard]] BITSERIALIZER_FORCEINLINE TWord SwarMatch(TWord word) noexcept
		{
			constexpr TWord ones = static_cast<TWord>(~static_cast<TWord>(0) / 0xFFu);
			constexpr TWord high = static_cast<TWord>(ones * 0x80u);
			TWord match = 0;
			((match |= ((word ^ (ones * static_cast<TWord>(static_cast<unsigned char>(TChars)))) - ones)
						& ~(word ^ (ones * static_cast<TWord>(static_cast<unsigned char>(TChars))))
						& high), ...);
			return match;
		}

		/**
		 * @brief Same as above, but also matches a character supplied at runtime.
		 */
		template <typename TWord, char... TChars>
		[[nodiscard]] BITSERIALIZER_FORCEINLINE TWord SwarMatch(TWord word, char extraChar) noexcept
		{
			constexpr TWord ones = static_cast<TWord>(~static_cast<TWord>(0) / 0xFFu);
			constexpr TWord high = static_cast<TWord>(ones * 0x80u);
			const TWord xe = word ^ (ones * static_cast<TWord>(static_cast<unsigned char>(extraChar)));
			TWord match = (xe - ones) & ~xe & high;
			((match |= ((word ^ (ones * static_cast<TWord>(static_cast<unsigned char>(TChars)))) - ones)
						& ~(word ^ (ones * static_cast<TWord>(static_cast<unsigned char>(TChars))))
						& high), ...);
			return match;
		}

		/**
		 * @brief Scalar scan of the trailing bytes for a byte set defined by a match table
		 * (`< TLimit` unsigned, or equal to any of `TChars`), starting at `pos`.
		 *
		 * Shared by the SSE2/NEON/scalar branches of the scanners and by the NEON block locator.
		 */
		template <unsigned char TLimit, char... TChars>
		[[nodiscard]] BITSERIALIZER_FORCEINLINE size_t ScanScalar(std::string_view data, size_t pos) noexcept
		{
			static constexpr std::array<bool, 256> matchTable = MakeByteMatchTable<TLimit, TChars...>();
			const char* const base = data.data();
			const size_t size = data.size();
			size_t i = pos;
			if constexpr (TLimit == 0)
			{
				// SWAR pre-check (word-sized) for short inputs that never reach the SIMD block.
				for (; i + sizeof(swar_word_t) <= size; i += sizeof(swar_word_t))
				{
					swar_word_t word;
					std::memcpy(&word, base + i, sizeof(word));
					if (SwarMatch<swar_word_t, TChars...>(word) != 0) {
						break;
					}
				}
			}
			for (; i < size; ++i)
			{
				if (matchTable[static_cast<unsigned char>(base[i])]) {
					return i;
				}
			}
			return std::string_view::npos;
		}

		/**
		 * @brief Scalar scan of the trailing bytes for a byte set defined by a match table plus one
		 * character supplied at runtime (e.g. a configurable CSV separator).
		 */
		template <char... TChars>
		[[nodiscard]] BITSERIALIZER_FORCEINLINE size_t ScanScalar(std::string_view data, size_t pos, char extraChar) noexcept
		{
			static constexpr std::array<bool, 256> matchTable = MakeByteMatchTable<0, TChars...>();
			const char* const base = data.data();
			const size_t size = data.size();
			size_t i = pos;
			for (; i + sizeof(swar_word_t) <= size; i += sizeof(swar_word_t))
			{
				swar_word_t word;
				std::memcpy(&word, base + i, sizeof(word));
				if (SwarMatch<swar_word_t, TChars...>(word, extraChar) != 0) {
					break;
				}
			}
			for (; i < size; ++i)
			{
				if (base[i] == extraChar || matchTable[static_cast<unsigned char>(base[i])]) {
					return i;
				}
			}
			return std::string_view::npos;
		}

#if BITSERIALIZER_HAS_SSE2
		/**
		 * @brief Returns the index of the lowest set bit in a 16-lane mask produced by `_mm_movemask_epi8`.
		 */
		[[nodiscard]] BITSERIALIZER_FORCEINLINE size_t FirstSetBit(int mask) noexcept
		{
#if defined(_MSC_VER)
			unsigned long rawIndex;
			_BitScanForward(&rawIndex, static_cast<unsigned long>(mask));
			return static_cast<size_t>(rawIndex);
#else
			return static_cast<size_t>(__builtin_ctz(static_cast<unsigned int>(mask)));
#endif
		}
#endif
	}

	/**
	 * @brief Returns the index of the first occurrence of `TChars` at or after `pos`,
	 * or `std::string_view::npos` if none is found.
	 *
	 * On SSE2/NEON targets it scans 16 bytes per iteration and locates the exact byte within the block;
	 * the trailing (<16) bytes are scanned scalar. On other targets it falls back to `std::string_view::find_first_of`.
	 * Byte comparisons are performed per lane, so the result is independent of endianness.
	 *
	 * @note Intended for small character sets (typically 1-4). There is no hard upper bound, but each extra
	 * character adds one vector comparison per block, so very large sets are not the target use case.
	 *
	 * Typical usage (JSON): `FindFirstOf<'"', '\\'>(input, pos)`.
	 *
	 * @tparam TChars Characters to search for (at least one).
	 * @param data    Input buffer.
	 * @param pos     Start position.
	 */
	template <char... TChars>
	[[nodiscard]] BITSERIALIZER_FORCEINLINE size_t FindFirstOf(std::string_view data, size_t pos) noexcept
	{
		static_assert(sizeof...(TChars) > 0, "BitSerializer. At least one character must be specified");

		const size_t size = data.size();
		if (pos >= size) {
			return std::string_view::npos;
		}

#if BITSERIALIZER_HAS_SSE2
		const char* const base = data.data();
		size_t i = pos;
		for (; i + 16 <= size; i += 16)
		{
			const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(base + i));
			__m128i match = _mm_setzero_si128();
			((match = _mm_or_si128(match, _mm_cmpeq_epi8(chunk, _mm_set1_epi8(TChars)))), ...);
			const int mask = _mm_movemask_epi8(match);
			if (mask != 0) {
				// The lowest set bit corresponds to the first matching byte in the block.
				return i + Detail::FirstSetBit(mask);
			}
		}
		return Detail::ScanScalar<0, TChars...>(data, i);
#elif BITSERIALIZER_HAS_NEON
		const char* const base = data.data();
		size_t i = pos;
		for (; i + 16 <= size; i += 16)
		{
			const uint8x16_t chunk = vld1q_u8(reinterpret_cast<const uint8_t*>(base + i));
			uint8x16_t match = vdupq_n_u8(0);
			((match = vorrq_u8(match, vceqq_u8(chunk, vdupq_n_u8(static_cast<uint8_t>(TChars))))), ...);
			const uint64x2_t bits = vreinterpretq_u64_u8(match);
			if ((vgetq_lane_u64(bits, 0) | vgetq_lane_u64(bits, 1)) != 0) {
				// A match exists in this block - locate it with the scalar matcher (same byte set).
				return Detail::ScanScalar<0, TChars...>(data, i);
			}
		}
		return Detail::ScanScalar<0, TChars...>(data, i);
#else
		// No SIMD: a dedicated scalar loop is faster only for short inputs, while the standard
		// search wins on longer ones and supports arbitrary set sizes.
		static constexpr char needle[] = { TChars... };
		return data.find_first_of(needle, pos, sizeof...(TChars));
#endif
	}

	/**
	 * @brief Returns the index of the first byte equal to any of `TChars` or to `extraChar` at or after `pos`,
	 * or `std::string_view::npos` if none is found.
	 *
	 * Runtime-character counterpart of `FindFirstOf`, for a character that is not known at compile time
	 * (e.g. a configurable CSV separator) combined with a fixed compile-time set. On SSE2/NEON targets it
	 * scans 16 bytes per iteration and locates the exact byte within the block; the trailing (<16) bytes
	 * are scanned scalar. On other targets a scalar loop is used.
	 *
	 * @tparam TChars Compile-time characters to search for (may be empty).
	 * @param data      Input buffer.
	 * @param pos       Start position.
	 * @param extraChar Additional character supplied at runtime.
	 */
	template <char... TChars>
	[[nodiscard]] BITSERIALIZER_FORCEINLINE size_t FindFirstOf(std::string_view data, size_t pos, char extraChar) noexcept
	{
		const size_t size = data.size();
		if (pos >= size) {
			return std::string_view::npos;
		}

#if BITSERIALIZER_HAS_SSE2
		const char* const base = data.data();
		const __m128i extra = _mm_set1_epi8(extraChar);
		size_t i = pos;
		for (; i + 16 <= size; i += 16)
		{
			const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(base + i));
			__m128i match = _mm_cmpeq_epi8(chunk, extra);
			((match = _mm_or_si128(match, _mm_cmpeq_epi8(chunk, _mm_set1_epi8(TChars)))), ...);
			const int mask = _mm_movemask_epi8(match);
			if (mask != 0) {
				return i + Detail::FirstSetBit(mask);
			}
		}
		return Detail::ScanScalar<TChars...>(data, i, extraChar);
#elif BITSERIALIZER_HAS_NEON
		const char* const base = data.data();
		const uint8x16_t vextra = vdupq_n_u8(static_cast<uint8_t>(extraChar));
		size_t i = pos;
		for (; i + 16 <= size; i += 16)
		{
			const uint8x16_t chunk = vld1q_u8(reinterpret_cast<const uint8_t*>(base + i));
			uint8x16_t match = vceqq_u8(chunk, vextra);
			((match = vorrq_u8(match, vceqq_u8(chunk, vdupq_n_u8(static_cast<uint8_t>(TChars))))), ...);
			const uint64x2_t bits = vreinterpretq_u64_u8(match);
			if ((vgetq_lane_u64(bits, 0) | vgetq_lane_u64(bits, 1)) != 0) {
				return Detail::ScanScalar<TChars...>(data, i, extraChar);
			}
		}
		return Detail::ScanScalar<TChars...>(data, i, extraChar);
#else
		return Detail::ScanScalar<TChars...>(data, pos, extraChar);
#endif
	}

	/**
	 * @brief Returns the index of the first byte that is equal to any of `TChars` or is less than
	 * `TLimit` (unsigned comparison), at or after `pos`, or `std::string_view::npos` if none is found.
	 *
	 * The `< TLimit` test is unsigned, so bytes `>= 0x80` are never matched by a small limit even where
	 * `char` is signed. On SSE2/NEON targets it scans 16 bytes per iteration and locates the exact byte
	 * within the block; the trailing (<16) bytes are scanned scalar. On other targets a scalar loop is used.
	 *
	 * @note Intended for small character sets (typically 1-4). There is no hard upper bound.
	 *
	 * Typical usage (JSON string escaping): `FindFirstOfOrLessThan<0x20, '"', '\\'>(input, pos)`
	 * finds the first byte that must be escaped, i.e. `"`, `\` or a control character below 0x20.
	 *
	 * @tparam TLimit Upper (exclusive) limit of the unsigned byte range.
	 * @tparam TChars Characters to search for (may be empty).
	 * @param data    Input buffer.
	 * @param pos     Start position.
	 */
	template <char TLimit, char... TChars>
	[[nodiscard]] BITSERIALIZER_FORCEINLINE size_t FindFirstOfOrLessThan(std::string_view data, size_t pos) noexcept
	{
		const size_t size = data.size();
		if (pos >= size) {
			return std::string_view::npos;
		}

#if BITSERIALIZER_HAS_SSE2
		const char* const base = data.data();
		size_t i = pos;
		for (; i + 16 <= size; i += 16)
		{
			const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(base + i));
			__m128i match = _mm_setzero_si128();
			if constexpr (static_cast<unsigned char>(TLimit) > 0) {
				// min_epu8(chunk, TLimit - 1) == chunk  <=>  (unsigned)chunk < TLimit
				const char belowLimit = static_cast<char>(static_cast<unsigned char>(TLimit) - 1);
				match = _mm_cmpeq_epi8(_mm_min_epu8(chunk, _mm_set1_epi8(belowLimit)), chunk);
			}
			((match = _mm_or_si128(match, _mm_cmpeq_epi8(chunk, _mm_set1_epi8(TChars)))), ...);
			const int mask = _mm_movemask_epi8(match);
			if (mask != 0) {
				return i + Detail::FirstSetBit(mask);
			}
		}
		return Detail::ScanScalar<static_cast<unsigned char>(TLimit), TChars...>(data, i);
#elif BITSERIALIZER_HAS_NEON
		const char* const base = data.data();
		size_t i = pos;
		for (; i + 16 <= size; i += 16)
		{
			const uint8x16_t chunk = vld1q_u8(reinterpret_cast<const uint8_t*>(base + i));
			uint8x16_t match = vdupq_n_u8(0);
			if constexpr (static_cast<unsigned char>(TLimit) > 0) {
				match = vcltq_u8(chunk, vdupq_n_u8(static_cast<uint8_t>(TLimit)));
			}
			((match = vorrq_u8(match, vceqq_u8(chunk, vdupq_n_u8(static_cast<uint8_t>(TChars))))), ...);
			const uint64x2_t bits = vreinterpretq_u64_u8(match);
			if ((vgetq_lane_u64(bits, 0) | vgetq_lane_u64(bits, 1)) != 0) {
				return Detail::ScanScalar<static_cast<unsigned char>(TLimit), TChars...>(data, i);
			}
		}
		return Detail::ScanScalar<static_cast<unsigned char>(TLimit), TChars...>(data, i);
#else
		return Detail::ScanScalar<static_cast<unsigned char>(TLimit), TChars...>(data, pos);
#endif
	}
}
