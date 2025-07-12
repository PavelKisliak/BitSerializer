/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/serialization_detail/key_value.h"

namespace BitSerializer
{
	/**
	 * @brief Wrapper for serializing attribute values (specific for XML format).
	 *
	 * @tparam TAttrKey     Type of the attribute key.
	 * @tparam TValue       Type of the attribute value.
	 * @tparam TValidators  Optional validator types.
	 */
	template<class TAttrKey, class TValue, class... TValidators>
	class AttributeValue : public KeyValue<TAttrKey, TValue, TValidators...>
	{
	public:
		/**
		 * @brief Constructs an AttributeValue instance from a key, value, and optional validators.
		 *
		 * @param attributeKey The key identifying the attribute.
		 * @param value        The value to be serialized as an attribute.
		 * @param validators   Optional validation rules to apply to the value.
		 */
		AttributeValue(TAttrKey&& attributeKey, TValue&& value, TValidators&&... validators)
			: KeyValue<TAttrKey, TValue, TValidators...>(std::forward<TAttrKey>(attributeKey), std::forward<TValue>(value), std::forward<TValidators>(validators)...)
		{
		}
	};

	// Deduction guide for constructing `KeyValue` class when value passed as lvalue
	template<class TAttrKey, class TValue, class... Validators>
	AttributeValue(TAttrKey&&, TValue&, Validators&&...) -> AttributeValue<TAttrKey, TValue&, Validators...>;

	// Deduction guide for constructing `KeyValue` class when value passed as rvalue
	template<class TAttrKey, class TValue, class... Validators>
	AttributeValue(TAttrKey&&, TValue&&, Validators&&...) -> AttributeValue<TAttrKey, TValue, Validators...>;

} // namespace BitSerializer
