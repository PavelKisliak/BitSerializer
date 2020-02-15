/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <algorithm>
#include <type_traits>
#include "object_traits.h"
#include "archive_traits.h"
#include "base_object.h"
#include "../string_conversion.h"

namespace BitSerializer
{
	//-----------------------------------------------------------------------------
	// Serialize fundamental types
	//-----------------------------------------------------------------------------
	template <typename TArchive, typename TKey, typename TValue, std::enable_if_t<std::is_fundamental_v<TValue>, int> = 0>
	static bool Serialize(TArchive& archive, TKey&& key, TValue& value)
	{
		constexpr auto hasValueWithKeySupport = can_serialize_value_with_key_v<TArchive, TValue, TKey>;
		static_assert(hasValueWithKeySupport, "BitSerializer. The archive doesn't support serialize fundamental type with key on this level.");

		if constexpr (hasValueWithKeySupport) {
			return archive.SerializeValue(key, value);
		}
		return false;
	}

	template <typename TArchive, typename TValue, std::enable_if_t<std::is_fundamental_v<TValue>, int> = 0>
	static void Serialize(TArchive& archive, TValue& value)
	{
		constexpr auto hasValueTypeSupport = can_serialize_value_v<TArchive, TValue>;
		static_assert(hasValueTypeSupport, "BitSerializer. The archive doesn't support serialize fundamental type without key on this level.");

		if constexpr (hasValueTypeSupport) {
			archive.SerializeValue(value);
		}
	}

	//------------------------------------------------------------------------------
	// Serialize string types
	//------------------------------------------------------------------------------
	template <class TArchive, typename TKey, typename TSym, typename TAllocator>
	static bool Serialize(TArchive& archive, TKey&& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		constexpr auto hasStringWithKeySupport = can_serialize_value_with_key_v<TArchive,
			std::basic_string<TSym, std::char_traits<TSym>, TAllocator>, TKey>;
		static_assert(hasStringWithKeySupport, "BitSerializer. The archive doesn't support serialize string type with key on this level.");

		if constexpr (hasStringWithKeySupport) {
			return archive.SerializeValue(key, value);
		}
		return false;
	}

	template <class TArchive, typename TSym, typename TAllocator>
	static void Serialize(TArchive& archive, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		constexpr auto hasStringSupport = can_serialize_value_v<TArchive, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>;
		static_assert(hasStringSupport, "BitSerializer. The archive doesn't support serialize string type without key on this level.");

		if constexpr (hasStringSupport) {
			archive.SerializeValue(value);
		}
	}

	//-----------------------------------------------------------------------------
	// Serialize enum types
	//-----------------------------------------------------------------------------
	template <class TArchive, typename TKey, class TValue, std::enable_if_t<std::is_enum_v<TValue>, int> = 0>
	static bool Serialize(TArchive& archive, TKey&& key, TValue& value)
	{
		if constexpr (archive.IsLoading())
		{
			std::string str;
			auto result = Serialize(archive, key, str);
			Convert::Detail::To(str, value);
			return result;
		}
		else
		{
			auto str = Convert::ToString(value);
			return Serialize(archive, key, str);
		}
	}

	template <class TArchive, class TValue, std::enable_if_t<std::is_enum_v<TValue>, int> = 0>
	static void Serialize(TArchive& archive, TValue& value)
	{
		if constexpr (archive.IsLoading())
		{
			std::string str;
			Serialize(archive, str);
			Convert::Detail::To(str, value);
		}
		else
		{
			auto str = Convert::ToString(value);
			Serialize(archive, str);
		}
	}

	//------------------------------------------------------------------------------
	// Serialize classes
	//------------------------------------------------------------------------------
	template <class TArchive, typename TKey, typename TValue, std::enable_if_t<(std::is_class_v<TValue> || std::is_union_v<TValue>), int> = 0>
	static bool Serialize(TArchive& archive, TKey&& key, TValue& value)
	{
		// If you are trying to serialize one of known STD type, please make sure that you are included
		// required header with implementation (from "bitserializer/types/std/").
		constexpr auto isSerializableClass = is_serializable_class_v<TValue>;
		static_assert(isSerializableClass, "BitSerializer. The class must have Serialize() defined method internally or externally (in namespace BitSerializer).");

		if constexpr (isSerializableClass)
		{
			constexpr auto hasObjectWithKeySupport = can_serialize_object_with_key_v<TArchive, TKey>;
			static_assert(hasObjectWithKeySupport, "BitSerializer. The archive doesn't support serialize class with key on this level.");

			if constexpr (hasObjectWithKeySupport)
			{
				auto objectScope = archive.OpenObjectScope(key);
				if (objectScope)
					value.Serialize(*objectScope);
				return objectScope.has_value();
			}
		}
		return false;
	}

