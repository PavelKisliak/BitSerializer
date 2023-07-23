/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <algorithm>
#include <type_traits>
#include "bitserializer/convert.h"

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
				if (mapLoadMode == MapLoadMode::Clean)
					cont.clear();
				auto hint = cont.begin();
				auto endIt = scope.cend();
				for (auto it = scope.cbegin(); it != endIt; ++it)
				{
					// Convert archive key to key type of target map
					decltype(auto) archiveKey = *it;
					TMapKey key;
					if constexpr (std::is_convertible_v<TMapKey, typename TArchive::key_type>) {
						key = archiveKey;
					}
					else
					{
						try {
							key = Convert::To<TMapKey>(archiveKey);
						}
						catch (const std::invalid_argument&)
						{
							if (scope.GetOptions().mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
							{
								throw SerializationException(SerializationErrorCode::MismatchedTypes,
									"The value being loaded cannot be converted to target map key");
							}
							continue;
						}
						catch (const std::out_of_range&)
						{
							if (scope.GetOptions().overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
							{
								throw SerializationException(SerializationErrorCode::Overflow,
									"The size of target map key is not sufficient to store value from the parsed string");
							}
							continue;
						}
						catch (...) {
							throw SerializationException(SerializationErrorCode::ParsingError, "Unknown error when parsing string");
						}
					}

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
			}
		}
	}
}
