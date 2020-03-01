/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <vector>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
	namespace Detail
	{
		template<typename TArchive, typename TAllocator>
		static void SerializeVectorOfBooleansImpl(TArchive& scope, std::vector<bool, TAllocator>& cont)
		{
			if constexpr (scope.IsLoading()) {
				cont.resize(scope.GetSize());
			}
			bool value;
			const auto size = cont.size();
			for (size_t i = 0; i < size; i++)
			{
				if constexpr (scope.IsLoading()) {
					Serialize(scope, value);
					cont[i] = value;
				}
				else
				{
					value = cont[i];
					Serialize(scope, value);
				}
			}
		}
	}

	/// <summary>
	/// Serialize std::vector with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TValue, typename TAllocator>
	bool Serialize(TArchive& archive, TKey&& key, std::vector<TValue, TAllocator>& cont)
	{
		return Detail::SerializeContainer(archive, std::forward<TKey>(key), cont);
	}

	/// <summary>
	/// Serialize std::vector.
	/// </summary>
	template<typename TArchive, typename TValue, typename TAllocator>
	void Serialize(TArchive& archive, std::vector<TValue, TAllocator>& cont)
	{
		Detail::SerializeContainer(archive, cont);
	}

	/// <summary>
	/// Serialize specialization of std::vector for boolean type with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TAllocator>
	static bool Serialize(TArchive& archive, TKey&& key, std::vector<bool, TAllocator>& cont)
	{
		constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
		static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

		if constexpr (hasArrayWithKeySupport)
		{
			auto arrayScope = archive.OpenArrayScope(std::forward<TKey>(key), cont.size());
			if (arrayScope)
				Detail::SerializeVectorOfBooleansImpl(*arrayScope, cont);
			return arrayScope.has_value();
		}

		return false;
	}

	/// <summary>
	/// Serialize specialization of std::vector for boolean type.
	/// </summary>
	template<typename TArchive, typename TAllocator>
	static void Serialize(TArchive& archive, std::vector<bool, TAllocator>& cont)
	{
		constexpr auto hasArraySupport = can_serialize_array_v<TArchive>;
		static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

		if constexpr (hasArraySupport)
		{
			auto arrayScope = archive.OpenArrayScope(cont.size());
			if (arrayScope)
				Detail::SerializeVectorOfBooleansImpl(*arrayScope, cont);
		}
	}
}
