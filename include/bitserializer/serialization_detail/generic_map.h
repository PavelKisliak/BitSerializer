/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstddef>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include "bitserializer/convert.h"
#include "bitserializer/serialization_options.h"
#include "bitserializer/types/std/pair.h"
#include "bitserializer/serialization_detail/archive_traits.h"
#include "bitserializer/serialization_detail/archive_key.h"
#include "bitserializer/serialization_detail/bin_timestamp.h"

namespace BitSerializer::Detail
{
		/**
		 * @brief Detects whether a container has unique keys (i.e. `try_emplace`), such as `std::map` / `std::unordered_map`.
		 */
		template <typename TMap, typename = void>
		struct has_try_emplace : std::false_type {};

		template <typename TMap>
		struct has_try_emplace<TMap, std::void_t<decltype(std::declval<TMap&>().try_emplace(std::declval<const typename TMap::key_type&>()))>> : std::true_type {};

		template <typename TMap>
		constexpr bool has_try_emplace_v = has_try_emplace<TMap>::value;

		/**
		 * @brief Detects whether a container supports `upper_bound` (i.e. is ordered), such as `std::multimap`.
		 */
		template <typename TMap, typename = void>
		struct has_upper_bound : std::false_type {};

		template <typename TMap>
		struct has_upper_bound<TMap, std::void_t<decltype(std::declval<TMap&>().upper_bound(std::declval<const typename TMap::key_type&>()))>> : std::true_type {};

		template <typename TMap>
		constexpr bool has_upper_bound_v = has_upper_bound<TMap>::value;

		/**
		 * @brief Loads a single key-value entry into an associative container, honoring the load mode.
		 *
		 * Uses the common associative-container interface (`find`, and either `try_emplace` for
		 * unique-key maps or `emplace_hint` for multimaps):
		 * - `Replace` / `InsertOrUpdate` insert the entry; `try_emplace` updates an existing key of a
		 *   unique-key map, while `emplace_hint` always appends a new element to a multimap;
		 * - `UpdateExisting` skips the entry when its key is absent.
		 *
		 * For ordered multimaps the element is inserted at the upper bound of the key, so that the
		 * original order of duplicate keys is preserved.
		 *
		 * @tparam TMap            Type of the associative container.
		 * @tparam TKey            Type of the entry key.
		 * @tparam TSerializeValue Callback that deserializes the value into the target slot.
		 * @param cont             Target associative container.
		 * @param key              Key of the entry being loaded.
		 * @param mapLoadMode      Load behavior mode.
		 * @param hint             Iterator hint for the insertion point.
		 * @param serializeValue   Callback invoked with a reference to the value slot (`it->second`).
		 * @return Iterator to the updated/inserted element (or `hint` when the entry was skipped).
		 */
		template<typename TMap, typename TKey, typename TSerializeValue>
		typename TMap::iterator LoadMapEntry(TMap& cont, TKey&& key, MapLoadMode mapLoadMode,
			typename TMap::iterator hint, TSerializeValue&& serializeValue)
		{
			if (mapLoadMode == MapLoadMode::UpdateExisting && cont.find(key) == cont.end())
			{
				return hint;
			}

			typename TMap::iterator it = hint;
			if constexpr (has_try_emplace_v<TMap>)
			{
				// Unique-key map (std::map / std::unordered_map): try_emplace avoids constructing
				// the mapped value when the key already exists
				it = cont.try_emplace(hint, std::forward<TKey>(key));
			}
			else if constexpr (has_upper_bound_v<TMap>)
			{
				// Ordered multimap: insert at the upper bound to preserve the order of duplicate keys
				const auto insertPos = cont.upper_bound(key);
				it = cont.emplace_hint(insertPos, std::forward<TKey>(key), typename TMap::mapped_type{});
			}
			else
			{
				// Unordered multimap: the order of duplicate keys is not defined
				it = cont.emplace_hint(hint, std::forward<TKey>(key), typename TMap::mapped_type{});
			}

			serializeValue(it->second);
			return it;
		}

