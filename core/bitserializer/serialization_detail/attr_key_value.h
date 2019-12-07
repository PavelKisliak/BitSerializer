/*******************************************************************************
* Copyright (C) 2019 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/serialization_detail/key_value.h"

namespace BitSerializer {

/// <summary>
/// Implementation of helper class for wrap an attribute value.
/// </summary>
template<class TAttrKey, class TValue, class... TValidators>
class AttributeValue : public KeyValue<TAttrKey, TValue, TValidators...>
{
public:
	AttributeValue(TAttrKey&& attributeKey, TValue& value, const TValidators&... validators)
		: KeyValue<TAttrKey, TValue, TValidators...>(std::forward<TAttrKey>(attributeKey), value, validators...)
	{}

	AttributeValue(TAttrKey&& attributeKey, TValue& value, std::tuple<TValidators...>&& validators)
		: KeyValue<TAttrKey, TValue, TValidators...>(std::forward<TAttrKey>(attributeKey))
	{}
};

/// <summary>
/// The helper function for making an attribute value.
/// </summary>
/// <param name="attributeKey">The attribute key.</param>
/// <param name="value">The value.</param>
/// <param name="validators">Validators</param>
template <class TAttrKey, class TValue, class... TValidators>
constexpr AttributeValue<TAttrKey, TValue, TValidators...> MakeAttributeValue(TAttrKey&& attributeKey, TValue& value, const TValidators&... validators)
{
	return AttributeValue<TAttrKey, TValue, TValidators...>(std::forward<TAttrKey>(attributeKey), value, validators...);
}

}	// namespace BitSerializer
