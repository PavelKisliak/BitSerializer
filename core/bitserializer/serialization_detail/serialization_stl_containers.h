/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>
#include <array>
#include <vector>
#include <deque>
#include <list>
#include <forward_list>
#include <set>
#include <map>
#include "object_traits.h"
#include "archive_traits.h"
#include "media_archive_base.h"
#include "../string_conversion.h"

namespace BitSerializer {

//-----------------------------------------------------------------------------
// Common implementation of serialization for most types of stl containers
//-----------------------------------------------------------------------------
namespace Detail
{
	template<typename TArchive, typename TKey, typename TContainer>
	static bool SerializeContainer(TArchive& archive, TKey&& key, TContainer& cont)
	{
		constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
		static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

		if constexpr (hasArrayWithKeySupport)
		{
			const size_t size = GetContainerSize(cont);
			auto arrayScope = archive.OpenArrayScope(key, size);
			if (arrayScope)
			{
				auto& scope = *arrayScope.get();
				if constexpr (archive.IsLoading())
				{
					if constexpr (is_resizeable_cont_v<TContainer>)
						cont.resize(scope.GetSize());
				}
				for (auto& elem : cont) {
					Serialize(scope, elem);
				}
			}
			return arrayScope != nullptr;
		}
        return false;
    }

	template<typename TArchive, typename TContainer>
	static void SerializeContainer(TArchive& archive, TContainer& cont)
	{
		constexpr auto hasArraySupport = can_serialize_array_v<TArchive>;
		static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

		if constexpr (hasArraySupport)
		{
			const size_t size = GetContainerSize(cont);
			auto arrayScope = archive.OpenArrayScope(size);
			if (arrayScope)
			{
				auto& scope = *arrayScope.get();
				if constexpr (archive.IsLoading())
				{
					if constexpr (is_resizeable_cont_v<TContainer>)
						cont.resize(scope.GetSize());
				}
				for (auto& elem : cont) {
					Serialize(scope, elem);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Serialize std::array
//-----------------------------------------------------------------------------
template<typename TArchive, typename TKey, typename TValue, size_t ArraySize>
inline bool Serialize(TArchive& archive, TKey&& key, std::array<TValue, ArraySize>& cont)
{
	return Detail::SerializeContainer(archive, key, cont);
}

template<typename TArchive, typename TValue, size_t ArraySize>
inline void Serialize(TArchive& archive, std::array<TValue, ArraySize>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::vector
//-----------------------------------------------------------------------------
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

template<typename TArchive, typename TKey, typename TValue, typename TAllocator>
inline bool Serialize(TArchive& archive, TKey&& key, std::vector<TValue, TAllocator>& cont)
{
	return Detail::SerializeContainer(archive, key, cont);
}

template<typename TArchive, typename TValue, typename TAllocator>
inline void Serialize(TArchive& archive, std::vector<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

template<typename TArchive, typename TKey, typename TAllocator>
static bool Serialize(TArchive& archive, TKey&& key, std::vector<bool, TAllocator>& cont)
{
	constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
	static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

	if constexpr (hasArrayWithKeySupport)
	{
		auto arrayScope = archive.OpenArrayScope(key, cont.size());
		if (arrayScope)
			Detail::SerializeVectorOfBooleansImpl(*arrayScope.get(), cont);
		return arrayScope != nullptr;
	}

	return false;
}

template<typename TArchive, typename TAllocator>
static void Serialize(TArchive& archive, std::vector<bool, TAllocator>& cont)
{
	constexpr auto hasArraySupport = can_serialize_array_v<TArchive>;
	static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

	if constexpr (hasArraySupport)
	{
		auto arrayScope = archive.OpenArrayScope(cont.size());
		if (arrayScope)
			Detail::SerializeVectorOfBooleansImpl(*arrayScope.get(), cont);
	}
}

//-----------------------------------------------------------------------------
// Serialize std::deque
//-----------------------------------------------------------------------------
template<typename TArchive, typename TKey, typename TValue, typename TAllocator>
inline bool Serialize(TArchive& archive, TKey&& key, std::deque<TValue, TAllocator>& cont)
{
	return Detail::SerializeContainer(archive, key, cont);
}

template<typename TArchive, typename TValue, typename TAllocator>
inline void Serialize(TArchive& archive, std::deque<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::list
//-----------------------------------------------------------------------------
template<typename TArchive, typename TKey, typename TValue, typename TAllocator>
inline bool Serialize(TArchive& archive, TKey&& key, std::list<TValue, TAllocator>& cont)
{
	return Detail::SerializeContainer(archive, key, cont);
}

template<typename TArchive, typename TValue, typename TAllocator>
inline void Serialize(TArchive& archive, std::list<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::forward_list
//-----------------------------------------------------------------------------
template<typename TArchive, typename TKey, typename TValue, typename TAllocator>
inline bool Serialize(TArchive& archive, TKey&& key, std::forward_list<TValue, TAllocator>& cont)
{
	return Detail::SerializeContainer(archive, key, cont);
}

template<typename TArchive, typename TValue, typename TAllocator>
inline void Serialize(TArchive& archive, std::forward_list<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::set
//-----------------------------------------------------------------------------
namespace Detail
{
	template<typename TArchive, typename TValue, typename TAllocator>
	static void SerializeSetImpl(TArchive& scope, std::set<TValue, TAllocator>& cont)
	{
		if constexpr (scope.IsLoading())
		{
			auto contSize = scope.GetSize();
			cont.clear();
			auto hint = cont.begin();
			for (size_t c = 0; c<contSize; c++)
			{
				TValue value;
				Serialize(scope, value);
				hint = cont.insert(hint, std::move(value));
			}
		}
		else
		{
			for (const TValue& elem : cont) {
				Serialize(scope, const_cast<TValue&>(elem));
			}
		}
	}
}

template<typename TArchive, typename TKey, typename TValue, typename TAllocator>
static bool Serialize(TArchive& archive, TKey&& key, std::set<TValue, TAllocator>& cont)
{
	constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
	static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

	if constexpr (hasArrayWithKeySupport)
	{
		auto arrayScope = archive.OpenArrayScope(key, cont.size());
		if (arrayScope)
			Detail::SerializeSetImpl(*arrayScope.get(), cont);
		return arrayScope != nullptr;
	}

	return false;
}

template<typename TArchive, typename TValue, typename TAllocator>
static void Serialize(TArchive& archive, std::set<TValue, TAllocator>& cont)
{
	constexpr auto hasArraySupport = can_serialize_array_v<TArchive>;
	static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

	if constexpr (hasArraySupport)
	{
		auto arrayScope = archive.OpenArrayScope(cont.size());
		if (arrayScope)
			Detail::SerializeSetImpl(*arrayScope.get(), cont);
	}
}

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
			for (size_t c = 0; c < loadSize; c++)
			{
				decltype(auto) archiveKey = scope.GetKeyByIndex(c);
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
			Detail::SerializeMapImpl(*objectScope.get(), cont, mapLoadMode);
		return objectScope != nullptr;
	}

    return false;
}

template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
static void Serialize(TArchive& archive, std::map<TMapKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
{
	constexpr auto hasObjectSupport = can_serialize_object_v<TArchive>;
	static_assert(hasObjectSupport, "BitSerializer. The archive doesn't support serialize object without key on this level.");

	if constexpr (hasObjectSupport)
	{
		auto objectScope = archive.OpenObjectScope();
		if (objectScope)
			Detail::SerializeMapImpl(*objectScope.get(), cont, mapLoadMode);
	}
}

//-----------------------------------------------------------------------------
// Serialize std::multimap
//-----------------------------------------------------------------------------
namespace Detail
{
	template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
	static void SerializeMultimapImpl(TArchive& scope, std::multimap<TMapKey, TValue, TComparer, TAllocator>& cont)
	{
		using pair_type = typename std::multimap<TMapKey, TValue, TComparer, TAllocator>::value_type;
		if constexpr (scope.IsLoading())
		{
			auto loadSize = scope.GetSize();
			cont.clear();
			auto hint = cont.begin();
			for (size_t c = 0; c < loadSize; c++)
			{
				pair_type pair;
				Serialize(scope, pair);
				hint = cont.emplace_hint(hint, std::move(pair));
			}
		}
		else
		{
			for (auto& elem : cont) {
				Serialize(scope, elem);
			}
		}
	}
}

template<typename TArchive, typename TKey, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
static bool Serialize(TArchive& archive, TKey&& key, std::multimap<TMapKey, TValue, TComparer, TAllocator>& cont)
{
	constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
	static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

	if constexpr (hasArrayWithKeySupport)
	{
		auto arrayScope = archive.OpenArrayScope(key, cont.size());
		if (arrayScope)
			Detail::SerializeMultimapImpl(*arrayScope.get(), cont);
		return arrayScope != nullptr;
	}

	return false;
}

template<typename TArchive, typename TMapKey, typename TValue, typename TComparer, typename TAllocator>
static void Serialize(TArchive& archive, std::multimap<TMapKey, TValue, TComparer, TAllocator>& cont)
{
	constexpr auto hasArraySupport = can_serialize_array_v<TArchive>;
	static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

	if constexpr (hasArraySupport)
	{
		auto arrayScope = archive.OpenArrayScope(cont.size());
		if (arrayScope)
			Detail::SerializeMultimapImpl(*arrayScope.get(), cont);
	}
}


}	// namespace BitSerializer