		/**
		 * @brief Serializes a single map entry under the specified key.
		 *
		 * Serializes the key as-is when the archive supports its type, otherwise converts
		 * the key to a string (or to a binary timestamp when the archive supports it).
		 *
		 * @tparam TArchive Archive type used for serialization.
		 * @tparam TKey     Type of the entry key.
		 * @tparam TValue   Type of the entry value.
		 * @param scope		Archive scope object used for serializing the entry.
		 * @param key		Key of the entry.
		 * @param value		Value of the entry.
		 */
		template<typename TArchive, typename TKey, typename TValue>
		void SaveMapEntry(TArchive& scope, TKey& key, TValue& value)
		{
			constexpr auto hasSupportKeyType = is_convertible_to_any_of_tuple_v<TKey, typename TArchive::supported_key_types>;
			constexpr auto hasSupportBinTimestamp = is_convertible_to_any_of_tuple_v<BinTimestamp, typename TArchive::supported_key_types>;

			// If the archive supports the exact key type
			if constexpr (hasSupportKeyType)
			{
				Serialize(scope, key, value);
			}
			// If the key can be converted to BinTimestamp and the archive supports it
			else if constexpr (hasSupportBinTimestamp && Convert::IsConvertible<TKey, BinTimestamp>())
			{
				const auto timestamp = Convert::To<BinTimestamp>(key);
				Serialize(scope, timestamp, value);
			}
			else
			{
				// Save key as string fallback
				const auto strKey = Convert::To<typename TArchive::key_type>(key);
				Serialize(scope, strKey, value);
			}
		}

		/**
		 * @brief Generic function for serializing associative containers like `std::map` or `std::unordered_map`.
		 *
		 * Supports both saving and loading operations, with customizable map load behavior.
		 *
		 * When saving:
		 * - Serializes each key-value pair directly if the archive supports the key type.
		 * - Converts keys to strings if the target archive does not support the specified type.
		 *
		 * When loading:
		 * - Clears, updates, or merges based on the specified `MapLoadMode`.
		 * - Uses hinting and reserve strategies for performance optimization.
		 *
		 * @tparam TArchive   The archive type used for serialization.
		 * @tparam TMap       Type of the associative container.
		 * @param scope       Archive scope object used for serializing map content.
		 * @param cont        Reference to the map being serialized.
		 * @param mapLoadMode Load behavior mode (default: Replace).
		 */
		template<typename TArchive, typename TMap>
		static void SerializeMapImpl(TArchive& scope, TMap& cont, MapLoadMode mapLoadMode = MapLoadMode::Replace)
		{
			if constexpr (TArchive::IsSaving())
			{
				for (auto& elem : cont)
				{
					SaveMapEntry(scope, elem.first, elem.second);
				}
			}
			else
			{
				if (mapLoadMode == MapLoadMode::Replace)
				{
					cont.clear();
				}

				if constexpr (has_reserve_v<TMap>)
				{
					// Reserve capacity when approximate size is known
					if (const auto estimatedSize = scope.GetEstimatedSize(); estimatedSize != 0 && mapLoadMode != MapLoadMode::UpdateExisting)
					{
						cont.reserve(estimatedSize);
					}
				}

				auto hint = cont.begin();
				scope.VisitKeys([mapLoadMode, &scope, &cont, &hint](auto&& archiveKey)
				{
					// Convert archive key to target map's key type
					typename TMap::key_type key;
					if (ConvertByPolicy(archiveKey, key, scope.GetOptions().mismatchedTypesPolicy, scope.GetOptions().overflowNumberPolicy))
					{
						hint = LoadMapEntry(cont, std::move(key), mapLoadMode, hint,
							[&scope, &archiveKey](auto& value) { Serialize(scope, archiveKey, value); });
					}
				});
			}
		}

		/**
		 * @brief Serializes an associative container as a "dictionary" - a sequence of single-key objects.
		 *
		 * Unlike a plain object, this preserves the order of elements and allows duplicate keys. Supported
		 * by all archives except CSV (dynamic keys cannot be represented as fixed columns; use `MapAsPairs`).
		 *
		 * @tparam TArchive     Archive type used for serialization.
		 * @tparam TMap         Type of the associative container.
		 * @param arrayScope    Archive scope object used for serializing array content.
		 * @param targetMap     Reference to the map being serialized.
		 * @param explicitMode  Optional load mode; when not set, `SerializationOptions::mapOptions.loadMode` is used.
		 */
		template<typename TArchive, typename TMap>
		void SerializeMapAsDictionaryImpl(TArchive& arrayScope, TMap& targetMap, std::optional<MapLoadMode> explicitMode)
		{
			static_assert(TArchive::archive_type != ArchiveType::Csv,
				"BitSerializer. The CSV archive does not support the dictionary representation (dynamic keys cannot be represented as fixed columns); use 'MapAsPairs' instead.");

			using TKey = typename TMap::key_type;

			if constexpr (TArchive::IsLoading())
			{
				const auto mapLoadMode = explicitMode.value_or(arrayScope.GetOptions().mapOptions.loadMode);
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
							// Convert archive key to target map's key type
							TKey key;
							if (ConvertByPolicy(archiveKey, key, objectScope.GetOptions().mismatchedTypesPolicy, objectScope.GetOptions().overflowNumberPolicy))
							{
								hint = LoadMapEntry(targetMap, std::move(key), mapLoadMode, hint,
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
						SaveMapEntry(objectScope, elem.first, elem.second);
					}
				}
			}
		}

