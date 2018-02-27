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
	template<typename TMediaArchive, typename TContainer>
	void SerializeContainer(TMediaArchive& archive, const typename TMediaArchive::key_type& key, TContainer& cont)
	{
		if constexpr (!can_serialize_array_with_key_v<TMediaArchive>) {
			static_assert(false, "BitSerializer. The archive doesn't support serialize array with key (maybe this is array level).");
		}
		else
		{
			switch (archive.GetState())
			{
			case SerializeState::Load:
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
				break;
			}
			case SerializeState::Save:
			{
				auto arrayScope = archive.OpenScopeForSaveArray(key, cont.size());
				auto& scope = *arrayScope.get();
				for (auto& elem : cont) {
					Serialize(scope, elem);
				}
				break;
			}
			default:
				break;
			}
		}
	}

	template<typename TMediaArchive, typename TContainer>
	void SerializeContainer(TMediaArchive& archive, TContainer& cont)
	{
		if constexpr (!can_serialize_array_v<TMediaArchive>) {
			static_assert(false, "BitSerializer. The archive doesn't support serialize array without key (possible only for the current level).");
		}
		else
		{
			switch (archive.GetState())
			{
			case SerializeState::Load:
			{
				auto arrayScope = archive.OpenScopeForLoadArray();
				if (arrayScope)
				{
					auto& scope = *arrayScope.get();
					if (is_resizeable_cont<TContainer>::value)
						cont.resize(scope.GetSize());
					else
						assert(cont.size() == scope.GetSize());
					for (auto& elem : cont) {
						Serialize(scope, elem);
					}
				}
				break;
			}
			case SerializeState::Save:
			{
				auto arrayScope = archive.OpenScopeForSaveArray(cont.size());
				auto& scope = *arrayScope.get();
				for (auto& elem : cont) {
					Serialize(scope, elem);
				}
				break;
			}
			default:
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Serialize std::array
//-----------------------------------------------------------------------------
template<typename TMediaArchive, typename TValue, size_t ArraySize>
inline void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, std::array<TValue, ArraySize>& cont)
{
	Detail::SerializeContainer(archive, key, cont);
}

template<typename TMediaArchive, typename TValue, size_t ArraySize>
inline void Serialize(TMediaArchive& archive, std::array<TValue, ArraySize>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::vector
//-----------------------------------------------------------------------------
template<typename TMediaArchive, typename TValue, typename TAllocator>
inline void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, std::vector<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, key, cont);
}

template<typename TMediaArchive, typename TValue, typename TAllocator>
inline void Serialize(TMediaArchive& archive, std::vector<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::deque
//-----------------------------------------------------------------------------
template<typename TMediaArchive, typename TValue, typename TAllocator>
inline void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, std::deque<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, key, cont);
}

template<typename TMediaArchive, typename TValue, typename TAllocator>
inline void Serialize(TMediaArchive& archive, std::deque<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::list
//-----------------------------------------------------------------------------
template<typename TMediaArchive, typename TValue, typename TAllocator>
inline void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, std::list<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, key, cont);
}

template<typename TMediaArchive, typename TValue, typename TAllocator>
inline void Serialize(TMediaArchive& archive, std::list<TValue, TAllocator>& cont)
{
	Detail::SerializeContainer(archive, cont);
}

//-----------------------------------------------------------------------------
// Serialize std::set
//-----------------------------------------------------------------------------
namespace Detail
{
	template<typename TMediaArchive, typename TValue, typename TAllocator>
	inline void LoadSetImpl(TMediaArchive& scope, std::set<TValue, TAllocator>& cont)
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

template<typename TMediaArchive, typename TValue, typename TAllocator>
void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, std::set<TValue, TAllocator>& cont)
{
	if constexpr (!can_serialize_array_with_key_v<TMediaArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize array with key (maybe this is array level).");
	}
	else
	{
		switch (archive.GetState())
		{
		case SerializeState::Load:
		{
			auto arrayScope = archive.OpenScopeForLoadArray(key);
			if (arrayScope)
				Detail::LoadSetImpl(*arrayScope.get(), cont);
			break;
		}
		case SerializeState::Save:
		{
			auto arrayScope = archive.OpenScopeForSaveArray(key, cont.size());
			auto& scope = *arrayScope.get();
			for (const TValue& elem : cont) {
				Serialize(scope, const_cast<TValue&>(elem));
			}
			break;
		}
		default:
			break;
		}
	}
}

