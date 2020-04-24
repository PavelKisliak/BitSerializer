/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <algorithm>
#include <type_traits>
#include "bitserializer/string_conversion.h"

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

			if constexpr (TArchive::IsSaving())
			{
				for (auto& elem : cont)
				{
					if constexpr (std::is_convertible_v<TMapKey, typename TArchive::key_type>)
						Serialize(scope, elem.first, elem.second);
					else
					{
						const auto strKey = Convert::To<typename TArchive::key_type>(elem.first);
						Serialize(scope, strKey, elem.second);
					}
				}
			}
			else
			{
				auto loadSize = scope.GetSize();
				if (mapLoadMode == MapLoadMode::Clean)
					cont.clear();
				auto hint = cont.begin();
				auto endIt = scope.cend();
				for (auto it = scope.cbegin(); it != endIt; ++it)
				{
					decltype(auto) archiveKey = *it;
					TMapKey key;
					if constexpr (std::is_convertible_v<TMapKey, typename TArchive::key_type>)
						key = archiveKey;
					else
						key = Convert::To<TMapKey>(archiveKey);
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
					default:
						break;
					}
				}
			}
		}
	}
}
