/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstddef>
#include <map>
#include <optional>
#include "bitserializer/serialization_detail/generic_map.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::map`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeObject(TArchive& archive, std::map<TMapKey, TValue, TComparer, TAllocator>& cont)
	{
		Detail::SerializeMapImpl(archive, cont, archive.GetOptions().mapOptions.loadMode);
	}

	/**
	 * @brief Serializes `std::multimap`.
	 */
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, std::multimap<TMapKey, TValue, TComparer, TAllocator>& cont)
	{
		Detail::SerializeMultiMapImpl(arrayScope, cont);
	}

	/**
	 * @brief A wrapper that serializes an associative container as an object, optionally overriding the load mode.
	 *
	 * This is the default representation of `std::map` / `std::unordered_map` (a set of key-value fields).
	 * The wrapper exists to pass an explicit `MapLoadMode`, overriding `SerializationOptions::mapOptions`.
	 *
	 * @par Example:
	 * @code
	 * std::map<std::string, int> data;
	 * archive << KeyValue("data", MapAsObject(data, MapLoadMode::UpdateExisting));
	 * @endcode
	 */
	template <typename T>
	struct MapAsObject
	{
		explicit MapAsObject(T& mapValue, std::optional<MapLoadMode> mapLoadMode = std::nullopt) noexcept
			: map(mapValue)
			, mapLoadMode(mapLoadMode)
		{ }

		T& map;
		std::optional<MapLoadMode> mapLoadMode;
	};

	/**
	 * @brief Serializes `MapAsObject<...>` as an object (the default map representation).
	 */
	template <typename TArchive, typename TMap>
	void SerializeObject(TArchive& archive, MapAsObject<TMap> wrappedMap)
	{
		const auto mapLoadMode = wrappedMap.mapLoadMode.value_or(archive.GetOptions().mapOptions.loadMode);
		Detail::SerializeMapImpl(archive, wrappedMap.map, mapLoadMode);
	}

	/**
	 * @brief A wrapper that serializes an associative container as a "dictionary".
	 *
	 * A dictionary is a sequence where every item is an object with a single key, which (unlike a
	 * plain object) preserves the order of elements and allows duplicate keys. For example, in YAML:
	 * @code
	 * - shape1: { type: square, width: 100, height: 100 }
	 * - shape2: { type: circle, radius: 50 }
	 * @endcode
	 * The same data in JSON (compact output is used by default):
	 * @code
	 * [{"shape1":{"type":"square","width":100,"height":100}},{"shape2":{"type":"circle","radius":50}}]
	 * @endcode
	 * Supported by all archives except CSV.
	 *
	 * @par Example:
	 * @code
	 * std::map<std::string, CShape> shapes;
	 * archive << KeyValue("shapes", MapAsDictionary(shapes));
	 * @endcode
	 */
	template <typename T>
	struct MapAsDictionary
	{
		explicit MapAsDictionary(T& mapValue, std::optional<MapLoadMode> mapLoadMode = std::nullopt) noexcept
			: map(mapValue)
			, mapLoadMode(mapLoadMode)
		{ }

		/**
		 * @brief Returns the number of entries (required by binary archives that need a known size upfront).
		 */
		[[nodiscard]] size_t size() const { return map.size(); }

		T& map;
		std::optional<MapLoadMode> mapLoadMode;
	};

	/**
	 * @brief Serializes `MapAsDictionary<...>` as a sequence of single-key objects.
	 */
	template <typename TArchive, typename TMap>
	void SerializeArray(TArchive& arrayScope, MapAsDictionary<TMap> wrappedMap)
	{
		static_assert(TArchive::archive_type != ArchiveType::Csv,
			"BitSerializer. MapAsDictionary is not supported by the CSV archive (dynamic keys cannot be represented as fixed columns).");

		using TKey = typename TMap::key_type;
		auto& targetMap = wrappedMap.map;

		if constexpr (TArchive::IsLoading())
		{
			const auto mapLoadMode = wrappedMap.mapLoadMode.value_or(arrayScope.GetOptions().mapOptions.loadMode);
			if (mapLoadMode == MapLoadMode::Replace)
			{
				targetMap.clear();
			}

			if constexpr (has_reserve_v<TMap>)
			{
				// Reserve capacity when approximate size is known
				if (const auto estimatedSize = arrayScope.GetEstimatedSize(); estimatedSize != 0 && mapLoadMode != MapLoadMode::UpdateExisting)
				{
					targetMap.reserve(estimatedSize);
				}
			}

			auto hint = targetMap.begin();
			while (!arrayScope.IsEnd())
			{
				auto objectScope = arrayScope.OpenObjectScope(1);
				if (objectScope)
				{
					objectScope.VisitKeys([&](auto&& archiveKey)
					{
						TKey key;
						if (Detail::ConvertByPolicy(archiveKey, key, objectScope.GetOptions().mismatchedTypesPolicy, objectScope.GetOptions().overflowNumberPolicy))
						{
							hint = Detail::LoadMapEntry(targetMap, std::move(key), mapLoadMode, hint,
								[&objectScope, &archiveKey](auto& value) { Serialize(objectScope, archiveKey, value); });
						}
					});
				}
			}
		}
		else
		{
			for (auto& elem : targetMap)
			{
				auto objectScope = arrayScope.OpenObjectScope(1);
				if (objectScope)
				{
					Detail::SaveMapEntry(objectScope, elem.first, elem.second);
				}
			}
		}
	}
}
