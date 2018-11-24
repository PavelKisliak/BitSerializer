/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>
#include <utility>
#include <optional>
#include "serialization_context.h"

namespace BitSerializer {

/// <summary>
/// Implementation of helper class for wrap a serialization value, key and validators.
/// </summary>
template<class TKey, class TValue, class... Validators>
struct KeyValue
{
protected:
	TKey mKey;
	TValue* const mValue;
	std::tuple<Validators...> mValidators;

public:
	explicit KeyValue(TKey&& key, TValue& value, const Validators&... validators)
		: mKey(key)
		, mValue(&value)
		, mValidators(validators...)
	{}

	KeyValue(TKey&& key, TValue& value, std::tuple<Validators...>&& validators)
		: mKey(key)
		, mValue(&value)
		, mValidators(validators)
	{}

	inline const TKey& GetKey() const noexcept	{ return mKey; }
	inline TValue& GetValue() const noexcept	{ return *mValue; }

	/// <summary>
	/// Validates deserialized value.
	/// </summary>
	/// <param name="isLoaded">if set to <c>true</c> [is loaded].</param>
	/// <returns></returns>
	std::optional<ValidationErrors> ValidateValue(const bool isLoaded) const
	{
		if constexpr (sizeof...(Validators) == 0)
			return std::nullopt;

		std::optional<ValidationErrors> validationResult;
		ValidateValueImpl(isLoaded, validationResult);
		return validationResult;
	}

private:
	template<std::size_t I = 0>
	void ValidateValueImpl(bool isLoaded, std::optional<ValidationErrors>& validationResult) const
	{
		if constexpr (I == sizeof...(Validators))
			return;
		else
		{
			decltype(auto) validator = std::get<I>(mValidators);
			auto result = validator(GetValue(), isLoaded);
			if (result.has_value())
			{
				if (!validationResult.has_value())
					validationResult = ValidationErrors();
				validationResult->emplace_back(std::move(*result));
			}
			ValidateValueImpl<I + 1>(isLoaded, validationResult);
		}
	}
};

/// <summary>
/// The helper function for making a wrapper with a serialization value, key and validators.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <param name="validators">Validators</param>
/// <returns></returns>
template <class TKey, class TValue, class... Validators>
constexpr KeyValue<TKey, TValue, Validators...> MakeKeyValue(TKey&& key, TValue& value, const Validators&... validators) noexcept {
	return KeyValue<TKey, TValue, Validators...>(std::forward<TKey>(key), value, validators...);
}

//------------------------------------------------------------------------------

/// <summary>
/// This key value wrapper allows to automatically adaptation key to type which is supported by archive (of course with reducing performance).
/// </summary>
template<class TKey, class TValue, class... Validators>
class AutoKeyValue : public KeyValue<TKey, TValue, Validators...>
{
public:
	explicit AutoKeyValue(TKey&& key, TValue& value, const Validators&... validators)
		: KeyValue<TKey, TValue, Validators...>(key, value, validators...)
	{}

	/// <summary>
	/// Adapts the key to target archive and move to base KeyValue type.
	/// </summary>
	/// <returns></returns>
	template<class TArchiveKey>
	KeyValue<TArchiveKey, TValue, Validators...> AdaptAndMoveToBaseKeyValue()
	{
		auto archiveCompatibleKey = BitSerializer::Convert::To<TArchiveKey>(this->GetKey());
		return BitSerializer::KeyValue<TArchiveKey, TValue, Validators...>(
			std::move(archiveCompatibleKey), this->GetValue(), std::move(this->mValidators));
	}
};

/// <summary>
/// The helper function for making an auto wrapper with a serialization value, key and validators.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <param name="validators">Validators</param>
/// <returns></returns>
template <class TKey, class TValue, class... Validators>
constexpr AutoKeyValue<TKey, TValue, Validators...> MakeAutoKeyValue(TKey&& key, TValue& value, const Validators&... validators) noexcept {
	return AutoKeyValue<TKey, TValue, Validators...>(std::forward<TKey>(key), value, validators...);
}

}	// namespace BitSerializer
