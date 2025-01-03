/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <unordered_map>
#include "bitserializer/serialization_detail/generic_map.h"

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::unordered_map.
	/// </summary>
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeObject(TArchive& archive, std::unordered_map<TMapKey, TValue, THasher, TKeyEq, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
	{
		Detail::SerializeMapImpl(archive, cont, mapLoadMode);
	}

	/// <summary>
	/// Serializes std::unordered_multimap.
	/// </summary>
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, std::unordered_multimap<TMapKey, TValue, THasher, TKeyEq, TAllocator>& cont)
	{
		Detail::SerializeMultiMapImpl(arrayScope, cont);
	}
}
