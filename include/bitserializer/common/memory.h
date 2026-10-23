/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <type_traits>
#include "bitserializer/config.h"
#if defined __has_include && __has_include(<version>)
#include <version>
#endif
#if defined(__cpp_lib_endian)
#include <bit>
#endif
#if BITSERIALIZER_HAS_SSE2
#include <emmintrin.h>
#endif


namespace BitSerializer::Memory
{
#if defined(__cpp_lib_endian)
	/// @brief Standard-compliant enum class representing endianness.
	using Endian = std::endian;
#else
	/**
	 * @brief Indicates the byte order (endianness) of scalar types.
	 */
	enum class Endian
	{
#if defined(__GNUC__)
		little = __ORDER_LITTLE_ENDIAN__,
		big = __ORDER_BIG_ENDIAN__,
		native = __BYTE_ORDER__
#else
		little = 0,
		big = 1,
		native = little
#endif
	};
#endif

	/**
	 * @brief Reverses the byte order of an integral value.
	 *
	 * Specialized for various sizes (1, 2, 4, and 8 bytes).
	 */
	template <typename T, std::enable_if_t<sizeof(T) == 1 && std::is_integral_v<T>, int> = 0>
	constexpr T Reverse(T val) noexcept
	{
		return val; // No-op for single-byte types
	}

	template <typename T, std::enable_if_t<sizeof(T) == 2 && std::is_integral_v<T>, int> = 0>
	constexpr T Reverse(T val) noexcept
	{
		return static_cast<std::uint16_t>((val & 0x00ff) << 8) ^ static_cast<std::uint16_t>((val >> 8) & 0x00ff);
	}

	template <typename T, std::enable_if_t<sizeof(T) == 4 && std::is_integral_v<T>, int> = 0>
	constexpr T Reverse(T val) noexcept
	{
		val = ((val & 0x0000ffff) << 16) ^ ((val >> 16) & 0x0000ffff);
		val = ((val & 0x00ff00ff) << 8) ^ ((val >> 8) & 0x00ff00ff);
		return val;
	}

	template <typename T, std::enable_if_t<sizeof(T) == 8 && std::is_integral_v<T>, int> = 0>
	constexpr T Reverse(T val) noexcept
	{
		val = ((val & 0x00000000ffffffff) << 32) ^ ((val >> 32) & 0x00000000ffffffff);
		val = ((val & 0x0000ffff0000ffff) << 16) ^ ((val >> 16) & 0x0000ffff0000ffff);
		val = ((val & 0x00ff00ff00ff00ff) << 8) ^ ((val >> 8) & 0x00ff00ff00ff00ff);
		return val;
	}

	/**
	 * @brief Reverses byte order in a sequence of integral values.
	 *
	 * @tparam TIterator Type of iterator that points to integral values.
	 * @param in Start of the range.
	 * @param end End of the range.
	 */
	template <typename TIterator, std::enable_if_t<std::is_integral_v<typename std::iterator_traits<TIterator>::value_type>, int> = 0>
	constexpr void Reverse(const TIterator& in, const TIterator& end) noexcept
	{
		if constexpr (sizeof(typename std::iterator_traits<TIterator>::value_type) > 1)
		{
			for (auto it = in; it != end; ++it)
			{
				*it = Reverse(*it);
			}
		}
	}

	/**
	 * @brief Converts a native-endian integer to big-endian format.
	 *
	 * @tparam T Integral type of the value.
	 * @param val Value in native endianness.
	 * @return Value in big-endian format.
	 */
	template <typename T>
	constexpr T NativeToBigEndian(T val) noexcept
	{
		if constexpr (Endian::native == Endian::big)
		{
			return val;
		}
		else
		{
			return Reverse(val);
		}
	}

	/**
	 * @brief Converts a big-endian integer to native endianness.
	 *
	 * @tparam T Integral type of the value.
	 * param val Value in big-endian format.
	 * @return Value in native endianness.
	 */
	template <typename T>
	constexpr T BigEndianToNative(T val) noexcept
	{
		if constexpr (Endian::native == Endian::big)
		{
			return val;
		}
		else
		{
			return Reverse(val);
		}
	}

	/**
	 * @brief Converts a native-endian integer to little-endian format.
	 *
	 * @tparam T Integral type of the value.
	 * @param val Value in native endianness.
	 * @return Value in little-endian format.
	 */
	template <typename T>
	constexpr T NativeToLittleEndian(T val) noexcept
	{
		if constexpr (Endian::native == Endian::little)
		{
			return val;
		}
		else
		{
			return Reverse(val);
		}
	}

