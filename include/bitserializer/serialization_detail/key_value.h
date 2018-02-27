/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

namespace BitSerializer {

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

template <typename TKey, typename TValue>
constexpr KeyValue<TKey, TValue> MakeKeyValue(const TKey* key, TValue& value) noexcept {
	return KeyValue<TKey, TValue>(key, value);
}

}	// namespace BitSerializer
