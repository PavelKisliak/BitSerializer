/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/key_value.h"

namespace BitSerializer
{
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
