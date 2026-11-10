/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>
#include "bitserializer/config.h"
#include "bitserializer/common/memory.h"

namespace BitSerializer::Convert::Detail
{
	/**
	 * @brief Maximum number of characters written by `FormatInteger()`.
	 *
	 * Covers the 20 digits of `uint64_t`, a possible minus sign and one byte of slack
	 * used by the two-digit store helpers.
	 */
	inline constexpr std::size_t MaxIntegerLength = 22;

	/**
	 * @brief Maximum number of characters needed by `FormatInteger()` for a concrete integer type.
	 *
	 * `digits10 + 1` covers the maximum digit count, the sign adds one more character and the
	 * final byte is slack for the internal two-byte store helpers.
	 */
	template <typename T>
	constexpr std::size_t MaxIntegerLengthFor() noexcept
	{
		return std::numeric_limits<T>::digits10 + 2 + (std::is_signed_v<T> ? 1 : 0);
	}

	namespace IntegerFormatDetail
	{
		/// Two-digit character pairs `"00"`..`"99"` (200 bytes).
		alignas(64) inline constexpr auto DigitChars = []() constexpr
		{
			std::array<char, 200> table{};
			for (std::size_t i = 0; i < 100; ++i)
			{
				table[i * 2] = static_cast<char>('0' + i / 10);
				table[i * 2 + 1] = static_cast<char>('0' + i % 10);
			}
			return table;
		}();

		/// Two-digit pairs as 16-bit words for direct two-byte stores (200 bytes).
		inline constexpr auto DigitPairs = []() constexpr
		{
			std::array<std::uint16_t, 100> table{};
			for (std::uint16_t i = 0; i < 100; ++i)
			{
				const auto tens = static_cast<std::uint16_t>('0' + i / 10);
				const auto ones = static_cast<std::uint16_t>('0' + i % 10);
				table[i] = Memory::Endian::native == Memory::Endian::little
					? static_cast<std::uint16_t>(tens | (ones << 8))
					: static_cast<std::uint16_t>((tens << 8) | ones);
			}
			return table;
		}();

		BITSERIALIZER_FORCEINLINE char* Write8Fixed(char* out, std::uint32_t value) noexcept
		{
			const std::uint32_t high = value / 10000;
			const std::uint32_t low = value - high * 10000;
			const std::uint32_t highHigh = high / 100;
			const std::uint32_t lowHigh = low / 100;
			std::memcpy(out, &DigitPairs[highHigh], 2);
			std::memcpy(out + 2, &DigitPairs[high - highHigh * 100], 2);
			std::memcpy(out + 4, &DigitPairs[lowHigh], 2);
			std::memcpy(out + 6, &DigitPairs[low - lowHigh * 100], 2);
			return out + 8;
		}

		BITSERIALIZER_FORCEINLINE char* Write2(char* out, std::uint32_t value) noexcept
		{
			const std::uint32_t leadingZero = value < 10 ? 1u : 0u;
			std::memcpy(out, DigitChars.data() + ((value * 2) | leadingZero), 2);
			return out + 2 - leadingZero;
		}

		BITSERIALIZER_FORCEINLINE char* Write4(char* out, std::uint32_t value) noexcept
		{
			const std::uint32_t high = value / 100;
			const std::uint32_t leadingZero = high < 10 ? 1u : 0u;
			std::memcpy(out, DigitChars.data() + ((high * 2) | leadingZero), 2);
			std::memcpy(out + 2 - leadingZero, &DigitPairs[value - high * 100], 2);
			return out + 4 - leadingZero;
		}

		BITSERIALIZER_FORCEINLINE char* Write6(char* out, std::uint32_t value) noexcept
		{
			const std::uint32_t high = value / 10000;
			const std::uint32_t low = value - high * 10000;
			const std::uint32_t lowHigh = low / 100;
			const std::uint32_t leadingZero = high < 10 ? 1u : 0u;
			std::memcpy(out, DigitChars.data() + ((high * 2) | leadingZero), 2);
			std::memcpy(out + 2 - leadingZero, &DigitPairs[lowHigh], 2);
			std::memcpy(out + 4 - leadingZero, &DigitPairs[low - lowHigh * 100], 2);
			return out + 6 - leadingZero;
		}

		BITSERIALIZER_FORCEINLINE char* Write8(char* out, std::uint32_t value) noexcept
		{
			const std::uint32_t high = value / 10000;
			const std::uint32_t low = value - high * 10000;
			const std::uint32_t highHigh = high / 100;
			const std::uint32_t lowHigh = low / 100;
			const std::uint32_t leadingZero = highHigh < 10 ? 1u : 0u;
			std::memcpy(out, DigitChars.data() + ((highHigh * 2) | leadingZero), 2);
			std::memcpy(out + 2 - leadingZero, &DigitPairs[high - highHigh * 100], 2);
			std::memcpy(out + 4 - leadingZero, &DigitPairs[lowHigh], 2);
			std::memcpy(out + 6 - leadingZero, &DigitPairs[low - lowHigh * 100], 2);
			return out + 8 - leadingZero;
		}

