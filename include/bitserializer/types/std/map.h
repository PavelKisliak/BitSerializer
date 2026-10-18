/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <map>
#include <optional>
#include "bitserializer/serialization_detail/generic_map.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::map` (as an object by default).
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeObject(TArchive& archive, std::map<TMapKey, TValue, TComparer, TAllocator>& cont)
	{
		Detail::SerializeMapImpl(archive, cont, archive.GetOptions().mapOptions.loadMode);
	}

	/**
	 * @brief Serializes `std::multimap` as a dictionary (a sequence of single-key objects).
	 *
	 * Use `MapAsPairs` to serialize it as a sequence of `{key, value}` objects instead.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, std::multimap<TMapKey, TValue, TComparer, TAllocator>& cont)
	{
		Detail::SerializeMapAsDictionaryImpl(arrayScope, cont, std::nullopt);
	}

	//-----------------------------------------------------------------------------
	// Serialization of wrappers (MapAsObject / MapAsDictionary / MapAsPairs).
	//
	// Each wrapper is supported only for the known containers, so third-party map
	// types (e.g. Qt's QMap) can provide their own overloads without ambiguity.
	// The `const` overloads enable saving const containers.
	//-----------------------------------------------------------------------------

	/**
	 * @brief Serializes `std::map` wrapped with `MapAsObject`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeObject(TArchive& archive, MapAsObject<std::map<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapImpl(archive, wrappedMap.map, wrappedMap.mapLoadMode.value_or(archive.GetOptions().mapOptions.loadMode));
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeObject(TArchive& archive, MapAsObject<const std::map<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapImpl(archive, wrappedMap.map, wrappedMap.mapLoadMode.value_or(archive.GetOptions().mapOptions.loadMode));
	}

	/**
	 * @brief Serializes `std::multimap` wrapped with `MapAsObject`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeObject(TArchive& archive, MapAsObject<std::multimap<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapImpl(archive, wrappedMap.map, wrappedMap.mapLoadMode.value_or(archive.GetOptions().mapOptions.loadMode));
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeObject(TArchive& archive, MapAsObject<const std::multimap<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapImpl(archive, wrappedMap.map, wrappedMap.mapLoadMode.value_or(archive.GetOptions().mapOptions.loadMode));
	}

	/**
	 * @brief Serializes `std::map` wrapped with `MapAsDictionary`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsDictionary<std::map<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsDictionaryImpl(arrayScope, wrappedMap.map, wrappedMap.mapLoadMode);
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsDictionary<const std::map<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsDictionaryImpl(arrayScope, wrappedMap.map, wrappedMap.mapLoadMode);
	}

	/**
	 * @brief Serializes `std::multimap` wrapped with `MapAsDictionary`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsDictionary<std::multimap<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsDictionaryImpl(arrayScope, wrappedMap.map, wrappedMap.mapLoadMode);
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsDictionary<const std::multimap<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsDictionaryImpl(arrayScope, wrappedMap.map, wrappedMap.mapLoadMode);
	}

	/**
	 * @brief Serializes `std::map` wrapped with `MapAsPairs`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsPairs<std::map<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsPairsImpl(arrayScope, wrappedMap.map, wrappedMap.keyField, wrappedMap.valueField, wrappedMap.mapLoadMode);
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsPairs<const std::map<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsPairsImpl(arrayScope, wrappedMap.map, wrappedMap.keyField, wrappedMap.valueField, wrappedMap.mapLoadMode);
	}

	/**
	 * @brief Serializes `std::multimap` wrapped with `MapAsPairs`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsPairs<std::multimap<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsPairsImpl(arrayScope, wrappedMap.map, wrappedMap.keyField, wrappedMap.valueField, wrappedMap.mapLoadMode);
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsPairs<const std::multimap<TMapKey, TValue, TComparer, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsPairsImpl(arrayScope, wrappedMap.map, wrappedMap.keyField, wrappedMap.valueField, wrappedMap.mapLoadMode);
	}
}