	template <class TArchive, class TValue, std::enable_if_t<(std::is_class_v<TValue> || std::is_union_v<TValue>), int> = 0>
	static void Serialize(TArchive& archive, TValue& value)
	{
		constexpr auto isSerializableClass = is_serializable_class_v<TValue>;
		// If you are trying to serialize one of known STD type, please make sure that you are included
		// required header with implementation (from "bitserializer/types/std/").
		static_assert(isSerializableClass, "BitSerializer. The class must have Serialize() defined method internally or externally (in namespace BitSerializer).");

		if constexpr (isSerializableClass)
		{
			constexpr auto hasObjectSupport = can_serialize_object_v<TArchive>;
			static_assert(hasObjectSupport, "BitSerializer. The archive doesn't support serialize class without key on this level.");

			if constexpr (hasObjectSupport) {
				auto objectScope = archive.OpenObjectScope();
				if (objectScope)
					value.Serialize(*objectScope);
			}
		}
	}

	/// <summary>
	/// Serializes the base class.
	/// </summary>
	template <typename TArchive, class TBase>
	static void Serialize(TArchive& archive, BaseObject<TBase>&& value)
	{
		constexpr auto isSerializableClass = is_serializable_class_v<TBase>;
		static_assert(isSerializableClass, "BitSerializer. The class must have Serialize() method internally or externally (in namespace BitSerializer).");

		if constexpr (isSerializableClass) {
			constexpr auto isObjectScope = is_object_scope_v<TArchive, typename TArchive::key_type>;
			static_assert(isObjectScope, "BitSerializer. The archive doesn't support serialize base class on this level.");

			if constexpr (isObjectScope) {
				value.Object.TBase::Serialize(archive);
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Serialize arrays
	//-----------------------------------------------------------------------------
	template<typename TArchive, typename TKey, typename TValue, size_t ArraySize>
	static bool Serialize(TArchive& archive, TKey&& key, TValue(&cont)[ArraySize])
	{
		constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
		static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

		if constexpr (hasArrayWithKeySupport)
		{
			auto arrayScope = archive.OpenArrayScope(key, ArraySize);
			if (arrayScope)
			{
				const auto size = archive.IsSaving() ? ArraySize : std::min(ArraySize, arrayScope->GetSize());
				for (size_t i = 0; i < size; i++) {
					Serialize(*arrayScope, cont[i]);
				}
			}
			return arrayScope.has_value();
		}
		return false;
	}

	template<typename TArchive, typename TValue, size_t ArraySize>
	static void Serialize(TArchive& archive, TValue(&cont)[ArraySize])
	{
		constexpr auto hasArraySupport = can_serialize_array_v<TArchive>;
		static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

		if constexpr (hasArraySupport)
		{
			auto arrayScope = archive.OpenArrayScope(ArraySize);
			if (arrayScope)
			{
				const auto size = archive.IsSaving() ? ArraySize : std::min(ArraySize, arrayScope->GetSize());
				for (size_t i = 0; i < size; i++) {
					Serialize(*arrayScope, cont[i]);
				}
			}
		}
	}

	/// <summary>
	/// The enumeration of available modes for loading maps.
	/// </summary>
	enum class MapLoadMode
	{
		Clean,			// Clean before load (default)
		OnlyExistKeys,	// Load only exists keys in map
		UpdateKeys,		// Load exists keys
	};

	namespace Detail
	{
		/// <summary>
		/// Generic function for serialization containers with key.
		/// </summary>
		/// <returns>Returns <c>true</c> when value successfully loaded</returns>
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
					if constexpr (archive.IsLoading() && is_resizeable_cont_v<TContainer>) {
						cont.resize(arrayScope->GetSize());
					}
					for (auto& elem : cont) {
						Serialize(*arrayScope, elem);
					}
				}
				return arrayScope.has_value();
			}
			return false;
		}

		/// <summary>
		/// Generic function for serialization containers.
		/// </summary>
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
					if constexpr (archive.IsLoading() && is_resizeable_cont_v<TContainer>) {
						cont.resize(arrayScope->GetSize());
					}
					for (auto& elem : cont) {
						Serialize(*arrayScope, elem);
					}
				}
			}
		}

		/// <summary>
		/// Generic function for serialization maps.
		/// </summary>
		template<typename TArchive, typename TMap>
		static void SerializeMapImpl(TArchive& scope, TMap& cont, MapLoadMode mapLoadMode = MapLoadMode::Clean)
		{
			using TMapKey = typename TMap::key_type;
			using TValue = typename TMap::mapped_type;
			
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

}	// namespace BitSerializer
