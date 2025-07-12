/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <unordered_map>
#include "bitserializer/serialization_detail/generic_map.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::unordered_map`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeObject(TArchive& archive, std::unordered_map<TMapKey, TValue, THasher, TKeyEq, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
	{
		Detail::SerializeMapImpl(archive, cont, mapLoadMode);
	}

	/**
	 * @brief Serializes `std::unordered_multimap`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, std::unordered_multimap<TMapKey, TValue, THasher, TKeyEq, TAllocator>& cont)
	{
		Detail::SerializeMultiMapImpl(arrayScope, cont);
	}
}
