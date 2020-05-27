/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <unordered_set>
#include "bitserializer/serialization_detail/archive_traits.h"
#include "bitserializer/serialization_detail/generic_set.h"

namespace BitSerializer
{
	/// <summary>
	/// Serialize std::unordered_set with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TValue, typename THasher, typename TComparer, typename TAllocator>
	static bool Serialize(TArchive& archive, TKey&& key, std::unordered_set<TValue, THasher, TComparer, TAllocator>& cont)
	{
		constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
		static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

		if constexpr (hasArrayWithKeySupport)
		{
			auto arrayScope = archive.OpenArrayScope(std::forward<TKey>(key), cont.size());
			if (arrayScope)
				Detail::SerializeSetImpl(*arrayScope, cont);
			return arrayScope.has_value();
		}

		return false;
	}

	/// <summary>
	/// Serialize std::unordered_set.
	/// </summary>
	template<typename TArchive, typename TValue, typename THasher, typename TComparer, typename TAllocator>
	static void Serialize(TArchive& archive, std::unordered_set<TValue, THasher, TComparer, TAllocator>& cont)
	{
		constexpr auto hasArraySupport = can_serialize_array_v<TArchive>;
		static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

		if constexpr (hasArraySupport)
		{
			auto arrayScope = archive.OpenArrayScope(cont.size());
			if (arrayScope)
				Detail::SerializeSetImpl(*arrayScope, cont);
		}
	}
}
