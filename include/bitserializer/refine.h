/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <utility>
#include <type_traits>

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
		void operator()(TValue& value, bool isLoaded) const noexcept(std::is_nothrow_assignable_v<TValue, T>)
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

} // namespace BitSerializer::Refine
