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
	template<typename TArchive, typename TContainer>
	static void SerializeContainer(TArchive& archive, const typename TArchive::key_type& key, TContainer& cont)
	{
		if constexpr (!can_serialize_array_with_key_v<TArchive>) {
			static_assert(false, "BitSerializer. The archive doesn't support serialize array with key on this level.");
		}
		else
		{
			const size_t size = GetContainerSize(cont);
			auto arrayScope = archive.OpenScopeForSerializeArray(key, size);
			if (arrayScope)
			{
				auto& scope = *arrayScope.get();
				if constexpr (archive.IsLoading())
				{
					if constexpr (is_resizeable_cont_v<TContainer>)
						cont.resize(scope.GetSize());
					else
						assert(size >= scope.GetSize());
				}
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
			const size_t size = GetContainerSize(cont);
			auto arrayScope = archive.OpenScopeForSerializeArray(size);
			if (arrayScope)
			{
				auto& scope = *arrayScope.get();
				if constexpr (archive.IsLoading())
				{
					if constexpr (is_resizeable_cont_v<TContainer>)
						cont.resize(scope.GetSize());
					else
						assert(size >= scope.GetSize());
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
// Serialize std::forward_list
//-----------------------------------------------------------------------------
template<typename TArchive, typename TValue, typename TAllocator>
inline void Serialize(TArchive& archive, const typename TArchive::key_type& key, std::forward_list<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, key, cont);
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
		auto arrayScope = archive.OpenScopeForSerializeArray(key, cont.size());
		if (arrayScope)
		{
			auto& scope = *arrayScope.get();
			if constexpr (archive.IsLoading()) {
				Detail::LoadSetImpl(*arrayScope.get(), cont);
			}
			else
			{
				for (const TValue& elem : cont) {
					Serialize(scope, const_cast<TValue&>(elem));
				}
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
		auto arrayScope = archive.OpenScopeForSerializeArray(cont.size());
		if (arrayScope)
		{
			auto& scope = *arrayScope.get();
			if constexpr (archive.IsLoading()) {
				Detail::LoadSetImpl(*arrayScope.get(), cont);
			}
			else
			{
				for (const TValue& elem : cont) {
					Serialize(scope, const_cast<TValue&>(elem));
				}
			}
		}
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
	template<typename TKey, typename TValue, typename TComparer, typename TAllocator>
	class MapSerializer
	{
	public:
		using value_type = std::map<TKey, TValue, TComparer, TAllocator>;

		MapSerializer(value_type& map, MapLoadMode mapLoadMode = MapLoadMode::Clean)
			: value(map)
			, mapLoadMode(mapLoadMode)
		{ }

		template <class TArchive>
		void Serialize(TArchive& archive)
		{
			if constexpr (archive.IsSaving())
			{
				for (auto& elem : value)
				{
					if constexpr (std::is_same_v<TKey, TArchive::key_type>)
						::BitSerializer::Serialize(archive, elem.first, elem.second);
					else
						::BitSerializer::Serialize(archive, Convert::To<TArchive::key_type>(elem.first), elem.second);
				}
			}
			else
			{
				auto loadSize = archive.GetSize();
				if (mapLoadMode == MapLoadMode::Clean)
					value.clear();
				auto hint = value.begin();
				for (size_t c = 0; c < loadSize; c++)
				{
					decltype(auto) archiveKey = archive.GetKeyByIndex(c);
					auto key = Convert::FromString<TKey>(archiveKey);
					switch (mapLoadMode)
					{
					case MapLoadMode::Clean:
						hint = value.emplace_hint(hint, std::move(key), TValue());
						::BitSerializer::Serialize(archive, archiveKey, hint->second);
						break;
					case MapLoadMode::OnlyExistKeys:
						hint = value.find(key);
						if (hint != value.end())
							::BitSerializer::Serialize(archive, archiveKey, hint->second);
						break;
					case MapLoadMode::UpdateKeys:
						::BitSerializer::Serialize(archive, archiveKey, value[key]);
						break;
					default:
						break;
					}
				}
			}
		}

		value_type& value;
		MapLoadMode mapLoadMode;
	};
}

template<typename TArchive, typename TKey, typename TValue, typename TComparer, typename TAllocator>
inline void Serialize(TArchive& archive, const typename TArchive::key_type& key, std::map<TKey, TValue, TComparer, TAllocator>& cont,
	MapLoadMode mapLoadMode = MapLoadMode::Clean)
{
	auto mapSerializer = Detail::MapSerializer<TKey, TValue, TComparer, TAllocator>(cont);
	Serialize(archive, key, mapSerializer);
}

template<typename TArchive, typename TKey, typename TValue, typename TComparer, typename TAllocator>
inline void Serialize(TArchive& archive, std::map<TKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
{
	auto mapSerializer = Detail::MapSerializer<TKey, TValue, TComparer, TAllocator>(cont);
	Serialize(archive, mapSerializer);
}

}	// namespace BitSerializer