		BITSERIALIZER_FORCEINLINE char* Write10(char* out, std::uint32_t value) noexcept
		{
			const std::uint32_t high = value / 10000;
			const std::uint32_t low = value - high * 10000;
			const std::uint32_t top = high / 10000;
			const std::uint32_t highLow = high - top * 10000;
			const std::uint32_t highLowHigh = highLow / 100;
			const std::uint32_t lowHigh = low / 100;
			const std::uint32_t leadingZero = top < 10 ? 1u : 0u;
			std::memcpy(out, DigitChars.data() + ((top * 2) | leadingZero), 2);
			std::memcpy(out + 2 - leadingZero, &DigitPairs[highLowHigh], 2);
			std::memcpy(out + 4 - leadingZero, &DigitPairs[highLow - highLowHigh * 100], 2);
			std::memcpy(out + 6 - leadingZero, &DigitPairs[lowHigh], 2);
			std::memcpy(out + 8 - leadingZero, &DigitPairs[low - lowHigh * 100], 2);
			return out + 10 - leadingZero;
		}

		BITSERIALIZER_FORCEINLINE char* WriteU32(char* out, std::uint32_t value) noexcept
		{
			if (value < 100) {
				return Write2(out, value);
			}
			if (value < 10000) {
				return Write4(out, value);
			}
			if (value < 1000000) {
				return Write6(out, value);
			}
			if (value < 100000000) {
				return Write8(out, value);
			}
			return Write10(out, value);
		}

		BITSERIALIZER_FORCEINLINE char* WriteUnsigned(char* out, std::uint64_t value) noexcept
		{
			if (value <= 0xFFFFFFFFULL)
			{
				return WriteU32(out, static_cast<std::uint32_t>(value));
			}

			const std::uint64_t high = value / 100000000ULL;
			const auto low = static_cast<std::uint32_t>(value - high * 100000000ULL);
			if (high <= 0xFFFFFFFFULL)
			{
				out = WriteU32(out, static_cast<std::uint32_t>(high));
			}
			else
			{
				const std::uint64_t top = high / 100000000ULL;
				const auto middle = static_cast<std::uint32_t>(high - top * 100000000ULL);
				out = WriteU32(out, static_cast<std::uint32_t>(top));
				out = Write8Fixed(out, middle);
			}
			return Write8Fixed(out, low);
		}

		/// Powers of ten used to check whether a value fits into a given number of digits.
		inline constexpr std::uint64_t Pow10[] = {
			1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL,
			100000000ULL, 1000000000ULL, 10000000000ULL, 100000000000ULL, 1000000000000ULL,
			10000000000000ULL, 100000000000000ULL, 1000000000000000ULL, 10000000000000000ULL,
			100000000000000000ULL, 1000000000000000000ULL, 10000000000000000000ULL
		};
	}

	/**
	 * @brief Returns `true` when `value` (with its sign and one store-slack character) fits into `spare` characters.
	 *
	 * Unlike a full digit count, this only performs one comparison against the largest power of ten
	 * that fits into the available capacity.
	 */
	template <typename T>
	inline bool FitsInCapacity(T value, std::size_t spare) noexcept
	{
		static_assert(std::is_integral_v<T>, "BitSerializer. FitsInCapacity() expects an integral type");
		static_assert(!std::is_same_v<T, bool>, "BitSerializer. FitsInCapacity() does not accept bool");
		static_assert(sizeof(T) <= sizeof(std::uint64_t), "BitSerializer. FitsInCapacity() supports integers up to 64 bits");

		if (spare == 0) {
			return false;
		}

		std::size_t signChars = 0;
		std::uint64_t magnitude;
		if constexpr (std::is_signed_v<T>)
		{
			const auto signedValue = static_cast<std::int64_t>(value);
			if (signedValue < 0)
			{
				magnitude = static_cast<std::uint64_t>(0) - static_cast<std::uint64_t>(signedValue);
				signChars = 1;
			}
			else
			{
				magnitude = static_cast<std::uint64_t>(signedValue);
			}
		}
		else
		{
			magnitude = static_cast<std::uint64_t>(value);
		}

		// One character is reserved for the safe tail used by the two-byte store helpers.
		if (spare <= signChars + 1) {
			return false;
		}
		const std::size_t maxDigits = spare - signChars - 1;
		return maxDigits >= 20 || magnitude < IntegerFormatDetail::Pow10[maxDigits];
	}

