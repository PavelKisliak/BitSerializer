/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

namespace BitSerializer {

/// <summary>
/// Implementation of helper class for wrap a serialization value and key.
/// </summary>
template<typename TKey, typename TValue>
struct KeyValue
{
private:
	const TKey* Key;
	TValue* const Value;

public:
	explicit KeyValue(const TKey* key, TValue& value)
		: Key(key)
		, Value(&value)
	{}

	KeyValue(const KeyValue& rhs)
		: Key(rhs.Key)
		, Value(rhs.Value)
	{}

	inline const TKey* GetKey() const noexcept		{ return Key; }
	inline TValue& GetValue() const noexcept		{ return *Value; }
};

/// <summary>
/// The helper function for making a wrapper with a serialization value and key.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
template <typename TKey, typename TValue>
constexpr KeyValue<TKey, TValue> MakeKeyValue(const TKey* key, TValue& value) noexcept {
	return KeyValue<TKey, TValue>(key, value);
}

}	// namespace BitSerializer
