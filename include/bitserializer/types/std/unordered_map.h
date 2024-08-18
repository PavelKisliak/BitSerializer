/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
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
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeObject(TArchive& archive, std::unordered_map<TMapKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
	{
		Detail::SerializeMapImpl(archive, cont, mapLoadMode);
	}

	/// <summary>
	/// Serializes std::unordered_multimap.
	/// </summary>
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, std::unordered_multimap<TMapKey, TValue, TComparer, TAllocator>& cont)
	{
		Detail::SerializeMultiMapImpl(arrayScope, cont);
	}
}
