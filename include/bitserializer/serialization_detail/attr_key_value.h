/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/serialization_detail/key_value.h"

namespace BitSerializer {

/// <summary>
/// The wrapper for attribute values (specific for XML format).
/// </summary>
template<class TAttrKey, class TValue, class... TValidators>
class AttributeValue : public KeyValue<TAttrKey, TValue, TValidators...>
{
public:
	AttributeValue(TAttrKey&& attributeKey, TValue&& value, TValidators&&... validators)
		: KeyValue<TAttrKey, TValue, TValidators...>(std::forward<TAttrKey>(attributeKey), std::forward<TValue>(value), std::forward<TValidators>(validators)...)
	{}

	AttributeValue(TAttrKey&& attributeKey, TValue&& value, std::tuple<TValidators...>&& validators)
		: KeyValue<TAttrKey, TValue, TValidators...>(std::forward<TAttrKey>(attributeKey), std::forward<TValue>(value), std::move(validators))
	{}
};

template<class TAttrKey, class TValue, class... Validators>
AttributeValue(TAttrKey&&, TValue&, Validators&&...) -> AttributeValue<TAttrKey, TValue&, Validators...>;

template<class TAttrKey, class TValue, class... Validators>
AttributeValue(TAttrKey&&, TValue&&, Validators&&...) -> AttributeValue<TAttrKey, TValue, Validators...>;

}
