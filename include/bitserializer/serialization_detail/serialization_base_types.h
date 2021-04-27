/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <algorithm>
#include <type_traits>
#include "object_traits.h"
#include "archive_traits.h"
#include "bitserializer/convert.h"

namespace BitSerializer
{
	//-----------------------------------------------------------------------------
	// Serialize fundamental types
	//-----------------------------------------------------------------------------
	template <typename TArchive, typename TKey, typename TValue, std::enable_if_t<std::is_fundamental_v<TValue>, int> = 0>
	bool Serialize(TArchive& archive, TKey&& key, TValue& value)
	{
		constexpr auto hasValueWithKeySupport = can_serialize_value_with_key_v<TArchive, TValue, TKey>;
		static_assert(hasValueWithKeySupport, "BitSerializer. The archive doesn't support serialize fundamental type with key on this level.");

		if constexpr (hasValueWithKeySupport) {
 			return archive.SerializeValue(std::forward<TKey>(key), value);
		}
		return false;
	}

	template <typename TArchive, typename TValue, std::enable_if_t<std::is_fundamental_v<TValue>, int> = 0>
	void Serialize(TArchive& archive, TValue& value)
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
	bool Serialize(TArchive& archive, TKey&& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		constexpr auto hasStringWithKeySupport = can_serialize_value_with_key_v<TArchive,
			std::basic_string<TSym, std::char_traits<TSym>, TAllocator>, TKey>;
		static_assert(hasStringWithKeySupport, "BitSerializer. The archive doesn't support serialize string type with key on this level.");

		if constexpr (hasStringWithKeySupport) {
			return archive.SerializeValue(std::forward<TKey>(key), value);
		}
		return false;
	}

