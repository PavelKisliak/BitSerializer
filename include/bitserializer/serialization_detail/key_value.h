/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>
#include <utility>
#include "serialization_context.h"

namespace BitSerializer {

/// <summary>
/// The wrapper for key, value and set of validators.
/// </summary>
template<class TKey, class TValue, class... Validators>
struct KeyValue
{
protected:
	TKey mKey;
	TValue mValue;
	std::tuple<Validators...> mValidators;

public:
	using value_type = TValue;
	using key_type = TKey;

	KeyValue(TKey&& key, TValue&& value, const Validators&... validators)
		: mKey(key)
		, mValue(value)
		, mValidators(validators...)
	{}

	KeyValue(TKey&& key, TValue&& value, std::tuple<Validators...>&& validators)
		: mKey(key)
		, mValue(value)
		, mValidators(std::move(validators))
	{}

	[[nodiscard]] const TKey& GetKey() const noexcept	{ return mKey; }
	[[nodiscard]] TValue GetValue() const noexcept		{ return mValue; }

	/// <summary>
	/// Applies the passed visitor to all extra arguments (which currently can be only validators).
	/// </summary>
	template <typename TVisitor>
	void VisitArgs(TVisitor visitor)
	{
		std::apply([&visitor](auto&& ...args) {
			(visitor(args), ...);
		}, mValidators);
	}
};

template<class TKey, class TValue, class... Validators>
KeyValue(TKey&&, TValue&, Validators&&...) -> KeyValue<TKey, TValue&, Validators...>;

template<class TKey, class TValue, class... Validators>
KeyValue(TKey&&, TValue&&, Validators&&...) -> KeyValue<TKey, TValue, Validators...>;

/// <summary>
/// The helper function for making the wrapper for key, value and set of validators.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <param name="validators">Validators</param>
/// <returns>The KeyValue object</returns>
template <class TKey, class TValue, class... Validators>
[[deprecated("Use directly KeyValue() constructor (allowed via C++ 17 template argument deduction)")]]
constexpr KeyValue<TKey, TValue, Validators...> MakeKeyValue(TKey&& key, TValue&& value, const Validators&... validators) {
	return KeyValue<TKey, TValue, Validators...>(std::forward<TKey>(key), std::forward<TValue>(value), validators...);
}

//------------------------------------------------------------------------------

/// <summary>
/// The wrapper for key and value with automatically adaptation to char type which is supported by archive (of course with reducing performance).
/// </summary>
template<class TKey, class TValue, class... Validators>
class AutoKeyValue : public KeyValue<TKey, TValue, Validators...>
{
public:
	explicit AutoKeyValue(TKey&& key, TValue&& value, const Validators&... validators)
		: KeyValue<TKey, TValue, Validators...>(std::forward<TKey>(key), std::forward<TValue>(value), validators...)
	{}

	/// <summary>
	/// Adapts the key to target archive and move to base KeyValue type.
	/// </summary>
	template<class TArchiveKey>
	KeyValue<TArchiveKey, TValue, Validators...> AdaptAndMoveToBaseKeyValue()
	{
		auto archiveCompatibleKey = Convert::To<TArchiveKey>(this->GetKey());
		return BitSerializer::KeyValue<TArchiveKey, TValue, Validators...>(
			std::move(archiveCompatibleKey), this->GetValue(), std::move(this->mValidators));
	}
};

template<class TKey, class TValue, class... Validators>
AutoKeyValue(TKey&&, TValue&, Validators&&...) -> AutoKeyValue<TKey, TValue&, Validators...>;

template<class TKey, class TValue, class... Validators>
AutoKeyValue(TKey&&, TValue&&, Validators&&...) -> AutoKeyValue<TKey, TValue, Validators...>;

/// <summary>
/// The helper function for making the auto keys wrapper.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <param name="validators">Validators</param>
/// <returns>The AutoKeyValue object</returns>
template <class TKey, class TValue, class... Validators>
[[deprecated("Use directly AutoKeyValue() constructor (allowed via C++ 17 template argument deduction)")]]
constexpr AutoKeyValue<TKey, TValue, Validators...> MakeAutoKeyValue(TKey&& key, TValue&& value, const Validators&... validators) noexcept {
	return AutoKeyValue<TKey, TValue, Validators...>(std::forward<TKey>(key), std::forward<TValue>(value), validators...);
}

}
