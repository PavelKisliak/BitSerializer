/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <map>
#include "bitserializer/types/std/pair.h"
#include "bitserializer/serialization_detail/generic_map.h"

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::map.
	/// </summary>
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeObject(TArchive& archive, std::map<TMapKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
	{
		Detail::SerializeMapImpl(archive, cont, mapLoadMode);
	}

	/// <summary>
	/// Serializes std::multimap.
	/// </summary>
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& archive, std::multimap<TMapKey, TValue, TComparer, TAllocator>& cont)
	{
		using TMultiMap = std::multimap<TMapKey, TValue, TComparer, TAllocator>;
		if constexpr (TArchive::IsLoading())
		{
			auto loadSize = archive.GetSize();
			cont.clear();
			auto hint = cont.begin();
			for (size_t c = 0; c < loadSize; c++)
			{
				typename TMultiMap::value_type pair;
				Serialize(archive, pair);
				hint = cont.emplace_hint(hint, std::move(pair));
			}
		}
		else
		{
			for (auto& elem : cont) {
				Serialize(archive, elem);
			}
		}
	}
}
