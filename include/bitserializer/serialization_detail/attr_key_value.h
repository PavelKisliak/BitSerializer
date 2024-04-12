/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
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

/// <summary>
/// The helper function for making the wrapper with attribute value.
/// </summary>
/// <param name="attributeKey">The attribute key.</param>
/// <param name="value">The value.</param>
/// <param name="validators">Validators</param>
/// <returns>The AttributeValue object</returns>
template <class TAttrKey, class TValue, class... TValidators>
[[deprecated("Use directly AttributeValue() constructor (allowed via C++ 17 template argument deduction)")]]
constexpr AttributeValue<TAttrKey, TValue, TValidators...> MakeAttributeValue(TAttrKey&& attributeKey, TValue&& value, const TValidators&... validators)
{
	return AttributeValue<TAttrKey, TValue, TValidators...>(std::forward<TAttrKey>(attributeKey), std::forward<TValue>(value), validators...);
}

//------------------------------------------------------------------------------

/// <summary>
/// The wrapper for attribute values with automatically adaptation to the char type which is supported by archive (of course with reducing performance).
/// </summary>
template<class TAttrKey, class TValue, class... Validators>
class AutoAttributeValue : public AttributeValue<TAttrKey, TValue, Validators...>
{
public:
	[[deprecated("Use regular KeyValue() for all cases")]]
	AutoAttributeValue(TAttrKey&& key, TValue& value, Validators&&... validators)
		: AttributeValue<TAttrKey, TValue, Validators...>(std::forward<TAttrKey>(key), std::forward<TValue>(value), std::forward<Validators>(validators)...)
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

template<class TAttrKey, class TValue, class... Validators>
AutoAttributeValue(TAttrKey&&, TValue&, Validators&&...) -> AutoAttributeValue<TAttrKey, TValue&, Validators...>;

template<class TAttrKey, class TValue, class... Validators>
AutoAttributeValue(TAttrKey&&, TValue&&, Validators&&...) -> AutoAttributeValue<TAttrKey, TValue, Validators...>;

/// <summary>
/// The helper function for making the auto attributes wrapper.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <param name="validators">Validators</param>
/// <returns>The AutoAttributeValue object</returns>
template <class TAttrKey, class TValue, class... Validators>
[[deprecated("Use directly AutoAttributeValue() constructor (allowed via C++ 17 template argument deduction)")]]
constexpr AutoAttributeValue<TAttrKey, TValue, Validators...> MakeAutoAttributeValue(TAttrKey&& key, TValue&& value, const Validators&... validators) noexcept {
	return AutoAttributeValue<TAttrKey, TValue, Validators...>(std::forward<TAttrKey>(key), std::forward<TValue>(value), validators...);
}

}