	/**
	 * @brief Converts a little-endian integer to native endianness.
	 *
	 * @tparam T Integral type of the value.
	 * @param val Value in little-endian format.
	 * @return Value in native endianness.
	 */
	template <typename T>
	constexpr T LittleEndianToNative(T val) noexcept
	{
		if constexpr (Endian::native == Endian::little)
		{
			return val;
		}
		else
		{
			return Reverse(val);
		}
	}

	/**
	 * Iterator adapter that returns integer values in reversed byte order.
	 *
	 * Useful when reading from or writing to memory with mismatched endianness.
	 */
	template <typename TBaseIt>
	class ReverseEndianIterator
	{
	public:
		using iterator_category = typename std::iterator_traits<TBaseIt>::iterator_category;
		using value_type = typename std::iterator_traits<TBaseIt>::value_type;
		using difference_type = typename std::iterator_traits<TBaseIt>::difference_type;
		using pointer = typename std::iterator_traits<TBaseIt>::pointer;
		using reference = typename std::iterator_traits<TBaseIt>::reference;

		explicit ReverseEndianIterator(TBaseIt it)
			: mBaseIt(std::move(it))
		{
			static_assert(std::is_integral_v<value_type>, "ReverseEndianIterator only supports integral types");
		}

		value_type operator*() const noexcept {
			return Reverse(*mBaseIt);
		}
		constexpr difference_type operator-(const ReverseEndianIterator<TBaseIt>& rhs) const noexcept {
			return mBaseIt - rhs.mBaseIt;
		}
		constexpr difference_type operator+(const ReverseEndianIterator<TBaseIt>& rhs) const noexcept {
			return mBaseIt + rhs.mBaseIt;
		}
		constexpr ReverseEndianIterator<TBaseIt>& operator+=(difference_type diff) noexcept {
			mBaseIt += diff;
			return *this;
		}
		constexpr ReverseEndianIterator<TBaseIt>& operator-=(difference_type diff) noexcept {
			mBaseIt -= diff;
			return *this;
		}
		bool operator==(const ReverseEndianIterator<TBaseIt>& rhs) const noexcept { return mBaseIt == rhs.mBaseIt; }
		bool operator!=(const ReverseEndianIterator<TBaseIt>& rhs) const noexcept { return mBaseIt != rhs.mBaseIt; }

		ReverseEndianIterator& operator++() noexcept
		{
			++mBaseIt;
			return *this;
		}

		operator const TBaseIt& () const { return mBaseIt; }

	private:
		TBaseIt mBaseIt;
	};

	/**
	 * @brief Makes an iterator adapter that converts integers to native endianness.
	 *
	 * If the source endianness matches native or the value size is 1 byte, no conversion occurs.
	 *
	 * @tparam SourceEndianness Endianness of the input data.
	 * @tparam TBaseIt Type of base iterator.
	 * @param it Base iterator.
	 * @return Either the original iterator or a wrapped `ReverseEndianIterator`.
	 */
	template <Endian SourceEndianness, typename TBaseIt>
	constexpr auto MakeIteratorAdapter(TBaseIt it)
	{
		if constexpr (SourceEndianness == Endian::native || sizeof(typename std::iterator_traits<TBaseIt>::value_type) == 1)
		{
			return it;
		}
		else
		{
			return ReverseEndianIterator(std::move(it));
		}
	}

	namespace Detail
	{
		/**
		 * @brief Compares exactly `Count` bytes, with the count known at compile time.
		 *
		 * For short ranges the recursion is fully unrolled into a fixed sequence of integer loads,
		 * so no runtime loop, length test or overlapping tail is needed. From 32 bytes SSE2 loads
		 * are used (the tail overlaps bytes already compared), and very long ranges fall back to
		 * `std::memcmp` to avoid excessive code expansion. Clang's `memcmp` is faster than the
		 * scalar path for 17-31 bytes, so that narrow range is delegated to it (MSVC is the
		 * opposite). Every load stays inside the `[data, data + Count)` range; the caller
		 * guarantees `Count` readable bytes on both sides.
		 */
		template <std::size_t Count>
		[[nodiscard]] inline bool EqualBytesFixed(const char* lhs, const char* rhs) noexcept
		{
#if defined(__clang__) && (defined(_M_X64) || defined(__x86_64__))
			if constexpr ((Count >= 17 && Count <= 31) || Count > 256)
			{
				return std::memcmp(lhs, rhs, Count) == 0;
			}
			else
#else
			if constexpr (Count > 256)
			{
				return std::memcmp(lhs, rhs, Count) == 0;
			}
			else
#endif
#if BITSERIALIZER_HAS_SSE2
			if constexpr (Count >= 32 && Count <= 256)
			{
				std::size_t i = 0;
				for (; i + 16 <= Count; i += 16)
				{
					const __m128i a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(lhs + i));
					const __m128i b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(rhs + i));
					if (_mm_movemask_epi8(_mm_cmpeq_epi8(a, b)) != 0xFFFF) {
						return false;
					}
				}
				if constexpr (Count % 16 != 0)
				{
					// The tail overlaps the previous block; Count >= 16 guarantees it is in bounds.
					constexpr std::size_t tail = Count - 16;
					const __m128i a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(lhs + tail));
					const __m128i b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(rhs + tail));
					return _mm_movemask_epi8(_mm_cmpeq_epi8(a, b)) == 0xFFFF;
				}
				else {
					return true;
				}
			}
			else
