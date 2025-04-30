/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>
#include <utility>

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

	constexpr KeyValue(TKey&& key, TValue&& value, Validators&&... validators)
		: mKey(std::forward<TKey>(key)), mValue(std::forward<TValue>(value)), mValidators(std::forward<Validators>(validators)...)
	{}

	constexpr KeyValue(TKey&& key, TValue&& value, std::tuple<Validators...>&& validators)
		: mKey(std::forward<TKey>(key)), mValue(std::forward<TValue>(value)), mValidators(std::move(validators))
	{}

	[[nodiscard]] constexpr const TKey& GetKey() const noexcept	{ return mKey; }
	[[nodiscard]] constexpr TValue GetValue() const noexcept(std::is_reference_v<TValue> || std::is_nothrow_copy_constructible_v<TValue>)
	{
		return mValue;
	}

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

}
