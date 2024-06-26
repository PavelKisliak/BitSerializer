/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <atomic>

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::atomic.
	/// </summary>
	template<typename TArchive, typename TKey, typename TValue>
	bool Serialize(TArchive& archive, TKey&& key, std::atomic<TValue>& value)
	{
		constexpr auto hasValueWithKeySupport = can_serialize_value_with_key_v<TArchive, TValue, TKey>;
		static_assert(hasValueWithKeySupport, "BitSerializer. The archive doesn't support serialize fundamental type with key on this level.");

		if constexpr (hasValueWithKeySupport)
		{
			TValue temp;
			if constexpr (TArchive::IsLoading())
			{
				archive.SerializeValue(std::forward<TKey>(key), temp);
				value.store(temp);
			}
			else
			{
				temp = value.load();
				return archive.SerializeValue(std::forward<TKey>(key), temp);
			}
		}
		return false;
	}

	template <typename TArchive, typename TValue>
	bool Serialize(TArchive& archive, std::atomic<TValue>& value)
	{
		constexpr auto hasValueTypeSupport = can_serialize_value_v<TArchive, TValue>;
		static_assert(hasValueTypeSupport, "BitSerializer. The archive doesn't support serialize fundamental type without key on this level.");

		if constexpr (hasValueTypeSupport)
		{
			TValue temp;
			if constexpr (TArchive::IsLoading())
			{
				archive.SerializeValue(temp);
				value.store(temp);
			}
			else
			{
				temp = value.load();
				return archive.SerializeValue(temp);
			}
		}
		return false;
	}
}
