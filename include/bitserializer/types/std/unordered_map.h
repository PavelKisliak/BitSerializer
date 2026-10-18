/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include <unordered_map>
#include "bitserializer/serialization_detail/generic_map.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::unordered_map` (as an object by default).
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeObject(TArchive& archive, std::unordered_map<TMapKey, TValue, THasher, TKeyEq, TAllocator>& cont)
	{
		Detail::SerializeMapImpl(archive, cont, archive.GetOptions().mapOptions.loadMode);
	}

	/**
	 * @brief Serializes `std::unordered_multimap` as a dictionary (a sequence of single-key objects).
	 *
	 * Use `MapAsPairs` to serialize it as a sequence of `{key, value}` objects instead.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, std::unordered_multimap<TMapKey, TValue, THasher, TKeyEq, TAllocator>& cont)
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
	 * @brief Serializes `std::unordered_map` wrapped with `MapAsObject`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeObject(TArchive& archive, MapAsObject<std::unordered_map<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapImpl(archive, wrappedMap.map, wrappedMap.mapLoadMode.value_or(archive.GetOptions().mapOptions.loadMode));
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeObject(TArchive& archive, MapAsObject<const std::unordered_map<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapImpl(archive, wrappedMap.map, wrappedMap.mapLoadMode.value_or(archive.GetOptions().mapOptions.loadMode));
	}

	/**
	 * @brief Serializes `std::unordered_multimap` wrapped with `MapAsObject`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeObject(TArchive& archive, MapAsObject<std::unordered_multimap<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapImpl(archive, wrappedMap.map, wrappedMap.mapLoadMode.value_or(archive.GetOptions().mapOptions.loadMode));
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeObject(TArchive& archive, MapAsObject<const std::unordered_multimap<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapImpl(archive, wrappedMap.map, wrappedMap.mapLoadMode.value_or(archive.GetOptions().mapOptions.loadMode));
	}

	/**
	 * @brief Serializes `std::unordered_map` wrapped with `MapAsDictionary`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsDictionary<std::unordered_map<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsDictionaryImpl(arrayScope, wrappedMap.map, wrappedMap.mapLoadMode);
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsDictionary<const std::unordered_map<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsDictionaryImpl(arrayScope, wrappedMap.map, wrappedMap.mapLoadMode);
	}

	/**
	 * @brief Serializes `std::unordered_multimap` wrapped with `MapAsDictionary`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsDictionary<std::unordered_multimap<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsDictionaryImpl(arrayScope, wrappedMap.map, wrappedMap.mapLoadMode);
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsDictionary<const std::unordered_multimap<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsDictionaryImpl(arrayScope, wrappedMap.map, wrappedMap.mapLoadMode);
	}

	/**
	 * @brief Serializes `std::unordered_map` wrapped with `MapAsPairs`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsPairs<std::unordered_map<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsPairsImpl(arrayScope, wrappedMap.map, wrappedMap.keyField, wrappedMap.valueField, wrappedMap.mapLoadMode);
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsPairs<const std::unordered_map<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsPairsImpl(arrayScope, wrappedMap.map, wrappedMap.keyField, wrappedMap.valueField, wrappedMap.mapLoadMode);
	}

	/**
	 * @brief Serializes `std::unordered_multimap` wrapped with `MapAsPairs`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsPairs<std::unordered_multimap<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsPairsImpl(arrayScope, wrappedMap.map, wrappedMap.keyField, wrappedMap.valueField, wrappedMap.mapLoadMode);
	}

	template<typename TArchive, typename TMapKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, MapAsPairs<const std::unordered_multimap<TMapKey, TValue, THasher, TKeyEq, TAllocator>> wrappedMap)
	{
		Detail::SerializeMapAsPairsImpl(arrayScope, wrappedMap.map, wrappedMap.keyField, wrappedMap.valueField, wrappedMap.mapLoadMode);
	}
}
