/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstddef>

namespace BitSerializer::Text
{
	/**
	 * @brief Checks if a character is ASCII whitespace.
	 *
	 * ASCII whitespace includes: space, tab, newline, vertical tab, form feed, carriage return.
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
			c == static_cast<TChar>('\v') ||
			c == static_cast<TChar>('\f') ||
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

} // namespace BitSerializer::Text
