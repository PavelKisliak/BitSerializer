/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 202002L) || __cplusplus >= 202002L)
#include <bit>
#endif


namespace BitSerializer::Memory
{
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 202002L) || __cplusplus >= 202002L)
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
		using value_type = std::decay_t<decltype(*std::declval<TBaseIt>())>;

		explicit ReverseEndianIterator(TBaseIt it)
			: mBaseIt(std::move(it))
		{
			static_assert(std::is_integral_v<value_type>, "Reverse endian iterator supports only integral types");
		}

		value_type operator*() const noexcept {
			return Reverse(*mBaseIt);
		}

		bool operator==(const TBaseIt& rhs) const { return mBaseIt == rhs; }
		bool operator!=(const TBaseIt& rhs) const { return mBaseIt != rhs; }
		TBaseIt& operator++() noexcept { return ++mBaseIt; }
		operator const TBaseIt& () const { return mBaseIt; }

	private:
		TBaseIt mBaseIt;
	};
}
