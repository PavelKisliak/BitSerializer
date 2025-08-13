/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <iterator>
#include <type_traits>
#if defined __has_include && __has_include(<version>)
#include <version>
#endif
#if defined(__cpp_lib_endian)
#include <bit>
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
}
