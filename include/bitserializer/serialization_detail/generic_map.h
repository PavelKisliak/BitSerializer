/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/convert.h"
#include "bitserializer/types/std/pair.h"
#include "bitserializer/serialization_detail/archive_traits.h"
#include "bitserializer/serialization_detail/bin_timestamp.h"

namespace BitSerializer
{
	/**
	 * @brief Specifies how a map should be loaded during deserialization.
	 */
	enum class MapLoadMode
	{
		Clean,			///< Clear the map before loading (default behavior).
		OnlyExistKeys,	///< Only update existing keys; ignore new ones.
		UpdateKeys,		///< Update existing keys or insert new ones as needed.
	};

	namespace Detail
	{
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
		 * @param mapLoadMode Load behavior mode (default: Clean).
		 */
		template<typename TArchive, typename TMap>
		static void SerializeMapImpl(TArchive& scope, TMap& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
		{
			constexpr auto hasSupportKeyType = BitSerializer::is_convertible_to_one_from_tuple_v<typename TMap::key_type, typename TArchive::supported_key_types>;
			constexpr auto hasSupportBinTimestamp = BitSerializer::is_convertible_to_one_from_tuple_v<CBinTimestamp, typename TArchive::supported_key_types>;
			if constexpr (TArchive::IsSaving())
			{
				for (auto& elem : cont)
				{
					// If the archive supports the exact key type
					if constexpr (hasSupportKeyType)
					{
						Serialize(scope, elem.first, elem.second);
					}
					// If the key can be converted to CBinTimestamp and the archive supports it
					else if constexpr (hasSupportBinTimestamp && Convert::IsConvertible<typename TMap::key_type, CBinTimestamp>())
					{
						const auto timestamp = Convert::To<CBinTimestamp>(elem.first);
						Serialize(scope, timestamp, elem.second);
					}
					else
					{
						// Save key as string fallback
						const auto strKey = Convert::To<typename TArchive::key_type>(elem.first);
						Serialize(scope, strKey, elem.second);
					}
				}
			}
			else
			{
				if (mapLoadMode == MapLoadMode::Clean)
				{
					cont.clear();
				}

				if constexpr (has_reserve_v<TMap>)
				{
					// Reserve capacity when approximate size is known
					if (const auto estimatedSize = scope.GetEstimatedSize(); estimatedSize != 0 && mapLoadMode != MapLoadMode::OnlyExistKeys)
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
						switch (mapLoadMode)
						{
						case MapLoadMode::Clean:
							hint = cont.try_emplace(hint, std::move(key));
							Serialize(scope, archiveKey, hint->second);
							break;
						case MapLoadMode::OnlyExistKeys:
							hint = cont.find(key);
							if (hint != cont.end())
							{
								Serialize(scope, archiveKey, hint->second);
							}
							break;
						case MapLoadMode::UpdateKeys:
							Serialize(scope, archiveKey, cont[key]);
							break;
						}
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
	} // namespace Detail
} // namespace BitSerializer
