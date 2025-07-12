/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>
#include <utility>

namespace BitSerializer
{
	/**
	 * @brief A generic wrapper for a key-value pair with optional validators.
	 *
	 * This class is used to associate a key with a value and a set of validation rules.
	 *
	 * @tparam TKey       Type of the key.
	 * @tparam TValue     Type of the value being stored.
	 * @tparam Validators Optional validator types.
	 */
	template<class TKey, class TValue, class... Validators>
	class KeyValue
	{
	protected:
		TKey mKey;
		TValue mValue;
		std::tuple<Validators...> mValidators;

	public:
		using value_type = TValue;
		using key_type = TKey;

		/**
		 * @brief Constructs a KeyValue instance from a key, value, and optional validators.
		 *
		 * @param key        The key associated with the value.
		 * @param value      The value to store.
		 * @param validators Optional validation rules to apply to the value.
		 */
		constexpr KeyValue(TKey&& key, TValue&& value, Validators&&... validators)
			: mKey(std::forward<TKey>(key))
			, mValue(std::forward<TValue>(value))
			, mValidators(std::forward<Validators>(validators)...)
		{
		}

		/**
		 * @brief Retrieves a constant reference to the key.
		 *
		 * @return A const reference to the key.
		 */
		[[nodiscard]] constexpr const TKey& GetKey() const noexcept
		{
			return mKey;
		}

		/**
		 * @brief Retrieves the stored value (usually `TValue` is a reference to the serializing value).
		 *
		 * @return The stored value.
		 */
		[[nodiscard]] constexpr TValue GetValue() const noexcept (std::is_reference_v<TValue> || std::is_nothrow_copy_constructible_v<TValue>)
		{
			return mValue;
		}

		/**
		 * @brief Applies a visitor function to each validator in the argument pack.
		 *
		 * @tparam TVisitor Type of the visitor callable.
		 * @param visitor   Callable object to apply to each validator.
		 */
		template <typename TVisitor>
		void VisitArgs(TVisitor visitor)
		{
			std::apply([&visitor](auto&& ...args) {
				(visitor(args), ...);
			}, mValidators);
		}
	};

	// Deduction guide for constructing `KeyValue` class when value passed as lvalue
	template<class TKey, class TValue, class... Validators>
	KeyValue(TKey&&, TValue&, Validators&&...) -> KeyValue<TKey, TValue&, Validators...>;

	// Deduction guide for constructing `KeyValue` class when value passed as rvalue
	template<class TKey, class TValue, class... Validators>
	KeyValue(TKey&&, TValue&&, Validators&&...) -> KeyValue<TKey, TValue, Validators...>;

} // namespace BitSerializer
