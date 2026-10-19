/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <string_view>
#include <utility>
#include <cctype>
#include "bitserializer/common/text.h"
#include "bitserializer/convert.h"
#include "bitserializer/serialization_detail/object_traits.h"

namespace BitSerializer::Refine
{
	/**
	 * @brief Provides a fallback value for missing deserialization data.
	 *
	 * Applies default value ONLY when value was not deserialized.
	 */
	template <class T>
	class Fallback
	{
	public:
		constexpr explicit Fallback() noexcept = default;

		template <typename... TInitArgs, std::enable_if_t<std::is_constructible_v<T, TInitArgs...>, int> = 0>
		constexpr explicit Fallback(TInitArgs&&... initArgs) noexcept(std::is_nothrow_constructible_v<T, TInitArgs...>)
			: mDefaultValue(std::forward<TInitArgs>(initArgs)...)
		{ }

		template <typename TValue>
		void operator()(TValue& value, bool isLoaded) const
		{
			static_assert(std::is_assignable_v<TValue&, T>, "BitSerializer. The specified fallback value cannot be assigned to the target value.");

			if (!isLoaded) {
				value = mDefaultValue;
			}
		}

	private:
		T mDefaultValue;
	};

	// Deduction guide for constructing `Fallback` class
	template<class T>
	Fallback(T&&) -> Fallback<T>;

	namespace Detail
	{
		/**
		 * @brief Applies a character transformation to a mutable string-like value.
		 *
		 * Enumerable types are transformed in-place. Types registered via
		 * `BITSERIALIZER_DECLARE_STRING_TYPE` are copied into a temporary buffer, transformed,
		 * and assigned back (their view is read-only).
		 */
		template <typename TString, typename TFunc>
		void TransformChars(TString& str, TFunc&& func)
		{
			constexpr auto isEnumerable = is_enumerable_v<TString>;
			constexpr auto isStringLike = Convert::Detail::is_string_type_v<TString>;
			static_assert(isEnumerable || isStringLike,
				"BitSerializer. Target string must support iterators or be registered as a string-like type.");

			if constexpr (isEnumerable)
			{
				for (auto& c : str) {
					func(c);
				}
			}
			else
			{
				using namespace Convert::Detail;
				const auto view = Convert::ToStringView(str);
				using char_type = typename decltype(view)::value_type;
				std::basic_string<char_type> buffer(view);
				for (auto& c : buffer) {
					func(c);
				}
				AssignString(str, std::basic_string_view<char_type>(buffer));
			}
		}
	}

	/**
	 * @brief Trims leading and trailing whitespace from string-like types.
	 */
	class TrimWhitespace
	{
	public:
		template <typename TString>
		void operator()(TString& str, bool isLoaded) const
		{
			constexpr auto isEnumerable = is_enumerable_v<TString>;
			constexpr auto isStringLike = Convert::Detail::is_string_type_v<TString>;
			static_assert(isEnumerable || isStringLike,
				"BitSerializer. Target string must support iterators or be registered as a string-like type.");

			if (!isLoaded) {
				return;
			}

			if constexpr (isEnumerable) {
				Text::TrimWhitespace(str);
			}
			else
			{
				using namespace Convert::Detail;
				const auto view = Convert::ToStringView(str);
				using char_type = typename decltype(view)::value_type;
				std::basic_string<char_type> buffer(view);
				Text::TrimWhitespace(buffer);
				AssignString(str, std::basic_string_view<char_type>(buffer));
			}
		}
	};

	/**
	 * @brief Converts string to lowercase (ASCII only, leaves other characters unchanged).
	 */
	class ToLowerCase
	{
	public:
		template <typename TString>
		void operator()(TString& str, bool isLoaded) const noexcept(is_enumerable_v<TString>)
		{
			if (isLoaded)
			{
				Detail::TransformChars(str, [](auto& c)
				{
					using char_type = std::remove_reference_t<decltype(c)>;
					if (c < 128) {
						c = static_cast<char_type>(std::tolower(static_cast<int>(c)));
					}
				});
			}
		}
	};

	/**
	 * @brief Converts string to uppercase (ASCII only, leaves other characters unchanged).
	 */
	class ToUpperCase
	{
	public:
		template <typename TString>
		void operator()(TString& str, bool isLoaded) const noexcept(is_enumerable_v<TString>)
		{
			if (isLoaded)
			{
				Detail::TransformChars(str, [](auto& c)
				{
					using char_type = std::remove_reference_t<decltype(c)>;
					if (c < 128) {
						c = static_cast<char_type>(std::toupper(static_cast<int>(c)));
					}
				});
			}
		}
	};

} // namespace BitSerializer::Refine
