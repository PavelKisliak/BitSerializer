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
template <typename TArchive, typename TValue, std::enable_if_t<std::is_fundamental_v<TValue>, int> = 0>
inline void Serialize(TArchive& archive, const typename TArchive::key_type& key, TValue& value)
{
	if constexpr (!can_serialize_value_with_key_v<TArchive, TValue>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize fundamental type with key on this level.");
	}
	else
	{
		archive.SerializeValue(key, value);
	}
};

template <typename TArchive, typename TValue, std::enable_if_t<std::is_fundamental_v<TValue>, int> = 0>
inline void Serialize(TArchive& archive, TValue& value)
{
	if constexpr (!can_serialize_value_v<TArchive, TValue>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize fundamental type without key on this level.");
	}
	else
	{
		archive.SerializeValue(value);
	}
};

//------------------------------------------------------------------------------
// Serialize string types
//------------------------------------------------------------------------------
template <class TArchive, typename TSym, typename TAllocator>
inline void Serialize(TArchive& archive, const typename TArchive::key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
{
	archive.SerializeString(key, value);
};

template <class TArchive, typename TSym, typename TAllocator>
inline void Serialize(TArchive& archive, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
{
	archive.SerializeString(value);
};

//-----------------------------------------------------------------------------
// Serialize enum types
//-----------------------------------------------------------------------------
template <class TArchive, class TValue, std::enable_if_t<std::is_enum_v<TValue>, int> = 0>
inline void Serialize(TArchive& archive, const typename TArchive::key_type& key, TValue& value)
{
	if constexpr (archive.IsLoading())
	{
		std::string str;
		archive.SerializeString(key, str);
		Convert::Detail::FromString(str, value);
	}
	else
	{
		auto str = Convert::ToString(value);
		archive.SerializeString(key, str);
	}
};

template <class TArchive, class TValue, std::enable_if_t<std::is_enum_v<TValue>, int> = 0>
inline void Serialize(TArchive& archive, TValue& value)
{
	if constexpr (archive.IsLoading())
	{
		std::string str;
		archive.SerializeString(str);
		Convert::Detail::FromString(str, value);
	}
	else
	{
		auto str = Convert::ToString(value);
		archive.SerializeString(str);
	}
};

//------------------------------------------------------------------------------
// Serialize classes
//------------------------------------------------------------------------------
template <class TArchive, typename TValue, std::enable_if_t<std::is_class_v<TValue>, int> = 0>
inline void Serialize(TArchive& archive, const typename TArchive::key_type& key, TValue& value)
{
	if constexpr (!is_serializable_class_v<TValue>) {
		static_assert(false, "BitSerializer. Class should has method Serialize() internally or externally.");
	}
	else if constexpr (!can_serialize_object_with_key_v<TArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize class with key on this level.");
	}
	else
	{
		auto objectScope = archive.OpenScopeForSerializeObject(key);
		value.Serialize(*objectScope.get());
	}
};

template <class TArchive, class TValue, std::enable_if_t<std::is_class_v<TValue>, int> = 0>
inline void Serialize(TArchive& archive, TValue& value)
{
	if constexpr (!is_serializable_class_v<TValue>) {
		static_assert(false, "BitSerializer. Class should has method Serialize() internally or externally.");
	}
	else if constexpr (!can_serialize_object_v<TArchive>) {
		static_assert(false, "BitSerializer. The archive doesn't support serialize class without key on this level.");
	}
	else
	{
		auto objectScope = archive.OpenScopeForSerializeObject();
		value.Serialize(*objectScope.get());
	}
};

// Serialize base class
template <typename TArchive, class TBase, class TDerived>
inline void Serialize(TArchive& archive, BaseObjectImpl<TBase, TDerived>&& value)
{
	if constexpr (!is_serializable_class_v<TBase>) {
		static_assert(false, "BitSerializer. Class should has method Serialize() internally or externally.");
	}
	else
	{
		value.Object.TBase::Serialize(archive);
	}
};

//-----------------------------------------------------------------------------
// Serialize arrays
//-----------------------------------------------------------------------------
template<typename TArchive, typename TValue, size_t ArraySize>
static void Serialize(TArchive& archive, const typename TArchive::key_type& key, TValue(&cont)[ArraySize])
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
				assert(ArraySize == scope.GetSize());
				for (size_t i = 0; i < ArraySize; i++) {
					Serialize(scope, cont[i]);
				}
			}
		}
		else
		{
			auto arrayScope = archive.OpenScopeForSaveArray(key, ArraySize);
			auto& scope = *arrayScope.get();
			for (size_t i = 0; i < ArraySize; i++) {
				Serialize(scope, cont[i]);
			}
		}
	}
}

template<typename TArchive, typename TValue, size_t ArraySize>
static void Serialize(TArchive& archive, TValue(&cont)[ArraySize])
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
				assert(ArraySize == scope.GetSize());
				for (size_t i = 0; i < ArraySize; i++) {
					Serialize(scope, cont[i]);
				}
			}
		}
		else
		{
			auto arrayScope = archive.OpenScopeForSaveArray(ArraySize);
			auto& scope = *arrayScope.get();
			for (size_t i = 0; i < ArraySize; i++) {
				Serialize(scope, cont[i]);
			}
		}
	}
}

}	// namespace BitSerializer