		/**
		 * @brief Serializes an associative container as a sequence of `{key, value}` objects.
		 *
		 * Unlike the dictionary representation, this form works with flat (tabular) archives such as CSV,
		 * because the key and value field names are fixed.
		 *
		 * @tparam TArchive       Archive type used for serialization.
		 * @tparam TMap           Type of the associative container.
		 * @param arrayScope      Archive scope object used for serializing array content.
		 * @param targetMap       Reference to the map being serialized.
		 * @param keyFieldName    Name of the field that stores the key.
		 * @param valueFieldName  Name of the field that stores the value.
		 * @param explicitMode    Optional load mode; when not set, `SerializationOptions::mapOptions.loadMode` is used.
		 */
		template<typename TArchive, typename TMap>
		void SerializeMapAsPairsImpl(TArchive& arrayScope, TMap& targetMap,
			std::string_view keyFieldName, std::string_view valueFieldName, std::optional<MapLoadMode> explicitMode)
		{
			using TKey = typename TMap::key_type;
			const auto keyField = Convert::To<typename TArchive::key_type>(keyFieldName);
			const auto valueField = Convert::To<typename TArchive::key_type>(valueFieldName);

			if constexpr (TArchive::IsLoading())
			{
				const auto mapLoadMode = explicitMode.value_or(arrayScope.GetOptions().mapOptions.loadMode);
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
					auto objectScope = arrayScope.OpenObjectScope(2);
					if (objectScope)
					{
						TKey key;
						if (Serialize(objectScope, keyField, key))
						{
							hint = LoadMapEntry(targetMap, std::move(key), mapLoadMode, hint,
								[&objectScope, &valueField](auto& value) { Serialize(objectScope, valueField, value); });
						}
					}
				}
			}
			else
			{
				for (auto& elem : targetMap)
				{
					auto objectScope = arrayScope.OpenObjectScope(2);
					if (objectScope)
					{
						Serialize(objectScope, keyField, const_cast<std::remove_const_t<TKey>&>(elem.first));
						Serialize(objectScope, valueField, elem.second);
					}
				}
			}
		}
} // namespace BitSerializer::Detail

namespace BitSerializer
{
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
	 * This is the default representation of multimap-like containers (e.g. `std::multimap`).
	 * Supported by all archives except CSV (use `MapAsPairs` there instead).
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
	 * @brief A wrapper that serializes an associative container as a sequence of `{key, value}` objects.
	 *
	 * Unlike `MapAsDictionary`, this representation uses fixed field names, so it also works with flat
	 * (tabular) archives such as CSV:
	 * @code
	 * - key: color
	 *   value: red
	 * - key: color
	 *   value: blue
	 * @endcode
	 * The field names are customizable, and the load mode can be passed explicitly, overriding
	 * `SerializationOptions::mapOptions`.
	 *
	 * @par Example:
	 * @code
	 * std::multimap<std::string, int> data;
	 * archive << KeyValue("data", MapAsPairs(data));
	 * archive << KeyValue("data", MapAsPairs(data, MapLoadMode::InsertOrUpdate, "name", "count"));
	 * @endcode
	 */
	template <typename T>
	struct MapAsPairs
	{
		explicit MapAsPairs(T& mapValue, std::optional<MapLoadMode> mapLoadMode = std::nullopt,
			std::string_view keyField = "key", std::string_view valueField = "value") noexcept
			: map(mapValue)
			, keyField(keyField)
			, valueField(valueField)
			, mapLoadMode(mapLoadMode)
		{ }

		/**
		 * @brief Returns the number of entries (required by binary archives that need a known size upfront).
		 */
		[[nodiscard]] size_t size() const { return map.size(); }

		T& map;
		std::string_view keyField;
		std::string_view valueField;
		std::optional<MapLoadMode> mapLoadMode;
	};
} // namespace BitSerializer
