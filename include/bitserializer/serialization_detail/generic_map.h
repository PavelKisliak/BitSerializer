/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
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
		 * Uses the common associative-container interface (`find` and `emplace_hint`), so it behaves
		 * uniformly for unique-key maps and multimaps:
		 * - `Replace` / `InsertOrUpdate` insert the entry; `emplace_hint` updates an existing key in a
		 *   unique-key map, but always appends a new element in a multimap;
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
			if constexpr (has_upper_bound_v<TMap> && !has_try_emplace_v<TMap>)
			{
				// Ordered multimap: insert at the upper bound to preserve the order of duplicate keys
				const auto insertPos = cont.upper_bound(key);
				it = cont.emplace_hint(insertPos, std::forward<TKey>(key), typename TMap::mapped_type{});
			}
			else
			{
				// Unique-key map or unordered multimap
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
		 * @brief Generic function for serializing multimap-like containers.
		 *
		 * Multimaps may contain duplicate keys, so they are serialized as an array of pairs
		 * to ensure compatibility with archives that do not support duplicate keys.
		 *
		 * @tparam TArchive   The archive type used for serialization.
		 * @tparam TMultiMap  Type of the multimap container.
		 * @param arrayScope  Archive scope object used for serializing array content.
		 * @param cont        Reference to the multimap being serialized.
		 */
		template<typename TArchive, typename TMultiMap>
		void SerializeMultiMapImpl(TArchive& arrayScope, TMultiMap& cont)
		{
			if constexpr (TArchive::IsLoading())
			{
				cont.clear();
				auto hint = cont.begin();
				while (!arrayScope.IsEnd())
				{
					typename TMultiMap::value_type pair;
					if (Serialize(arrayScope, pair))
					{
						hint = cont.emplace_hint(hint, std::move(pair));
					}
				}
			}
			else
			{
				for (auto& elem : cont)
				{
					Serialize(arrayScope, elem);
				}
			}
		}
} // namespace BitSerializer::Detail
