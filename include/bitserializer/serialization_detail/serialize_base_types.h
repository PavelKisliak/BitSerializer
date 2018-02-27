/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>
#include "object_traits.h"
#include "archive_traits.h"
#include "base_object.h"
#include "media_archive_base.h"
#include "..\string_conversion.h"

namespace BitSerializer {

//-----------------------------------------------------------------------------
// Serialize fundamental types
//-----------------------------------------------------------------------------
template <typename TMediaArchive, typename TValue, std::enable_if_t<std::is_fundamental_v<TValue>, int> = 0>
void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, TValue& value)
{
	if constexpr (!can_serialize_value_with_key_v<TMediaArchive, TValue>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize fundamental type with key (maybe this is array level).");
	}
	else
	{
		switch (archive.GetState())
		{
		case SerializeState::Load:
			archive.LoadValue(key, value);
			break;
		case SerializeState::Save:
			archive.SaveValue(key, value);
			break;
		default:
			break;
		}
	}
};

template <typename TMediaArchive, typename TValue, std::enable_if_t<std::is_fundamental_v<TValue>, int> = 0>
void Serialize(TMediaArchive& archive, TValue& value)
{
	if constexpr (!can_serialize_value_v<TMediaArchive, TValue>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize fundamental type without key (possible only for the current level).");
	}
	else
	{
		switch (archive.GetState())
		{
		case SerializeState::Load:
			archive.LoadValue(value);
			break;
		case SerializeState::Save:
			archive.SaveValue(value);
			break;
		default:
			break;
		}
	}
};

//------------------------------------------------------------------------------
// Serialize string types
//------------------------------------------------------------------------------
template <class TMediaArchive, typename TSym, typename TAllocator>
static void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
{
	switch (archive.GetState())
	{
	case SerializeState::Load:
		archive.LoadString(key, value);
		break;
	case SerializeState::Save:
		archive.SaveString(key, value);
		break;
	default:
		break;
	}
};

template <class TMediaArchive, typename TSym, typename TAllocator>
static void Serialize(TMediaArchive& archive, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
{
	switch (archive.GetState())
	{
	case SerializeState::Load:
		archive.LoadString(value);
		break;
	case SerializeState::Save:
		archive.SaveString(value);
		break;
	default:
		break;
	}
};

//-----------------------------------------------------------------------------
// Serialize enum types
//-----------------------------------------------------------------------------
template <class TMediaArchive, class TValue, std::enable_if_t<std::is_enum_v<TValue>, int> = 0>
void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, TValue& value)
{
	switch (archive.GetState())
	{
	case SerializeState::Load:
	{
		std::string str;
		archive.LoadString(key, str);
		Convert::Detail::FromString(str, value);
		break;
	}
	case SerializeState::Save:
	{
		auto str = Convert::ToWString(value);
		archive.SaveString(key, str);
		break;
	}
	default:
		break;
	}
};

template <class TMediaArchive, class TValue, std::enable_if_t<std::is_enum_v<TValue>, int> = 0>
void Serialize(TMediaArchive& archive, TValue& value)
{
	switch (archive.GetState())
	{
	case SerializeState::Load:
	{
		std::string str;
		archive.LoadString(str);
		Convert::Detail::FromString(str, value);
		break;
	}
	case SerializeState::Save:
	{
		auto str = Convert::ToWString(value);
		archive.SaveString(str);
		break;
	}
	default:
		break;
	}
};

//------------------------------------------------------------------------------
// Serialize classes
//------------------------------------------------------------------------------
template <class TMediaArchive, typename TValue, std::enable_if_t<std::is_class_v<TValue>, int> = 0>
void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, TValue& value)
{
	if constexpr (!is_serializable_class_v<TValue>) {
		static_assert(false, "BitSerializer. Class should has method Serialize() internally or externally.");
	}
	else if constexpr (!can_serialize_object_with_key_v<TMediaArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize class with key (maybe this is array level).");
	}
	else
	{
		switch (archive.GetState())
		{
		case SerializeState::Load:
		{
			auto objectScope = archive.OpenScopeForLoadObject(key);
			if (objectScope)
				value.Serialize(*objectScope.get());
			break;
		}
		case SerializeState::Save:
		{
			auto objectScope = archive.OpenScopeForSaveObject(key);
			value.Serialize(*objectScope.get());
			break;
		}
		default:
			break;
		}
	}
};

template <class TMediaArchive, class TValue, std::enable_if_t<std::is_class_v<TValue>, int> = 0>
void Serialize(TMediaArchive& archive, TValue& value)
{
	if constexpr (!is_serializable_class_v<TValue>) {
		static_assert(false, "BitSerializer. Class should has method Serialize() internally or externally.");
	}
	else if constexpr (!can_serialize_object_v<TMediaArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize class without key (possible only for the current level).");
	}
	else
	{
		switch (archive.GetState())
		{
		case SerializeState::Load:
		{
			auto objectScope = archive.OpenScopeForLoadObject();
			if (objectScope)
				value.Serialize(*objectScope.get());
			break;
		}
		case SerializeState::Save:
		{
			auto objectScope = archive.OpenScopeForSaveObject();
			value.Serialize(*objectScope.get());
			break;
		}
		default:
			break;
		}
	}
};

// Serialize base class
template <typename TMediaArchive, class TBase, class TDerived>
void Serialize(TMediaArchive& archive, BaseObjectImpl<TBase, TDerived>&& value)
{
	if constexpr (!is_serializable_class_v<TBase>) {
		static_assert(false, "BitSerializer. Class should has method Serialize() internally or externally.");
	}
	else
	{
		switch (archive.GetState())
		{
		case SerializeState::Load:
		{
			value.Object.TBase::Serialize(archive);
			break;
		}
		case SerializeState::Save:
			value.Object.TBase::Serialize(archive);
			break;
		default:
			break;
		}
	}
};

//-----------------------------------------------------------------------------
// Serialize arrays
//-----------------------------------------------------------------------------
template<typename TMediaArchive, typename TValue, size_t ArraySize>
void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, TValue(&cont)[ArraySize])
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
				assert(ArraySize == scope.GetSize());
				for (size_t i = 0; i < ArraySize; i++) {
					Serialize(scope, cont[i]);
				}
			}
			break;
		}
		case SerializeState::Save:
		{
			auto arrayScope = archive.OpenScopeForSaveArray(key, ArraySize);
			auto& scope = *arrayScope.get();
			for (size_t i = 0; i < ArraySize; i++) {
				Serialize(scope, cont[i]);
			}
			break;
		}
		default:
			break;
		}
	}
}

template<typename TMediaArchive, typename TValue, size_t ArraySize>
void Serialize(TMediaArchive& archive, TValue(&cont)[ArraySize])
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
				auto& scope = arrayScope.value();
				assert(ArraySize == scope.GetSize());
				for (size_t i = 0; i < ArraySize; i++) {
					Serialize(scope, cont[i]);
				}
			}
			break;
		}
		case SerializeState::Save:
		{
			auto scope = archive.OpenScopeForSaveArray(ArraySize);
			for (size_t i = 0; i < ArraySize; i++) {
				Serialize(scope, cont[i]);
			}
			break;
		}
		default:
			break;
		}
	}
}

}	// namespace BitSerializer
