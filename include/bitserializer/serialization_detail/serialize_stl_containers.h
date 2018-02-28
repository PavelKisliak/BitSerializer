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
#include <set>
#include <map>
#include "object_traits.h"
#include "archive_traits.h"
#include "media_archive_base.h"
#include "..\string_conversion.h"

namespace BitSerializer {

//-----------------------------------------------------------------------------
// Common implementation of serialization for most types of stl containers
//-----------------------------------------------------------------------------
namespace Detail
{
	template<typename TArchive, typename TContainer>
	static void SerializeContainer(TArchive& archive, const typename TArchive::key_type& key, TContainer& cont)
	{
		if constexpr (!can_serialize_array_with_key_v<TArchive>) {
			static_assert(false, "BitSerializer. The archive doesn't support serialize array with key on this level.");
		}
		else
		{
			if constexpr (archive.IsLoading())
			{
				auto arrayScope = archive.OpenScopeForLoadArray(key);
				if (arrayScope)
				{
					auto& scope = *arrayScope.get();
					if constexpr (is_resizeable_cont<TContainer>::value)
						cont.resize(scope.GetSize());
					else
						assert(cont.size() == scope.GetSize());
					for (auto& elem : cont) {
						Serialize(scope, elem);
					}
				}
			}
			else
			{
				auto arrayScope = archive.OpenScopeForSaveArray(key, cont.size());
				auto& scope = *arrayScope.get();
				for (auto& elem : cont) {
					Serialize(scope, elem);
				}
			}
		}
	}