	/**
	 * @brief Formats an integral value as a base-10 string into `out`.
	 *
	 * The output is identical to `std::to_chars(out, ..., value)` for base 10: no leading
	 * zeros, a minus sign for negative values and no null terminator.
	 *
	 * @param[out] out Destination buffer; must have at least `MaxIntegerLength` bytes.
	 * @param[in]  value Integral value (up to 64 bits, not `bool`).
	 * @return Pointer past the last written character.
	 */
	template <typename T>
	BITSERIALIZER_FORCEINLINE char* FormatInteger(char* out, T value) noexcept
	{
		static_assert(std::is_integral_v<T>, "BitSerializer. FormatInteger() expects an integral type");
		static_assert(!std::is_same_v<T, bool>, "BitSerializer. FormatInteger() does not accept bool");
		static_assert(sizeof(T) <= sizeof(std::uint64_t), "BitSerializer. FormatInteger() supports integers up to 64 bits");

		if constexpr (std::is_signed_v<T>)
		{
			const auto signedValue = static_cast<std::int64_t>(value);
			if (signedValue < 0)
			{
				*out++ = '-';
				const auto magnitude = static_cast<std::uint64_t>(0) - static_cast<std::uint64_t>(signedValue);
				return IntegerFormatDetail::WriteUnsigned(out, magnitude);
			}
			return IntegerFormatDetail::WriteUnsigned(out, static_cast<std::uint64_t>(signedValue));
		}
		else
		{
			return IntegerFormatDetail::WriteUnsigned(out, static_cast<std::uint64_t>(value));
		}
	}

	/**
	 * @brief Formats an integral value as base-10 characters of any character type.
	 *
	 * Used for `wchar_t`, `char16_t`, `char32_t` and other non-`char` character types to avoid
	 * an intermediate UTF-8 buffer and a separate transcoding step.
	 */
	template <typename CharT, typename T, std::enable_if_t<!std::is_same_v<CharT, char>, int> = 0>
	BITSERIALIZER_FORCEINLINE CharT* FormatInteger(CharT* out, T value) noexcept
	{
		static_assert(std::is_integral_v<T>, "BitSerializer. FormatInteger() expects an integral type");
		static_assert(!std::is_same_v<T, bool>, "BitSerializer. FormatInteger() does not accept bool");
		static_assert(sizeof(T) <= sizeof(std::uint64_t), "BitSerializer. FormatInteger() supports integers up to 64 bits");

		CharT temp[MaxIntegerLength];
		CharT* p = temp + MaxIntegerLength;
		std::uint64_t magnitude;
		bool negative = false;
		if constexpr (std::is_signed_v<T>)
		{
			const auto signedValue = static_cast<std::int64_t>(value);
			if (signedValue < 0)
			{
				magnitude = static_cast<std::uint64_t>(0) - static_cast<std::uint64_t>(signedValue);
				negative = true;
			}
			else
			{
				magnitude = static_cast<std::uint64_t>(signedValue);
			}
		}
		else
		{
			magnitude = static_cast<std::uint64_t>(value);
		}

		do
		{
			*--p = static_cast<CharT>('0' + magnitude % 10);
			magnitude /= 10;
		} while (magnitude != 0);

		if (negative) {
			*--p = static_cast<CharT>('-');
		}

		const std::size_t length = static_cast<std::size_t>(temp + MaxIntegerLength - p);
		for (std::size_t i = 0; i < length; ++i) {
			out[i] = p[i];
		}
		return out + length;
	}

	/**
	 * @brief Appends an integral value to a string, formatting directly into its storage when possible.
	 *
	 * If the string has enough spare capacity, the value is written into the tail and the string is
	 * shrunk to the actual length (no extra copy). The fit check uses the largest power of ten that
	 * fits into the current capacity, so it does not need an exact digit count. Otherwise the value
	 * is formatted into a type-sized stack buffer and appended, which keeps short numbers inside the
	 * string's small-object buffer and lets long ones request the exact capacity.
	 */
	template <typename T, typename CharT, typename TTraits, typename TAllocator>
	void FormatInteger(std::basic_string<CharT, TTraits, TAllocator>& out, T value)
	{
		constexpr std::size_t maxLength = MaxIntegerLengthFor<T>();
		const std::size_t oldSize = out.size();
		const std::size_t spare = out.capacity() - oldSize;
		if (spare >= maxLength)
		{
			out.resize(oldSize + maxLength);
			CharT* const end = FormatInteger(out.data() + oldSize, value);
			out.resize(static_cast<std::size_t>(end - out.data()));
		}
		else if (FitsInCapacity(value, spare))
		{
			out.resize(oldSize + spare);
			CharT* const end = FormatInteger(out.data() + oldSize, value);
			out.resize(static_cast<std::size_t>(end - out.data()));
		}
		else
		{
			// Keep short numbers inside the string's small-object buffer (and let `append`
			// request the exact capacity for long ones) instead of forcing a worst-case resize.
			CharT buf[maxLength];
			CharT* const end = FormatInteger(buf, value);
			out.append(buf, static_cast<std::size_t>(end - buf));
		}
	}
}