#endif
			if constexpr (Count >= 8)
			{
				std::uint64_t l, r;
				std::memcpy(&l, lhs, 8);
				std::memcpy(&r, rhs, 8);
				if (l != r) {
					return false;
				}
				return EqualBytesFixed<Count - 8>(lhs + 8, rhs + 8);
			}
			else if constexpr (Count >= 4)
			{
				std::uint32_t l, r;
				std::memcpy(&l, lhs, 4);
				std::memcpy(&r, rhs, 4);
				if (l != r) {
					return false;
				}
				return EqualBytesFixed<Count - 4>(lhs + 4, rhs + 4);
			}
			else if constexpr (Count >= 2)
			{
				std::uint16_t l, r;
				std::memcpy(&l, lhs, 2);
				std::memcpy(&r, rhs, 2);
				if (l != r) {
					return false;
				}
				return EqualBytesFixed<Count - 2>(lhs + 2, rhs + 2);
			}
			else if constexpr (Count == 1) {
				return *lhs == *rhs;
			}
			else {
				return true;
			}
		}
	}

	/**
	 * @brief Compares exactly `Count` bytes, with the count known at compile time.
	 *
	 * Dispatches to `Detail::EqualBytesFixed` (scalar unrolled loads, SSE2 or `memcmp` depending
	 * on `Count`). Both buffers must contain at least `Count` readable bytes.
	 */
	template <std::size_t Count>
	[[nodiscard]] inline bool EqualBytes(const void* lhs, const void* rhs) noexcept
	{
		return Detail::EqualBytesFixed<Count>(static_cast<const char*>(lhs), static_cast<const char*>(rhs));
	}

	/**
	 * @brief Compares two byte ranges word-at-a-time, without going through `std::memcmp`.
	 *
	 * Cheaper than `std::memcmp` for the short ranges that dominate (keys are usually 8-32 bytes):
	 * it avoids a library call and the per-call setup, comparing 8 bytes per step and finishing with
	 * a single overlapping 8-byte load. The trailing load overlaps bytes already compared rather than
	 * reading past the end, so no caller-provided padding is required.
	 */
	[[nodiscard]] inline bool EqualBytes(const void* lhs, const void* rhs, std::size_t count) noexcept
	{
		const char* l = static_cast<const char*>(lhs);
		const char* r = static_cast<const char*>(rhs);

		if (count > 7)
		{
			std::uint64_t words[2];
			while (count > 8)
			{
				std::memcpy(words, l, 8);
				std::memcpy(words + 1, r, 8);
				if (words[0] != words[1]) {
					return false;
				}
				count -= 8;
				l += 8;
				r += 8;
			}
			l -= 8 - count;
			r -= 8 - count;
			std::memcpy(words, l, 8);
			std::memcpy(words + 1, r, 8);
			return words[0] == words[1];
		}

		std::uint32_t word32[2];
		if (count >= 4)
		{
			std::memcpy(word32, l, 4);
			std::memcpy(word32 + 1, r, 4);
			if (word32[0] != word32[1]) {
				return false;
			}
			count -= 4;
			l += 4;
			r += 4;
		}

		std::uint16_t word16[2];
		if (count >= 2)
		{
			std::memcpy(word16, l, 2);
			std::memcpy(word16 + 1, r, 2);
			if (word16[0] != word16[1]) {
				return false;
			}
			count -= 2;
			l += 2;
			r += 2;
		}

		return count == 0 || *l == *r;
	}
}