	template <class TArchive, typename TSym, typename TAllocator>
	void Serialize(TArchive& archive, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
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
	bool Serialize(TArchive& archive, TKey&& key, TValue& value)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::string str;
			auto result = Serialize(archive, std::forward<TKey>(key), str);
			Convert::Detail::To(str, value);
			return result;
		}
		else
		{
			auto str = Convert::ToString(value);
			return Serialize(archive, std::forward<TKey>(key), str);
		}
	}

	template <class TArchive, class TValue, std::enable_if_t<std::is_enum_v<TValue>, int> = 0>
	void Serialize(TArchive& archive, TValue& value)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::string str;
			Serialize(archive, str);
			Convert::Detail::To(std::string_view(str), value);
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
	bool Serialize(TArchive& archive, TKey&& key, TValue& value)
	{
		constexpr auto hasSerializeMethod = has_serialize_method_v<TValue>;
		constexpr auto hasGlobalSerializeObject = has_global_serialize_object_v<TValue>;
		constexpr auto hasGlobalSerializeArray = has_global_serialize_array_v<TValue>;

		// If you are trying to serialize one of known STD types, please make sure that you are included
		// required header with implementation (from "bitserializer/types/std/").
		static_assert(hasSerializeMethod || hasGlobalSerializeObject || hasGlobalSerializeArray,
			"BitSerializer. The class must have defined Serialize() method or one of global functions - SerializeObject() or SerializeArray().");

		if constexpr (hasSerializeMethod || hasGlobalSerializeObject || hasGlobalSerializeArray)
		{
			static_assert(!(hasGlobalSerializeObject && hasGlobalSerializeArray),
				"BitSerializer. Only one function from SerializeObject() or SerializeArray() should be defined.");

			constexpr auto hasObjectWithKeySupport = can_serialize_object_with_key_v<TArchive, TKey>;
			static_assert(hasObjectWithKeySupport, "BitSerializer. The archive doesn't support serialize class with key on this level.");

			if constexpr (hasObjectWithKeySupport)
			{
				// Globally defined functions have higher priority over internal ones
				if constexpr (hasGlobalSerializeObject) {
					auto objectScope = archive.OpenObjectScope(std::forward<TKey>(key));
					if (objectScope) {
						SerializeObject(*objectScope, value);
					}
					return objectScope.has_value();
				}
				else if constexpr (hasGlobalSerializeArray) {
					size_t arraySize = 0;
					if constexpr (TArchive::IsLoading() && has_size_v<TValue>) {
						arraySize = value.size();
					}
					auto arrayScope = archive.OpenArrayScope(std::forward<TKey>(key), arraySize);
					if (arrayScope) {
						SerializeArray(*arrayScope, value);
					}
					return arrayScope.has_value();
				}
				else if constexpr (hasSerializeMethod) {
					auto objectScope = archive.OpenObjectScope(std::forward<TKey>(key));
					if (objectScope) {
						value.Serialize(*objectScope);
					}
					return objectScope.has_value();
				}
			}
		}
		return false;
	}

	template <class TArchive, class TValue, std::enable_if_t<(std::is_class_v<TValue> || std::is_union_v<TValue>), int> = 0>
	void Serialize(TArchive& archive, TValue& value)
	{
		constexpr auto hasSerializeMethod = has_serialize_method_v<TValue>;
		constexpr auto hasGlobalSerializeObject = has_global_serialize_object_v<TValue>;
		constexpr auto hasGlobalSerializeArray = has_global_serialize_array_v<TValue>;

		// If you are trying to serialize one of known STD types, please make sure that you are included
		// required header with implementation (from "bitserializer/types/std/").
		static_assert(hasSerializeMethod || hasGlobalSerializeObject || hasGlobalSerializeArray,
			"BitSerializer. The class must have defined Serialize() method or one of global functions - SerializeObject() or SerializeArray().");

		if constexpr (hasSerializeMethod || hasGlobalSerializeObject || hasGlobalSerializeArray)
		{
			static_assert(!(hasGlobalSerializeObject && hasGlobalSerializeArray),
				"BitSerializer. Only one function from SerializeObject() or SerializeArray() should be defined.");

			constexpr auto hasObjectSupport = can_serialize_object_v<TArchive>;
			static_assert(hasObjectSupport, "BitSerializer. The archive doesn't support serialize class without key on this level.");

			if constexpr (hasObjectSupport)
			{
				// Globally defined methods have higher priority
				if constexpr (hasGlobalSerializeObject) {
					auto objectScope = archive.OpenObjectScope();
					if (objectScope) {
						SerializeObject(*objectScope, value);
					}
				}
				else if constexpr (hasGlobalSerializeArray) {
					size_t arraySize = 0;
					if constexpr (TArchive::IsLoading() && has_size_v<TValue>) {
						arraySize = value.size();
					}
					auto arrayScope = archive.OpenArrayScope(arraySize);
					if (arrayScope) {
						SerializeArray(*arrayScope, value);
					}
				}
				else if constexpr (hasSerializeMethod) {
					auto objectScope = archive.OpenObjectScope();
					if (objectScope) {
						value.Serialize(*objectScope);
					}
				}
			}
		}
	}

	/// <summary>
	/// The wrapper class that keeps a reference to a base user object, used to simplify the serialization of base objects.
	/// </summary>
	template<class TBase>
	struct BaseObject
	{
		template <typename TDerived>
		explicit BaseObject(TDerived& object) noexcept
			: Object(object)
		{
			static_assert(std::is_base_of_v<TBase, TDerived>,
				"BitSerializer. The template parameter 'TBase' should be a base type of passed object.");
		}

		TBase& Object;
	};

	/// <summary>
	/// Serializes the base class.
	/// </summary>
	template <typename TArchive, class TBase>
	void Serialize(TArchive& archive, BaseObject<TBase>&& value)
	{
		constexpr auto isSerializableClass = has_serialize_method_v<TBase>;
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
	bool Serialize(TArchive& archive, TKey&& key, TValue(&cont)[ArraySize])
	{
		constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
		static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

		if constexpr (hasArrayWithKeySupport)
		{
			auto arrayScope = archive.OpenArrayScope(std::forward<TKey>(key), ArraySize);
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
	void Serialize(TArchive& archive, TValue(&cont)[ArraySize])
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
}
