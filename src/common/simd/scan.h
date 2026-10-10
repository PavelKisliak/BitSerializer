/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <array>
#include <cstddef>
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

namespace BitSerializer::Detail
{
	namespace ScanDetail
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
	[[nodiscard]] size_t FindFirstOf(std::string_view data, size_t pos) noexcept
	{
		static_assert(sizeof...(TChars) > 0, "BitSerializer. At least one character must be specified");

		const size_t size = data.size();
		if (pos >= size) {
			return std::string_view::npos;
		}

#if BITSERIALIZER_HAS_SSE2
		const char* const base = data.data();
		static constexpr std::array<bool, 256> matchTable = ScanDetail::MakeByteMatchTable<0, TChars...>();
		size_t i = pos;
		for (; i + 16 <= size; i += 16)
		{
			const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(base + i));
			__m128i match = _mm_setzero_si128();
			((match = _mm_or_si128(match, _mm_cmpeq_epi8(chunk, _mm_set1_epi8(TChars)))), ...);
			const int mask = _mm_movemask_epi8(match);
			if (mask != 0)
			{
				// The lowest set bit corresponds to the first matching byte in the block.
				int bitIndex;
#if defined(_MSC_VER)
				unsigned long rawIndex;
				_BitScanForward(&rawIndex, static_cast<unsigned long>(mask));
				bitIndex = static_cast<int>(rawIndex);
#else
				bitIndex = __builtin_ctz(static_cast<unsigned int>(mask));
#endif
				return i + static_cast<size_t>(bitIndex);
			}
		}
		for (; i < size; ++i)
		{
			if (matchTable[static_cast<unsigned char>(base[i])]) {
				return i;
			}
		}
		return std::string_view::npos;
#elif BITSERIALIZER_HAS_NEON
		const char* const base = data.data();
		static constexpr std::array<bool, 256> matchTable = ScanDetail::MakeByteMatchTable<0, TChars...>();
		size_t i = pos;
		for (; i + 16 <= size; i += 16)
		{
			const uint8x16_t chunk = vld1q_u8(reinterpret_cast<const uint8_t*>(base + i));
			uint8x16_t match = vdupq_n_u8(0);
			((match = vorrq_u8(match, vceqq_u8(chunk, vdupq_n_u8(static_cast<uint8_t>(TChars))))), ...);
			const uint64x2_t bits = vreinterpretq_u64_u8(match);
			if ((vgetq_lane_u64(bits, 0) | vgetq_lane_u64(bits, 1)) != 0)
			{
				// Locate the exact byte in this block (byte-wise, so independent of endianness).
				for (size_t k = 0; k < 16; ++k)
				{
					if (((base[i + k] == TChars) || ...)) {
						return i + k;
					}
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
#else
		// No SIMD: a dedicated scalar loop is faster only for short inputs, while the standard
		// search wins on longer ones and supports arbitrary set sizes.
		static constexpr char needle[] = { TChars... };
		return data.find_first_of(needle, pos, sizeof...(TChars));
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
	[[nodiscard]] size_t FindFirstOfOrLessThan(std::string_view data, size_t pos) noexcept
	{
		const size_t size = data.size();
		if (pos >= size) {
			return std::string_view::npos;
		}

#if BITSERIALIZER_HAS_SSE2
		const char* const base = data.data();
		static constexpr std::array<bool, 256> matchTable = ScanDetail::MakeByteMatchTable<static_cast<unsigned char>(TLimit), TChars...>();
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
			if (mask != 0)
			{
				int bitIndex;
#if defined(_MSC_VER)
				unsigned long rawIndex;
				_BitScanForward(&rawIndex, static_cast<unsigned long>(mask));
				bitIndex = static_cast<int>(rawIndex);
#else
				bitIndex = __builtin_ctz(static_cast<unsigned int>(mask));
#endif
				return i + static_cast<size_t>(bitIndex);
			}
		}
		for (; i < size; ++i)
		{
			if (matchTable[static_cast<unsigned char>(base[i])]) {
				return i;
			}
		}
		return std::string_view::npos;
#elif BITSERIALIZER_HAS_NEON
		const char* const base = data.data();
		static constexpr std::array<bool, 256> matchTable = ScanDetail::MakeByteMatchTable<static_cast<unsigned char>(TLimit), TChars...>();
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
			if ((vgetq_lane_u64(bits, 0) | vgetq_lane_u64(bits, 1)) != 0)
			{
				for (size_t k = 0; k < 16; ++k)
				{
					if ((static_cast<unsigned char>(base[i + k]) < static_cast<unsigned char>(TLimit)) || ((base[i + k] == TChars) || ...)) {
						return i + k;
					}
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
#else
		const char* const base = data.data();
		static constexpr std::array<bool, 256> matchTable = ScanDetail::MakeByteMatchTable<static_cast<unsigned char>(TLimit), TChars...>();
		for (size_t i = pos; i < size; ++i)
		{
			if (matchTable[static_cast<unsigned char>(base[i])]) {
				return i;
			}
		}
		return std::string_view::npos;
#endif
	}
}