template<typename TMediaArchive, typename TValue, typename TAllocator>
void Serialize(TMediaArchive& archive, std::set<TValue, TAllocator>& cont)
{
	if constexpr (!can_serialize_array_v<TMediaArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize array without key (possible only for the current level).");
	}
	else
	{
		switch (archive.GetState())
		{
		case SerializeState::Load:
		{
			auto arrayScope = archive.OpenScopeForLoadArray();
			if (arrayScope)
				Detail::LoadSetImpl(arrayScope.value(), cont);
			break;
		}
		case SerializeState::Save:
		{
			auto arrayScope = archive.OpenScopeForSaveArray(cont.size());
			for (const TValue& elem : cont) {
				Serialize(arrayScope, const_cast<TValue&>(elem));
			}
			break;
		}
		default:
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Serialize std::pair
//-----------------------------------------------------------------------------
namespace Detail
{
	template<typename TFirst, typename TSecond>
	class PairSerializer
	{
	public:
		using value_type = std::pair<TFirst, TSecond>;

		PairSerializer(value_type& pair)
			: value(pair)
		{ }

		template <class TMediaArchive>
		inline void Serialize(TMediaArchive& archive)
		{
			using noConstKeyType = std::remove_const_t<value_type::first_type>;
			::BitSerializer::Serialize(archive, U("key"), const_cast<noConstKeyType&>(value.first));
			::BitSerializer::Serialize(archive, U("value"), value.second);
		}

		value_type& value;
	};
}

template<typename TMediaArchive, typename TFirst, typename TSecond>
void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, std::pair<TFirst, TSecond>& pair)
{
	Serialize(archive, key, Detail::PairSerializer<TFirst, TSecond>(pair));
}

template<typename TMediaArchive, typename TFirst, typename TSecond>
void Serialize(TMediaArchive& archive, std::pair<TFirst, TSecond>& pair)
{
	Serialize(archive, Detail::PairSerializer<TFirst, TSecond>(pair));
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
	template<typename TMediaArchive, typename TKey, typename TValue, typename TComparer, typename TAllocator>
	inline void LoadMapImpl(TMediaArchive& scope, std::map<TKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode)
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

template<typename TMediaArchive, typename TKey, typename TValue, typename TComparer, typename TAllocator>
void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, std::map<TKey, TValue, TComparer, TAllocator>& cont,
	MapLoadMode mapLoadMode = MapLoadMode::Clean)
{
	if constexpr (!can_serialize_array_with_key_v<TMediaArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize array with key (maybe this is array level).");
	}
	else
	{
		switch (archive.GetState())
		{
		case SerializeState::Load:
		{
			auto objectScope = archive.OpenScopeForLoadObject(key);
			if (objectScope)
				Detail::LoadMapImpl(*objectScope.get(), cont, mapLoadMode);
			break;
		}
		case SerializeState::Save:
		{
			auto objectScope = archive.OpenScopeForSaveObject(key);
			for (auto& elem : cont)
			{
				if constexpr (std::is_same_v<TKey, TMediaArchive::key_type>)
					Serialize(*objectScope.get(), elem.first, elem.second);
				else
					Serialize(*objectScope.get(), Convert::To<TMediaArchive::key_type>(elem.first), elem.second);
			}
			break;
		}
		default:
			break;
		}
	}
}

template<typename TMediaArchive, typename TKey, typename TValue, typename TComparer, typename TAllocator>
void Serialize(TMediaArchive& archive, std::map<TKey, TValue, TComparer, TAllocator>& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
{
	if constexpr (!can_serialize_array_v<TMediaArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize array without key (possible only for the current level).");
	}
	else
	{
		switch (archive.GetState())
		{
		case SerializeState::Load:
		{
			auto objectScope = archive.OpenScopeForLoadObject();
			if (objectScope)
				Detail::LoadMapImpl(*objectScope.get(), cont, mapLoadMode);
			break;
		}
		case SerializeState::Save:
		{
			auto objectScope = archive.OpenScopeForSaveObject();
			auto& scope = *objectScope.get();
			for (auto& elem : cont)
			{
				if constexpr (std::is_same_v<TKey, TMediaArchive::key_type>)
					Serialize(*objectScope.get(), elem.first, elem.second);
				else
					Serialize(*objectScope.get(), Convert::To<TMediaArchive::key_type>(elem.first), elem.second);
			}
			break;
		}
		default:
			break;
		}
	}
}

}	// namespace BitSerializer
