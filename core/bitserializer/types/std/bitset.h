/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <bitset>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
	namespace Detail
	{
		template<typename TArchive, size_t Size>
		static void SerializeBitSet(TArchive& scope, std::bitset<Size>& cont)
		{
			bool value;
			for (size_t i = 0; i < Size; i++)
			{
				if constexpr (TArchive::IsLoading())
				{
					Serialize(scope, value);
					cont.set(i, value);
				}
				else
				{
					value = cont.test(i);
					Serialize(scope, value);
				}
			}
		}
	}

	/// <summary>
	/// Serialize std::bitset with key.
	/// </summary>
	template<typename TArchive, typename TKey, size_t Size>
	bool Serialize(TArchive& archive, TKey&& key, std::bitset<Size>& cont)
	{
		constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
		static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

		if constexpr (hasArrayWithKeySupport)
		{
			auto arrayScope = archive.OpenArrayScope(std::forward<TKey>(key), cont.size());
			if (arrayScope)
				Detail::SerializeBitSet(*arrayScope, cont);
			return arrayScope.has_value();
		}

		return false;
	}

	/// <summary>
	/// Serialize std::bitset.
	/// </summary>
	template<typename TArchive, size_t Size>
	void Serialize(TArchive& archive, std::bitset<Size>& cont)
	{
		constexpr auto hasArraySupport = can_serialize_array_v<TArchive>;
		static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

		if constexpr (hasArraySupport)
		{
			auto arrayScope = archive.OpenArrayScope(cont.size());
			if (arrayScope)
				Detail::SerializeBitSet(*arrayScope, cont);
		}
	}
}
