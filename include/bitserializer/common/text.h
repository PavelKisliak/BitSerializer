/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstddef>
#include <string_view>
#include <type_traits>
#include "bitserializer/common/memory.h"

namespace BitSerializer::Text
{
	/**
	 * @brief Compares two string views for equality.
	 *
	 * Equivalent to `lhs == rhs`, but for the short strings that dominate (keys are usually
	 * 8-32 bytes) it avoids the `std::char_traits`/`std::memcmp` call: the lengths are checked
	 * first, then the bytes are compared word-at-a-time (see `Memory::EqualBytes`).
	 */
	[[nodiscard]] inline bool Equal(std::string_view lhs, std::string_view rhs) noexcept
	{
		if (lhs.size() != rhs.size()) {
			return false;
		}
		const char* l = lhs.data();
		const char* r = rhs.data();
		return Memory::EqualBytes(l, r, lhs.size());
	}

	/**
	 * @brief Compares a string view against a character array whose size is known at compile time.
	 *
	 * The array is expected to be a null-terminated string literal, so `N - 1` characters are
	 * compared. Specialising on `N` lets the compiler unroll the comparison into a fixed sequence
	 * of loads (see `Memory::EqualBytes`).
	 */
	template <std::size_t N>
	[[nodiscard]] inline bool Equal(std::string_view lhs, const char (&rhs)[N]) noexcept
	{
		constexpr std::size_t count = N - 1;
		if (lhs.size() != count) {
			return false;
		}
		const char* l = lhs.data();
		return Memory::EqualBytes<count>(l, rhs);
	}

	/**
	 * @brief Reversed counterpart of `Equal(std::string_view, const char (&)[N])`.
	 */
	template <std::size_t N>
	[[nodiscard]] inline bool Equal(const char (&lhs)[N], std::string_view rhs) noexcept
	{
		constexpr std::size_t count = N - 1;
		if (rhs.size() != count) {
			return false;
		}
		const char* r = rhs.data();
		return Memory::EqualBytes<count>(lhs, r);
	}

	/**
	 * @brief Compares two character arrays whose sizes are known at compile time.
	 *
	 * This overload is also what keeps `Equal("a", "b")` unambiguous: it is an exact match for
	 * both arguments, while the mixed array/view overloads each need one user-defined conversion.
	 *
	 * Unlike the mixed overloads, this one is `constexpr`: both lengths are part of the type and
	 * the comparison is a plain loop, so it can be used in `static_assert`. The hot JSON path
	 * compares a runtime `std::string_view` with an array and uses the word-at-a-time overload
	 * above; this overload is intended for compile-time/known-length literal comparisons.
	 */
	template <std::size_t N, std::size_t M>
	[[nodiscard]] constexpr bool Equal(const char (&lhs)[N], const char (&rhs)[M]) noexcept
	{
		constexpr std::size_t lhsCount = N - 1;
		constexpr std::size_t rhsCount = M - 1;
		if constexpr (lhsCount != rhsCount) {
			return false;
		}
		else {
			for (std::size_t i = 0; i < lhsCount; ++i) {
				if (lhs[i] != rhs[i]) {
					return false;
				}
			}
			return true;
		}
	}

	/**
	 * @brief Checks if a character is ASCII whitespace.
	 *
	 * ASCII whitespace includes: space, tab, newline, carriage return.
	 *
	 * @param c Character to check
	 * @return true if character is whitespace, false otherwise
	 */
	template <typename TChar>
	constexpr bool IsWhitespace(TChar c) noexcept
	{
		return c == static_cast<TChar>(' ') ||
			c == static_cast<TChar>('\t') ||
			c == static_cast<TChar>('\n') ||
			c == static_cast<TChar>('\r');
	}

	/**
	 * @brief Converts an ASCII uppercase letter to lowercase.
	 *
	 * The conversion is locale-independent and branchless; non-ASCII characters are returned unchanged.
	 *
	 * @param c Character to convert.
	 * @return Lowercase ASCII character, or the original character.
	 */
	template <typename TChar>
	constexpr TChar ToAsciiLower(TChar c) noexcept
	{
		using UChar = std::make_unsigned_t<TChar>;
		const auto value = static_cast<UChar>(c);
		return static_cast<TChar>(value >= static_cast<UChar>('A') && value <= static_cast<UChar>('Z')
			? static_cast<UChar>(value + ('a' - 'A'))
			: value);
	}

	/**
	 * @brief Converts an ASCII lowercase letter to uppercase.
	 *
	 * The conversion is locale-independent and branchless; non-ASCII characters are returned unchanged.
	 *
	 * @param c Character to convert.
	 * @return Uppercase ASCII character, or the original character.
	 */
	template <typename TChar>
	constexpr TChar ToAsciiUpper(TChar c) noexcept
	{
		using UChar = std::make_unsigned_t<TChar>;
		const auto value = static_cast<UChar>(c);
		return static_cast<TChar>(value >= static_cast<UChar>('a') && value <= static_cast<UChar>('z')
			? static_cast<UChar>(value - ('a' - 'A'))
			: value);
	}

