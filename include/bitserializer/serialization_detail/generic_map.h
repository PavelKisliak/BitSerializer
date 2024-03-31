/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <algorithm>
#include <type_traits>
#include "bitserializer/convert.h"
#include "bitserializer/serialization_detail/bin_timestamp.h"

namespace BitSerializer
{
	/// <summary>
	/// The enumeration of available modes for loading maps.
	/// </summary>
	enum class MapLoadMode
	{
		/// <summary>
		/// Clean before load (default).
		/// </summary>
		Clean,

		/// <summary>
		/// Load only objects which already exist in the map.
		/// </summary>
		OnlyExistKeys,

		/// <summary>
		/// Load to existing or new objects.
		/// </summary>
		UpdateKeys
	};

	namespace Detail
	{
		/// <summary>
		/// Generic function for serialization maps.
		/// </summary>
		template<typename TArchive, typename TMap>
		static void SerializeMapImpl(TArchive& scope, TMap& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
		{
			using TMapKey = typename TMap::key_type;
			using TValue = typename TMap::mapped_type;

			constexpr auto hasSupportKeyType = BitSerializer::is_convertible_to_one_from_tuple_v<TMapKey, typename TArchive::supported_key_types>;
			constexpr auto hasSupportBinTimestamp = BitSerializer::is_convertible_to_one_from_tuple_v<CBinTimestamp, typename TArchive::supported_key_types>;
			if constexpr (TArchive::IsSaving())
			{
				for (auto& elem : cont)
				{
					// If the archive supports the exact key type
					if constexpr (hasSupportKeyType) {
						Serialize(scope, elem.first, elem.second);
					}
					// If the archive supports serialization CBinTimestamp and key is convertible to it
					else if constexpr (hasSupportBinTimestamp && Convert::IsConvertible<TMapKey, CBinTimestamp>())
					{
						const auto timestamp = Convert::To<CBinTimestamp>(elem.first);
						Serialize(scope, timestamp, elem.second);
					}
					else
					{
						// Save key as string
						const auto strKey = Convert::To<typename TArchive::key_type>(elem.first);
						Serialize(scope, strKey, elem.second);
					}
				}
			}
			else
			{
				if (mapLoadMode == MapLoadMode::Clean)
					cont.clear();

				if constexpr (has_reserve_v<TMap>)
				{
					// Reserve map capacity (like for std::unordered_map) when the approximate size is known
					if (const auto estimatedSize = scope.GetEstimatedSize(); estimatedSize != 0 && mapLoadMode != MapLoadMode::OnlyExistKeys) {
						cont.reserve(estimatedSize);
					}
				}

				auto hint = cont.begin();
				scope.VisitKeys([mapLoadMode, &scope, &cont, &hint](auto&& archiveKey)
				{
					// Converting an archive key to key type of target map
					TMapKey key;
					if (ConvertByPolicy(archiveKey, key, scope.GetOptions()))
					{
						switch (mapLoadMode)
						{
						case MapLoadMode::Clean:
							hint = cont.emplace_hint(hint, std::move(key), TValue());
							Serialize(scope, archiveKey, hint->second);
							break;
						case MapLoadMode::OnlyExistKeys:
							hint = cont.find(key);
							if (hint != cont.end())
								Serialize(scope, archiveKey, hint->second);
							break;
						case MapLoadMode::UpdateKeys:
							Serialize(scope, archiveKey, cont[key]);
							break;
						}
					}
				});
			}
		}
	}
}
