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
	using Endian = std::endian;
#else
	/// <summary>
	/// Indicates the endianness of all scalar types.
	/// </summary>
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

	/// <summary>
	/// Returns integral values in reversed byte order (implementation for uint8_t returns as is).
	/// </summary>
	template <typename T, std::enable_if_t<sizeof(T) == 1 && std::is_integral_v<T>, int> = 0>
	constexpr T Reverse(T val) noexcept
	{
		return val;
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

	/// <summary>
	/// Reverses byte order in the passed sequence of integral values.
	/// </summary>
	template <typename TIterator, std::enable_if_t<std::is_integral_v<typename std::iterator_traits<TIterator>::value_type>, int> = 0>
	constexpr void Reverse(TIterator in, const TIterator& end) noexcept
	{
		if constexpr (sizeof (typename std::iterator_traits<TIterator>::value_type) > 1)
		{
			for (auto it = in; it != end; ++it) {
				*it = Reverse(*it);
			}
		}
	}

	/// <summary>
	/// Converts native representation of integer value to big endian.
	/// </summary>
	template <typename T>
	constexpr T NativeToBigEndian(T val) noexcept
	{
		if constexpr (Endian::native == Endian::big) {
			return val;
		}
		else {
			return Reverse(val);
		}
	}

	/// <summary>
	/// Converts big endian representation of integer value to native.
	/// </summary>
	template <typename T>
	constexpr T BigEndianToNative(T val) noexcept
	{
		if constexpr (Endian::native == Endian::big) {
			return val;
		}
		else {
			return Reverse(val);
		}
	}

	/// <summary>
	/// Converts native representation of integer value to little endian.
	/// </summary>
	template <typename T>
	constexpr T NativeToLittleEndian(T val) noexcept
	{
		if constexpr (Endian::native == Endian::little)	{
			return val;
		}
		else {
			return Reverse(val);
		}
	}

	/// <summary>
	/// Converts little endian representation of integer value to native.
	/// </summary>
	template <typename T>
	constexpr T LittleEndianToNative(T val) noexcept
	{
		if constexpr (Endian::native == Endian::little) {
			return val;
		}
		else {
			return Reverse(val);
		}
	}

	/// <summary>
	/// Wraps an iterator that refers to integer type, returns values in reverse byte order.
	/// </summary>
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
			static_assert(std::is_integral_v<value_type>, "Reverse endian iterator supports only integral types");
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
		TBaseIt& operator++() noexcept { return ++mBaseIt; }

		operator const TBaseIt& () const { return mBaseIt; }

	private:
		TBaseIt mBaseIt;
	};

	/// <summary>
	/// Makes an adapter for an iterator that converts integers to native endianness.
	/// </summary>
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
