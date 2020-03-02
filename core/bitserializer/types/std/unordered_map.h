/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <unordered_map>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
	/// <summary>
	/// Serialize std::unordered_map with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	static bool Serialize(TArchive& archive, TKey&& key, std::unordered_map<TMapKey, TValue, TComparer, TAllocator>& cont,
		MapLoadMode mapLoadMode = MapLoadMode::Clean)
	{
		constexpr auto hasObjectWithKeySupport = can_serialize_object_with_key_v<TArchive, TKey>;
		static_assert(hasObjectWithKeySupport, "BitSerializer. The archive doesn't support serialize object with key on this level.");

		if constexpr (hasObjectWithKeySupport)
		{
			auto objectScope = archive.OpenObjectScope(key);
			if (objectScope)
				Detail::SerializeMapImpl(*objectScope, cont, mapLoadMode);
			return objectScope.has_value();
		}

		return false;
	}

	/// <summary>
	/// Serialize std::unordered_map.
	/// </summary>
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	static void Serialize(TArchive& archive, std::unordered_map<TMapKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
	{
		constexpr auto hasObjectSupport = can_serialize_object_v<TArchive>;
		static_assert(hasObjectSupport, "BitSerializer. The archive doesn't support serialize object without key on this level.");

		if constexpr (hasObjectSupport)
		{
			auto objectScope = archive.OpenObjectScope();
			if (objectScope)
				Detail::SerializeMapImpl(*objectScope, cont, mapLoadMode);
		}
	}
}