	template<typename TArchive, typename TContainer>
	static void SerializeContainer(TArchive& archive, TContainer& cont)
	{
		if constexpr (!can_serialize_array_v<TArchive>) {
			static_assert(false, "BitSerializer. The archive doesn't support serialize array without key on this level.");
		}
		else
		{
			if constexpr (archive.IsLoading())
			{
				auto arrayScope = archive.OpenScopeForLoadArray();
				if (arrayScope)
				{
					auto& scope = *arrayScope.get();
					if constexpr (is_resizeable_cont<TContainer>::value)
						cont.resize(scope.GetSize());
					else
						assert(cont.size() == scope.GetSize());
					for (auto& elem : cont) {
						Serialize(scope, elem);
					}
				}
			}
			else
			{
				auto arrayScope = archive.OpenScopeForSaveArray(cont.size());
				auto& scope = *arrayScope.get();
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
template<typename TArchive, typename TValue, size_t ArraySize>
inline void Serialize(TArchive& archive, const typename TArchive::key_type& key, std::array<TValue, ArraySize>& cont)
{
	Detail::SerializeContainer(archive, key, cont);
}

template<typename TArchive, typename TValue, size_t ArraySize>
inline void Serialize(TArchive& archive, std::array<TValue, ArraySize>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::vector
//-----------------------------------------------------------------------------
template<typename TArchive, typename TValue, typename TAllocator>
inline void Serialize(TArchive& archive, const typename TArchive::key_type& key, std::vector<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, key, cont);
}

template<typename TArchive, typename TValue, typename TAllocator>
inline void Serialize(TArchive& archive, std::vector<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::deque
//-----------------------------------------------------------------------------
template<typename TArchive, typename TValue, typename TAllocator>
inline void Serialize(TArchive& archive, const typename TArchive::key_type& key, std::deque<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, key, cont);
}

template<typename TArchive, typename TValue, typename TAllocator>
inline void Serialize(TArchive& archive, std::deque<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::list
//-----------------------------------------------------------------------------
template<typename TArchive, typename TValue, typename TAllocator>
inline void Serialize(TArchive& archive, const typename TArchive::key_type& key, std::list<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, key, cont);
}

template<typename TArchive, typename TValue, typename TAllocator>
inline void Serialize(TArchive& archive, std::list<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::set
//-----------------------------------------------------------------------------
namespace Detail
{
	template<typename TArchive, typename TValue, typename TAllocator>
	inline void LoadSetImpl(TArchive& scope, std::set<TValue, TAllocator>& cont)
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
}

template<typename TArchive, typename TValue, typename TAllocator>
static void Serialize(TArchive& archive, const typename TArchive::key_type& key, std::set<TValue, TAllocator>& cont)
{
	if constexpr (!can_serialize_array_with_key_v<TArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize array with key on this level.");
	}
	else
	{
		if constexpr (archive.IsLoading())
		{
			auto arrayScope = archive.OpenScopeForLoadArray(key);
			if (arrayScope)
				Detail::LoadSetImpl(*arrayScope.get(), cont);
		}
		else
		{
			auto arrayScope = archive.OpenScopeForSaveArray(key, cont.size());
			auto& scope = *arrayScope.get();
			for (const TValue& elem : cont) {
				Serialize(scope, const_cast<TValue&>(elem));
			}
		}
	}
}

template<typename TArchive, typename TValue, typename TAllocator>
static void Serialize(TArchive& archive, std::set<TValue, TAllocator>& cont)
{
	if constexpr (!can_serialize_array_v<TArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize array without key on this level.");
	}
	else
	{
		if constexpr (archive.IsLoading())
		{
			auto arrayScope = archive.OpenScopeForLoadArray();
			if (arrayScope)
				Detail::LoadSetImpl(*arrayScope.get(), cont);
		}
		else
		{
			auto arrayScope = archive.OpenScopeForSaveArray(cont.size());
			auto& scope = *arrayScope.get();
			for (const TValue& elem : cont) {
				Serialize(scope, const_cast<TValue&>(elem));
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Serialize std::map
//-----------------------------------------------------------------------------
enum class MapLoadMode
{
	Clean,			// Clean before load
	OnlyExistKeys,	// Load only exists keys in map
	UpdateKeys,		// Load exists keys
};

namespace Detail
{
	template<typename TArchive, typename TKey, typename TValue, typename TComparer, typename TAllocator>
	inline void LoadMapImpl(TArchive& scope, std::map<TKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode)
	{
		using mapType = std::map<TKey, TValue, TComparer, TAllocator>;
		auto contSize = scope.GetSize();
		if (mapLoadMode == MapLoadMode::Clean)
			cont.clear();
		auto hint = cont.begin();
		for (size_t c = 0; c < contSize; c++)
		{
			decltype(auto) archiveKey = scope.GetKeyByIndex(c);
			auto key = Convert::FromString<TKey>(archiveKey);
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

template<typename TArchive, typename TKey, typename TValue, typename TComparer, typename TAllocator>
static void Serialize(TArchive& archive, const typename TArchive::key_type& key, std::map<TKey, TValue, TComparer, TAllocator>& cont,
	MapLoadMode mapLoadMode = MapLoadMode::Clean)
{
	if constexpr (!can_serialize_object_with_key_v<TArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize class with key on this level.");
	}
	else
	{
		if constexpr (archive.IsLoading())
		{
			auto objectScope = archive.OpenScopeForSerializeObject(key);
			if (objectScope)
				Detail::LoadMapImpl(*objectScope.get(), cont, mapLoadMode);
		}
		else
		{
			auto objectScope = archive.OpenScopeForSerializeObject(key);
			auto& scope = *objectScope.get();
			for (auto& elem : cont)
			{
				if constexpr (std::is_same_v<TKey, TArchive::key_type>)
					Serialize(scope, elem.first, elem.second);
				else
					Serialize(scope, Convert::To<TArchive::key_type>(elem.first), elem.second);
			}
		}
	}
}

template<typename TArchive, typename TKey, typename TValue, typename TComparer, typename TAllocator>
static void Serialize(TArchive& archive, std::map<TKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
{
	if constexpr (!can_serialize_object_v<TArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize class without key on this level.");
	}
	else
	{
		if constexpr (archive.IsLoading())
		{
			auto objectScope = archive.OpenScopeForSerializeObject();
			if (objectScope)
				Detail::LoadMapImpl(*objectScope.get(), cont, mapLoadMode);
		}
		else
		{
			auto objectScope = archive.OpenScopeForSerializeObject();
			auto& scope = *objectScope.get();
			for (auto& elem : cont)
			{
				if constexpr (std::is_same_v<TKey, TArchive::key_type>)
					Serialize(scope, elem.first, elem.second);
				else
					Serialize(scope, Convert::To<TArchive::key_type>(elem.first), elem.second);
			}
		}
	}
}

}	// namespace BitSerializer