	namespace Detail
	{
		/**
		 * @brief Compares two character ranges as ASCII strings, ignoring case.
		 */
		template <typename TChar>
		[[nodiscard]] constexpr bool EqualIgnoreCaseBytes(std::basic_string_view<TChar> lhs, std::basic_string_view<TChar> rhs) noexcept
		{
			if (lhs.size() != rhs.size()) {
				return false;
			}
			for (std::size_t i = 0; i < lhs.size(); ++i)
			{
				if (ToAsciiLower(lhs[i]) != ToAsciiLower(rhs[i])) {
					return false;
				}
			}
			return true;
		}
	}

	/**
	 * @brief Compares two ASCII strings case-insensitively.
	 *
	 * Only ASCII letters are case-folded; non-ASCII characters must match exactly.
	 */
	[[nodiscard]] constexpr bool EqualIgnoreCase(std::string_view lhs, std::string_view rhs) noexcept
	{
		return Detail::EqualIgnoreCaseBytes(lhs, rhs);
	}

	/**
	 * @brief Compares two ASCII strings case-insensitively.
	 *
	 * Only ASCII letters are case-folded; non-ASCII characters must match exactly.
	 */
	template <typename TChar>
	[[nodiscard]] constexpr bool EqualIgnoreCase(std::basic_string_view<TChar> lhs, std::basic_string_view<TChar> rhs) noexcept
	{
		return Detail::EqualIgnoreCaseBytes(lhs, rhs);
	}

	/**
	 * @brief Compares an ASCII string view with a character array, ignoring case.
	 */
	template <typename TChar, std::size_t N>
	[[nodiscard]] constexpr bool EqualIgnoreCase(std::basic_string_view<TChar> lhs, const TChar (&rhs)[N]) noexcept
	{
		return Detail::EqualIgnoreCaseBytes(lhs, std::basic_string_view<TChar>(rhs, N - 1));
	}

	/**
	 * @brief Compares a character array with an ASCII string view, ignoring case.
	 */
	template <typename TChar, std::size_t N>
	[[nodiscard]] constexpr bool EqualIgnoreCase(const TChar (&lhs)[N], std::basic_string_view<TChar> rhs) noexcept
	{
		return Detail::EqualIgnoreCaseBytes(std::basic_string_view<TChar>(lhs, N - 1), rhs);
	}

	/**
	 * @brief Compares two character arrays case-insensitively at compile time.
	 */
	template <typename TChar, std::size_t N, std::size_t M>
	[[nodiscard]] constexpr bool EqualIgnoreCase(const TChar (&lhs)[N], const TChar (&rhs)[M]) noexcept
	{
		return Detail::EqualIgnoreCaseBytes(
			std::basic_string_view<TChar>(lhs, N - 1),
			std::basic_string_view<TChar>(rhs, M - 1));
	}

	/**
	 * @brief Trims leading/trailing whitespace from mutable strings.
	 *
	 * @tparam TString Must support random-access iterators and `resize()`
	 * @param str String to trim in-place
	 */
	template <typename TString>
	void TrimWhitespace(TString& str)
	{
		auto first = str.begin();
		auto last = str.end();
		while (first != last && IsWhitespace(*first)) {
			++first;
		}

		size_t newLength = 0;
		if (first != last)
		{
			auto lastNonWs = last - 1;
			while (lastNonWs >= first && IsWhitespace(*lastNonWs)) {
				--lastNonWs;
			}
			newLength = static_cast<size_t>(lastNonWs - first + 1);
		}

		if (first != str.begin() && newLength > 0)
		{
			auto dest = str.begin();
			for (auto src = first; src != first + newLength; ++src, ++dest) {
				*dest = *src;
			}
		}

		str.resize(newLength);
	}

	/**
	 * @brief Trims leading/trailing whitespace from string views.
	 *
	 * @tparam TChar Character type
	 * @tparam TTraits String traits
	 * @param str String view to trim
	 */
	template <typename TChar, typename TTraits = std::char_traits<TChar>>
	void TrimWhitespace(std::basic_string_view<TChar, TTraits>& str)
	{
		size_t start = 0;
		while (start < str.size() && IsWhitespace(str[start])) {
			++start;
		}

		if (start == str.size())
		{
			str = std::basic_string_view<TChar, TTraits>();
		}
		else
		{
			size_t end = str.size() - 1;
			while (end > start && IsWhitespace(str[end])) {
				--end;
			}
			str = str.substr(start, (end - start) + 1);
		}
	}

} // namespace BitSerializer::Text
