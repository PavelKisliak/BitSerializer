/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <map>
#include "bitserializer/types/std/pair.h"

namespace BitSerializer
{
	/// <summary>
	/// Serialize std::map with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	static bool Serialize(TArchive& archive, TKey&& key, std::map<TMapKey, TValue, TComparer, TAllocator>& cont,
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
	/// Serialize std::map.
	/// </summary>
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	static void Serialize(TArchive& archive, std::map<TMapKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
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

	namespace Detail
	{
		template<typename TArchive, typename TMultiMap>
		void SerializeMultimapImpl(TArchive& scope, TMultiMap& cont)
		{
			if constexpr (scope.IsLoading())
			{
				auto loadSize = scope.GetSize();
				cont.clear();
				auto hint = cont.begin();
				for (size_t c = 0; c < loadSize; c++)
				{
					typename TMultiMap::value_type pair;
					Serialize(scope, pair);
					hint = cont.emplace_hint(hint, std::move(pair));
				}
			}
			else
			{
				for (auto& elem : cont) {
					Serialize(scope, elem);
				}
			}
		}
	}

	/// <summary>
	/// Serialize std::multimap with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	static bool Serialize(TArchive& archive, TKey&& key, std::multimap<TMapKey, TValue, TComparer, TAllocator>& cont)
	{
		constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
		static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

		if constexpr (hasArrayWithKeySupport)
		{
			auto arrayScope = archive.OpenArrayScope(key, cont.size());
			if (arrayScope)
				Detail::SerializeMultimapImpl(*arrayScope, cont);
			return arrayScope.has_value();
		}

		return false;
	}

	/// <summary>
	/// Serialize std::multimap.
	/// </summary>
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	static void Serialize(TArchive& archive, std::multimap<TMapKey, TValue, TComparer, TAllocator>& cont)
	{
		constexpr auto hasArraySupport = can_serialize_array_v<TArchive>;
		static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

		if constexpr (hasArraySupport)
		{
			auto arrayScope = archive.OpenArrayScope(cont.size());
			if (arrayScope)
				Detail::SerializeMultimapImpl(*arrayScope, cont);
		}
	}
}
