/*******************************************************************************
* Copyright (C) 2019 by Pavel Kisliak                                          *
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
	AttributeValue(TAttrKey&& attributeKey, TValue& value, const TValidators&... validators)
		: KeyValue<TAttrKey, TValue, TValidators...>(std::forward<TAttrKey>(attributeKey), value, validators...)
	{}

	AttributeValue(TAttrKey&& attributeKey, TValue& value, std::tuple<TValidators...>&& validators)
		: KeyValue<TAttrKey, TValue, TValidators...>(std::forward<TAttrKey>(attributeKey), value, std::move(validators))
	{}
};

/// <summary>
/// The helper function for making the wrapper with attribute value.
/// </summary>
/// <param name="attributeKey">The attribute key.</param>
/// <param name="value">The value.</param>
/// <param name="validators">Validators</param>
/// <returns>The AttributeValue object</returns>
template <class TAttrKey, class TValue, class... TValidators>
constexpr AttributeValue<TAttrKey, TValue, TValidators...> MakeAttributeValue(TAttrKey&& attributeKey, TValue& value, const TValidators&... validators)
{
	return AttributeValue<TAttrKey, TValue, TValidators...>(std::forward<TAttrKey>(attributeKey), value, validators...);
}

//------------------------------------------------------------------------------

/// <summary>
/// The wrapper for attribute values with automatically adaptation to the char type which is supported by archive (of course with reducing performance).
/// </summary>
template<class TAttrKey, class TValue, class... Validators>
class AutoAttributeValue : public AttributeValue<TAttrKey, TValue, Validators...>
{
public:
	AutoAttributeValue(TAttrKey&& key, TValue& value, const Validators&... validators)
		: AttributeValue<TAttrKey, TValue, Validators...>(key, value, validators...)
	{}

	/// <summary>
	/// Adapts the attribute to target archive and move to base AttributeValue type.
	/// </summary>
	template<class TArchiveKey>
	AttributeValue<TArchiveKey, TValue, Validators...> AdaptAndMoveToBaseAttributeValue()
	{
		auto archiveCompatibleKey = Convert::To<TArchiveKey>(this->GetKey());
		return BitSerializer::AttributeValue<TArchiveKey, TValue, Validators...>(
			std::move(archiveCompatibleKey), this->GetValue(), std::move(this->mValidators));
	}
};

/// <summary>
/// The helper function for making the auto attributes wrapper.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <param name="validators">Validators</param>
/// <returns>The AutoAttributeValue object</returns>
template <class TAttrKey, class TValue, class... Validators>
constexpr AutoAttributeValue<TAttrKey, TValue, Validators...> MakeAutoAttributeValue(TAttrKey&& key, TValue& value, const Validators&... validators) noexcept {
	return AutoAttributeValue<TAttrKey, TValue, Validators...>(std::forward<TAttrKey>(key), value, validators...);
}

}	// namespace BitSerializer
