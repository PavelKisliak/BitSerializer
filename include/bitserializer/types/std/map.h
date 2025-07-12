/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <map>
#include "bitserializer/serialization_detail/generic_map.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::map`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeObject(TArchive& archive, std::map<TMapKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
	{
		Detail::SerializeMapImpl(archive, cont, mapLoadMode);
	}

	/**
	 * @brief Serializes `std::multimap`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, std::multimap<TMapKey, TValue, TComparer, TAllocator>& cont)
	{
		Detail::SerializeMultiMapImpl(arrayScope, cont);
	}
}
