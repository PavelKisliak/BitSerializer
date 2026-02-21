/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>
#include <utility>

namespace BitSerializer
{
	/**
	 * @brief Wrapper for key-value serialization with optional validators/refiners.
	 *
	 * Primary wrapper for serializing named fields. Use by default for all formats.
	 *
	 * @tparam TKey      Type of the key.
	 * @tparam TValue    Type of the value.
	 * @tparam TArgs     Optional extra types (validators, refiners).
	 */
	template<class TKey, class TValue, class... TArgs>
	class KeyValue
	{
	protected:
		TKey mKey;
		TValue mValue;
		std::tuple<TArgs...> mArgs;

	public:
		using value_type = TValue;
		using key_type = TKey;

		constexpr KeyValue(TKey&& key, TValue&& value, TArgs&&... args)
			: mKey(std::forward<TKey>(key))
			, mValue(std::forward<TValue>(value))
			, mArgs(std::forward<TArgs>(args)...)
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
			}, mArgs);
		}
	};

	// Deduction guide for constructing `KeyValue` class when value passed as lvalue
	template<class TKey, class TValue, class... TArgs>
	KeyValue(TKey&&, TValue&, TArgs&&...) -> KeyValue<TKey, TValue&, TArgs...>;

	// Deduction guide for constructing `KeyValue` class when value passed as rvalue
	template<class TKey, class TValue, class... TArgs>
	KeyValue(TKey&&, TValue&&, TArgs&&...) -> KeyValue<TKey, TValue, TArgs...>;


	/**
	 * @brief Wrapper for XML attribute serialization.
	 *
	 * Forces serialization as an XML attribute. Causes compile-time error if used with non-XML archives (JSON, YAML, CSV, MsgPack).
	 *
	 * @see PropertyValue For multi-format safe alternative
	 *
	 * @tparam TAttrKey  Type of the attribute key.
	 * @tparam TValue    Type of the attribute value.
	 * @tparam TArgs     Optional extra types (validators, refiners).
	 */
	template<class TAttrKey, class TValue, class... TArgs>
	class AttributeValue : public KeyValue<TAttrKey, TValue, TArgs...>
	{
	public:
		constexpr AttributeValue(TAttrKey&& key, TValue&& value, TArgs&&... args)
			: KeyValue<TAttrKey, TValue, TArgs...>(std::forward<TAttrKey>(key), std::forward<TValue>(value), std::forward<TArgs>(args)...)
		{ }
	};

	// Deduction guide for constructing `AttributeValue` class when value passed as lvalue
	template<class TAttrKey, class TValue, class... TArgs>
	AttributeValue(TAttrKey&&, TValue&, TArgs&&...) -> AttributeValue<TAttrKey, TValue&, TArgs...>;

	// Deduction guide for constructing `AttributeValue` class when value passed as rvalue
	template<class TAttrKey, class TValue, class... TArgs>
	AttributeValue(TAttrKey&&, TValue&&, TArgs&&...) -> AttributeValue<TAttrKey, TValue, TArgs...>;


	/**
	 * @brief Smart wrapper for property serialization with automatic format adaptation.
	 *
	 * Adapts serialization behavior based on archive type and value convertibility:
	 * - XML: Serializes as attribute if convertible to string, otherwise as element
	 * - Other formats: Always serializes as key-value pair
	 *
	 * Ideal for code generation (e.g., OpenAPI) and multi-format APIs.
	 *
	 * @see KeyValue For default element/key serialization
	 * @see AttributeValue For strict XML attribute serialization
	 *
	 * @tparam TKey      Type of the property key.
	 * @tparam TValue    Type of the property value.
	 * @tparam TArgs     Optional extra types (validators, refiners).
	 */
	template<class TKey, class TValue, class... TArgs>
	class PropertyValue : public AttributeValue<TKey, TValue, TArgs...>
	{
	public:
		constexpr PropertyValue(TKey&& key, TValue&& value, TArgs&&... args)
			: AttributeValue<TKey, TValue, TArgs...>(std::forward<TKey>(key), std::forward<TValue>(value), std::forward<TArgs>(args)...)
		{ }
	};

	// Deduction guide for constructing `PropertyValue` class when value passed as lvalue
	template<class TKey, class TValue, class... TArgs>
	PropertyValue(TKey&&, TValue&, TArgs&&...) -> PropertyValue<TKey, TValue&, TArgs...>;

	// Deduction guide for constructing `PropertyValue` class when value passed as rvalue
	template<class TKey, class TValue, class... TArgs>
	PropertyValue(TKey&&, TValue&&, TArgs&&...) -> PropertyValue<TKey, TValue, TArgs...>;

} // namespace BitSerializer
