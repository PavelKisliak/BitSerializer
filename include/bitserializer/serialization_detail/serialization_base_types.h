/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>
#include "object_traits.h"
#include "archive_traits.h"
#include "errors_handling.h"
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
	bool Serialize(TArchive& archive, TValue& value)
	{
		constexpr auto hasValueTypeSupport = can_serialize_value_v<TArchive, TValue>;
		static_assert(hasValueTypeSupport, "BitSerializer. The archive doesn't support serialize fundamental type without key on this level.");

		if constexpr (hasValueTypeSupport) {
			return archive.SerializeValue(value);
		}
		return false;
	}

	//-----------------------------------------------------------------------------
	// Serialize nullptr type
	//-----------------------------------------------------------------------------
	template <typename TArchive, typename TKey>
	bool Serialize(TArchive& archive, TKey&& key, std::nullptr_t& value)
	{
		constexpr auto hasValueWithKeySupport = can_serialize_value_with_key_v<TArchive, std::nullptr_t, TKey>;
		static_assert(hasValueWithKeySupport, "BitSerializer. The archive doesn't support serialize nullptr type with key on this level.");

		if constexpr (hasValueWithKeySupport) {
			return archive.SerializeValue(std::forward<TKey>(key), value);
		}
		return false;
	}

	template <typename TArchive>
	bool Serialize(TArchive& archive, std::nullptr_t& value)
	{
		constexpr auto hasValueTypeSupport = can_serialize_value_v<TArchive, std::nullptr_t>;
		static_assert(hasValueTypeSupport, "BitSerializer. The archive doesn't support serialize nullptr type without key on this level.");

		if constexpr (hasValueTypeSupport) {
			return archive.SerializeValue(value);
		}
		return false;
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
	bool Serialize(TArchive& archive, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		constexpr auto hasStringSupport = can_serialize_value_v<TArchive, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>;
		static_assert(hasStringSupport, "BitSerializer. The archive doesn't support serialize string type without key on this level.");

		if constexpr (hasStringSupport) {
			return archive.SerializeValue(value);
		}
		return false;
	}

	//-----------------------------------------------------------------------------
	// Serialize enum types
	//-----------------------------------------------------------------------------
	namespace Detail
	{
		template <class TValue, std::enable_if_t<std::is_enum_v<TValue>, int> = 0>
		bool ConvertStringToEnumByPolicy(const std::string& str, TValue& out_value, MismatchedTypesPolicy policy)
		{
			try
			{
				out_value = Convert::To<TValue>(str);
				return true;
			}
			catch (const std::invalid_argument&)
			{
				if (policy == MismatchedTypesPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::MismatchedTypes,
						"The string (" + str + ") cannot be converted to target enum");
				}
			}
			catch (...) {
				throw SerializationException(SerializationErrorCode::ParsingError, "Unknown error when converting enum to string");
			}
			return false;
		}
	}

	template <class TArchive, typename TKey, class TValue, std::enable_if_t<std::is_enum_v<TValue>, int> = 0>
	bool Serialize(TArchive& archive, TKey&& key, TValue& value)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::string str;
			if (Serialize(archive, std::forward<TKey>(key), str)) {
				return Detail::ConvertStringToEnumByPolicy(str, value, archive.GetOptions().mismatchedTypesPolicy);
			}
			return false;
		}
		else
		{
			// May throw exception when enum is not registered or has invalid value
			auto str = Convert::ToString(value);
			return Serialize(archive, std::forward<TKey>(key), str);
		}
	}

	template <class TArchive, class TValue, std::enable_if_t<std::is_enum_v<TValue>, int> = 0>
	bool Serialize(TArchive& archive, TValue& value)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::string str;
			if (Serialize(archive, str)) {
				return Detail::ConvertStringToEnumByPolicy(str, value, archive.GetOptions().mismatchedTypesPolicy);
			}
			return false;
		}
		else
		{
			// May throw exception when enum is not registered or has invalid value
			auto str = Convert::ToString(value);
			return Serialize(archive, str);
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
				"BitSerializer. Only one function from SerializeObject() or SerializeArray() should be defined for particular type.");

			// Globally defined functions have higher priority over internal ones
			if constexpr (hasGlobalSerializeObject)
			{
				constexpr auto hasObjectWithKeySupport = can_serialize_object_with_key_v<TArchive, TKey>;
				static_assert(hasObjectWithKeySupport, "BitSerializer. The archive doesn't support serialize class with key on this level.");

				if constexpr (hasObjectWithKeySupport)
				{
					const size_t mapSize = CountMapObjectFields(archive, value);
					auto objectScope = archive.OpenObjectScope(std::forward<TKey>(key), mapSize);
					if (objectScope) {
						SerializeObject(*objectScope, value);
					}
					return objectScope.has_value();
				}
			}
			else if constexpr (hasGlobalSerializeArray)
			{
				constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
				static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

				if constexpr (hasArrayWithKeySupport)
				{
					size_t arraySize = 0;
					if constexpr (TArchive::IsSaving())
					{
						constexpr auto hasGlobalSize = has_global_size_v<TValue>;
						constexpr auto isEnumerable = is_enumerable_v<TValue>;
						static_assert(!TArchive::is_binary || (hasGlobalSize || isEnumerable), "BitSerializer. Saving to a binary archive requires a known container size.");

						if constexpr (hasGlobalSize || isEnumerable) {
							arraySize = GetContainerSize(value);
						}
					}

					auto arrayScope = archive.OpenArrayScope(std::forward<TKey>(key), arraySize);
					if (arrayScope) {
						SerializeArray(*arrayScope, value);
					}
					return arrayScope.has_value();
				}
			}
			else if constexpr (hasSerializeMethod)
			{
				constexpr auto hasObjectWithKeySupport = can_serialize_object_with_key_v<TArchive, TKey>;
				static_assert(hasObjectWithKeySupport, "BitSerializer. The archive doesn't support serialize class with key on this level.");

				if constexpr (hasObjectWithKeySupport)
				{
					const size_t mapSize = CountMapObjectFields(archive, value);
					auto objectScope = archive.OpenObjectScope(std::forward<TKey>(key), mapSize);
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
	bool Serialize(TArchive& archive, TValue& value)
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

			// Globally defined methods have higher priority
			if constexpr (hasGlobalSerializeObject)
			{
				constexpr auto hasObjectSupport = can_serialize_object_v<TArchive>;
				static_assert(hasObjectSupport, "BitSerializer. The archive doesn't support serialize class without key on this level.");

				if constexpr (hasObjectSupport)
				{
					const size_t mapSize = CountMapObjectFields(archive, value);
					auto objectScope = archive.OpenObjectScope(mapSize);
					if (objectScope) {
						SerializeObject(*objectScope, value);
					}
					return objectScope.has_value();
				}
			}
			else if constexpr (hasGlobalSerializeArray)
			{
				constexpr auto hasArraySupport = can_serialize_array_v<TArchive>;
				static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

				if constexpr (hasArraySupport)
				{
					size_t arraySize = 0;
					if constexpr (TArchive::IsSaving())
					{
						constexpr auto hasGlobalSize = has_global_size_v<TValue>;
						constexpr auto isEnumerable = is_enumerable_v<TValue>;
						static_assert(!TArchive::is_binary || (hasGlobalSize || isEnumerable), "BitSerializer. Saving to a binary archive requires a known container size.");

						if constexpr (hasGlobalSize || isEnumerable) {
							arraySize = GetContainerSize(value);
						}
					}

					auto arrayScope = archive.OpenArrayScope(arraySize);
					if (arrayScope) {
						SerializeArray(*arrayScope, value);
					}
					return arrayScope.has_value();
				}
			}
			else if constexpr (hasSerializeMethod)
			{
				constexpr auto hasObjectSupport = can_serialize_object_v<TArchive>;
				static_assert(hasObjectSupport, "BitSerializer. The archive doesn't support serialize class without key on this level.");

				if constexpr (hasObjectSupport)
				{
					const size_t mapSize = CountMapObjectFields(archive, value);
					auto objectScope = archive.OpenObjectScope(mapSize);
					if (objectScope) {
						value.Serialize(*objectScope);
					}
					return objectScope.has_value();
				}
			}
		}
		return false;
	}

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
	// Serialize C-arrays
	//-----------------------------------------------------------------------------
	namespace Detail
	{
		/// <summary>
		/// Generic function for serialization arrays with fixed size (like native C-arrays and std::array).
		/// </summary>
		template<typename TArchive, typename TIterator>
		bool SerializeFixedSizeArray(TArchive& arrayScope, TIterator startIt, TIterator endIt)
		{
			if constexpr (TArchive::IsLoading())
			{
				auto it = startIt;
				for (; it != endIt && !arrayScope.IsEnd(); ++it)
				{
					Serialize(arrayScope, *it);
				}

				if (it != endIt || !arrayScope.IsEnd())
				{
					throw SerializationException(SerializationErrorCode::OutOfRange,
						"Target array with fixed size does not match the number of loading items");
				}
			}
			else
			{
				for (auto it = startIt; it != endIt; ++it)
				{
					Serialize(arrayScope, *it);
				}
			}
			return true;
		}
	}

	template<typename TArchive, typename TKey, typename TValue, size_t ArraySize>
	bool Serialize(TArchive& archive, TKey&& key, TValue(&cont)[ArraySize])
	{
		constexpr auto hasArrayWithKeySupport = can_serialize_array_with_key_v<TArchive, TKey>;
		static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

		if constexpr (hasArrayWithKeySupport)
		{
			if (auto arrayScope = archive.OpenArrayScope(std::forward<TKey>(key), ArraySize))
			{
				return Detail::SerializeFixedSizeArray(arrayScope.value(), std::begin(cont), std::end(cont));
			}
		}
		return false;
	}

	template<typename TArchive, typename TValue, size_t ArraySize>
	bool Serialize(TArchive& archive, TValue(&cont)[ArraySize])
	{
		constexpr auto hasArraySupport = can_serialize_array_v<TArchive>;
		static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

		if constexpr (hasArraySupport)
		{
			if (auto arrayScope = archive.OpenArrayScope(ArraySize)) 
			{
				return Detail::SerializeFixedSizeArray(arrayScope.value(), std::begin(cont), std::end(cont));
			}
		}
		return false;
	}
}
