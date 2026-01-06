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
	 * @brief A generic wrapper for a key-value pair with optional extra parameters.
	 *
	 * This class is used to associate a key with a value and a set of extra parameters.
	 *
	 * @tparam TKey      Type of the key.
	 * @tparam TValue    Type of the value being stored.
	 * @tparam TArgs     Optional extra types.
	 */
	template<class TKey, class TValue, class... TArgs>
	class KeyValue
	{
	protected:
		TKey mKey;
		TValue mValue;
		std::tuple<TArgs...> mTArgs;

	public:
		using value_type = TValue;
		using key_type = TKey;

		/**
		 * @brief Constructs a KeyValue instance from a key, value, and optional extra parameters.
		 *
		 * @param key    The key associated with the value.
		 * @param value  The value to store.
		 * @param args   Optional extra parameters to apply to the value.
		 */
		constexpr KeyValue(TKey&& key, TValue&& value, TArgs&&... args)
			: mKey(std::forward<TKey>(key))
			, mValue(std::forward<TValue>(value))
			, mTArgs(std::forward<TArgs>(args)...)
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
		 * @brief Applies a visitor function to each extra parameter.
		 *
		 * @tparam TVisitor Type of the visitor callable.
		 * @param visitor   Callable object to apply to each extra parameter.
		 */
		template <typename TVisitor>
		void VisitArgs(TVisitor visitor)
		{
			std::apply([&visitor](auto&& ...args) {
				(visitor(args), ...);
			}, mTArgs);
		}
	};

	// Deduction guide for constructing `KeyValue` class when value passed as lvalue
	template<class TKey, class TValue, class... TArgs>
	KeyValue(TKey&&, TValue&, TArgs&&...) -> KeyValue<TKey, TValue&, TArgs...>;

	// Deduction guide for constructing `KeyValue` class when value passed as rvalue
	template<class TKey, class TValue, class... TArgs>
	KeyValue(TKey&&, TValue&&, TArgs&&...) -> KeyValue<TKey, TValue, TArgs...>;


	/**
	 * @brief Wrapper for serializing attribute values (specific for XML format).
	 *
	 * @tparam TAttrKey     Type of the attribute key.
	 * @tparam TValue       Type of the attribute value.
	 * @tparam TArgs        Types of extra parameters.
	 */
	template<class TAttrKey, class TValue, class... TArgs>
	class AttributeValue : public KeyValue<TAttrKey, TValue, TArgs...>
	{
	public:
		/**
		 * @brief Constructs an AttributeValue instance from a key, value, and optional validators.
		 *
		 * @param attributeKey The key identifying the attribute.
		 * @param value        The value to be serialized as an attribute.
		 * @param args         Optional extra parameters to apply to the value.
		 */
		AttributeValue(TAttrKey&& attributeKey, TValue&& value, TArgs&&... args)
			: KeyValue<TAttrKey, TValue, TArgs...>(std::forward<TAttrKey>(attributeKey), std::forward<TValue>(value), std::forward<TArgs>(args)...)
		{
		}
	};

	// Deduction guide for constructing `AttributeValue` class when value passed as lvalue
	template<class TAttrKey, class TValue, class... TArgs>
	AttributeValue(TAttrKey&&, TValue&, TArgs&&...) -> AttributeValue<TAttrKey, TValue&, TArgs...>;

	// Deduction guide for constructing `AttributeValue` class when value passed as rvalue
	template<class TAttrKey, class TValue, class... TArgs>
	AttributeValue(TAttrKey&&, TValue&&, TArgs&&...) -> AttributeValue<TAttrKey, TValue, TArgs...>;

} // namespace BitSerializer
