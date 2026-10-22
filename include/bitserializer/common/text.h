/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstddef>
#include <string_view>
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
