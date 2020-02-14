/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <map>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
	//-----------------------------------------------------------------------------
	// Serialize std::map
	//-----------------------------------------------------------------------------
	enum class MapLoadMode
	{
		Clean,			// Clean before load (default)
		OnlyExistKeys,	// Load only exists keys in map
		UpdateKeys,		// Load exists keys
	};

	namespace Detail
	{
		template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
		static void SerializeMapImpl(TArchive& scope, std::map<TMapKey, TValue, TComparer, TAllocator>& cont,
			MapLoadMode mapLoadMode = MapLoadMode::Clean)
		{
			if constexpr (scope.IsSaving())
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

	/// <summary>
	/// Serialize std::map with key.
	/// </summary>		
	template<typename TArchive, typename TKey, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	static bool Serialize(TArchive& archive, TKey&& key, std::map<TMapKey, TValue, TComparer, TAllocator>& cont,
		MapLoadMode mapLoadMode = MapLoadMode::Clean)
	{
		constexpr auto hasObjectWithKeySupport = can_serialize_object_with_key_v<TArchive, TKey>;
		static_assert(hasObjectWithKeySupport, "BitSerializer. The archive doesn't support serialize object with key on this level.");

		if constexpr (hasObjectWithKeySupport)
		{
			auto objectScope = archive.OpenObjectScope(key);
			if (objectScope)
				Detail::SerializeMapImpl(*objectScope, cont, mapLoadMode);
			return objectScope.has_value();
		}

		return false;
	}

	/// <summary>
	/// Serialize std::map.
	/// </summary>	
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	static void Serialize(TArchive& archive, std::map<TMapKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
	{
		constexpr auto hasObjectSupport = can_serialize_object_v<TArchive>;
		static_assert(hasObjectSupport, "BitSerializer. The archive doesn't support serialize object without key on this level.");

		if constexpr (hasObjectSupport)
		{
			auto objectScope = archive.OpenObjectScope();
			if (objectScope)
				Detail::SerializeMapImpl(*objectScope, cont, mapLoadMode);
		}
	}
}
