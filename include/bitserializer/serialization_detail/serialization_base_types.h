/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstddef>
#include <type_traits>
#include "bitserializer/serialization_detail/object_traits.h"
#include "bitserializer/serialization_detail/archive_traits.h"
#include "bitserializer/serialization_detail/errors_handling.h"
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
		else {
			return false;
		}
	}

	template <typename TArchive, typename TValue, std::enable_if_t<std::is_fundamental_v<TValue>, int> = 0>
	bool Serialize(TArchive& archive, TValue& value)
	{
		constexpr auto hasValueTypeSupport = can_serialize_value_v<TArchive, TValue>;
		static_assert(hasValueTypeSupport, "BitSerializer. The archive doesn't support serialize fundamental type without key on this level.");

		if constexpr (hasValueTypeSupport) {
			return archive.SerializeValue(value);
		}
		else {
			return false;
		}
	}

	//-----------------------------------------------------------------------------
	// Serialize std::byte
	//-----------------------------------------------------------------------------
	template <typename TArchive, typename TKey>
	bool Serialize(TArchive& archive, TKey&& key, std::byte& value)
	{
		constexpr auto hasValueWithKeySupport = can_serialize_value_with_key_v<TArchive, unsigned char, TKey>;
		static_assert(hasValueWithKeySupport, "BitSerializer. The archive doesn't support serialize `std::byte` with key on this level.");

		if constexpr (hasValueWithKeySupport)
		{
			unsigned char temp;
			if constexpr (TArchive::IsLoading())
			{
				if (archive.SerializeValue(std::forward<TKey>(key), temp))
				{
					value = static_cast<std::byte>(temp);
					return true;
				}
				return false;
			}
			else
			{
				temp = static_cast<unsigned char>(value);
				return archive.SerializeValue(std::forward<TKey>(key), temp);
			}
		}
	}

	template <typename TArchive>
	bool Serialize(TArchive& archive, std::byte& value)
	{
		constexpr auto hasValueTypeSupport = can_serialize_value_v<TArchive, unsigned char>;
		static_assert(hasValueTypeSupport, "BitSerializer. The archive doesn't support serialize `std::byte` without key on this level.");

		if constexpr (hasValueTypeSupport)
		{
			unsigned char temp;
			if constexpr (TArchive::IsLoading())
			{
				if (archive.SerializeValue(temp))
				{
					value = static_cast<std::byte>(temp);
					return true;
				}
				return false;
			}
			else
			{
				temp = static_cast<unsigned char>(value);
				return archive.SerializeValue(temp);
			}
		}
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
		else {
			return false;
		}
	}

	template <typename TArchive>
	bool Serialize(TArchive& archive, std::nullptr_t& value)
	{
		constexpr auto hasValueTypeSupport = can_serialize_value_v<TArchive, std::nullptr_t>;
		static_assert(hasValueTypeSupport, "BitSerializer. The archive doesn't support serialize nullptr type without key on this level.");

		if constexpr (hasValueTypeSupport) {
			return archive.SerializeValue(value);
		}
		else {
			return false;
		}
	}

	//------------------------------------------------------------------------------
	// Serialize string types
	//------------------------------------------------------------------------------
	namespace Detail
	{
		/// <summary>
		/// Transcodes string according to policy.
		/// </summary>
		template <typename TInChar, typename TOutChar>
		void TranscodeStringByPolicy(const std::basic_string_view<TInChar>& sourceString, std::basic_string_view<TOutChar>& targetString, SerializationContext& serializationContext)
		{
			auto& valueBuffer = serializationContext.GetStringValueBuffer<std::basic_string<TOutChar>>();
			valueBuffer.clear();
			if (Convert::Utf::Transcode(sourceString, valueBuffer, serializationContext.GetOptions().utfEncodingErrorPolicy)) {
				targetString = std::basic_string_view<TOutChar>(valueBuffer);
			}
			else {
				throw SerializationException(SerializationErrorCode::UtfEncodingError);
			}
		}

		/// <summary>
		/// Generic function for serialization string_view with key (the value will be valid until the next call).
		/// </summary>
		template <class TArchive, typename TKey, typename TSym>
		bool SerializeString(TArchive& archive, TKey&& key, std::basic_string_view<TSym>& value)
		{
			using archive_string_view = typename TArchive::string_view_type;

			constexpr auto hasKnownStringViewWithKeySupport = can_serialize_value_with_key_v<TArchive, archive_string_view, TKey>;
			constexpr auto hasExactStringViewWithKeySupport = can_serialize_value_with_key_v<TArchive, std::basic_string_view<TSym>, TKey>;
			static_assert(hasExactStringViewWithKeySupport || hasKnownStringViewWithKeySupport,
				"BitSerializer. The archive doesn't support serialize string type with key on this level.");

			if constexpr (hasExactStringViewWithKeySupport)
			{
				return archive.SerializeValue(std::forward<TKey>(key), value);
			}
			else if constexpr (hasKnownStringViewWithKeySupport)
			{
				archive_string_view archiveStringView;
				if constexpr (TArchive::IsLoading())
				{
					if (archive.SerializeValue(std::forward<TKey>(key), archiveStringView))
					{
						TranscodeStringByPolicy(archiveStringView, value, archive.GetContext());
						return true;
					}
					return false;
				}
				else
				{
					TranscodeStringByPolicy(value, archiveStringView, archive.GetContext());
					return archive.SerializeValue(std::forward<TKey>(key), archiveStringView);
				}
			}
		}

		/// <summary>
		/// Generic function for serialization string_view (the value will be valid until the next call).
		/// </summary>
		template <class TArchive, typename TSym>
		bool SerializeString(TArchive& archive, std::basic_string_view<TSym>& value)
		{
			using archive_string_view = typename TArchive::string_view_type;

			constexpr auto hasExactStringViewSupport = can_serialize_value_v<TArchive, std::basic_string_view<TSym>>;
			constexpr auto hasKnownStringViewSupport = can_serialize_value_v < TArchive, archive_string_view>;
			static_assert(hasExactStringViewSupport || hasKnownStringViewSupport,
				"BitSerializer. The archive doesn't support serialize string type without key on this level.");

			if constexpr (hasExactStringViewSupport)
			{
				return archive.SerializeValue(value);
			}
			else if constexpr (hasKnownStringViewSupport)
			{
				archive_string_view archiveStringView;
				if constexpr (TArchive::IsLoading())
				{
					if (archive.SerializeValue(archiveStringView))
					{
						TranscodeStringByPolicy(archiveStringView, value, archive.GetContext());
						return true;
					}
					return false;
				}
				else
				{
					TranscodeStringByPolicy(value, archiveStringView, archive.GetContext());
					return archive.SerializeValue(archiveStringView);
				}
			}
		}
	}

	template <class TArchive, typename TKey, typename TSym, typename TAllocator>
	bool Serialize(TArchive& archive, TKey&& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::basic_string_view<TSym> stringView;
			if (Detail::SerializeString(archive, std::forward<TKey>(key), stringView))
			{
				value.assign(stringView);
				return true;
			}
			return false;
		}
		else
		{
			std::basic_string_view<TSym> stringView(value);
			return Detail::SerializeString(archive, std::forward<TKey>(key), stringView);
		}
	}

	template <class TArchive, typename TSym, typename TAllocator>
	bool Serialize(TArchive& archive, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::basic_string_view<TSym> stringView;
			if (Detail::SerializeString(archive, stringView))
			{
				value.assign(stringView);
				return true;
			}
			return false;
		}
		else
		{
			std::basic_string_view<TSym> stringView(value);
			return Detail::SerializeString(archive, stringView);
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
			std::string_view stringView;
			if (Detail::SerializeString(archive, std::forward<TKey>(key), stringView))
			{
				return Detail::ConvertByPolicy(stringView, value, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy);
			}
			return false;
		}
		else
		{
			if (auto metadata = Convert::Detail::EnumRegistry<TValue>::GetEnumMetadata(value))
			{
				std::string_view valueName(metadata->Name);
				return Detail::SerializeString(archive, std::forward<TKey>(key), valueName);
			}
			if (Convert::Detail::EnumRegistry<TValue>::IsRegistered())
			{
				throw SerializationException(SerializationErrorCode::UnregisteredEnum,
					"Enum value (" + Convert::ToString(static_cast<std::underlying_type_t<TValue>>(value)) + ") is invalid or not registered");
			}
			throw SerializationException(SerializationErrorCode::UnregisteredEnum);
		}
	}

	template <class TArchive, class TValue, std::enable_if_t<std::is_enum_v<TValue>, int> = 0>
	bool Serialize(TArchive& archive, TValue& value)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::string_view stringView;
			if (Detail::SerializeString(archive, stringView))
			{
				return Detail::ConvertByPolicy(stringView, value, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy);
			}
			return false;
		}
		else
		{
			if (auto metadata = Convert::Detail::EnumRegistry<TValue>::GetEnumMetadata(value))
			{
				std::string_view valueName(metadata->Name);
				return Detail::SerializeString(archive, valueName);
			}
			if (Convert::Detail::EnumRegistry<TValue>::IsRegistered())
			{
				throw SerializationException(SerializationErrorCode::UnregisteredEnum,
					"Enum value (" + Convert::ToString(static_cast<std::underlying_type_t<TValue>>(value)) + ") is invalid or not registered");
			}
			throw SerializationException(SerializationErrorCode::UnregisteredEnum);
		}
	}

	/// <summary>
	/// Allows to serialize enum types as integers.
	///	Usage example: archive << MakeKeyValue("EnumValue", EnumAsBin(enumValue));
	/// </summary>
	template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
	struct EnumAsBin
	{
		using value_type = std::underlying_type_t<T>;

		explicit EnumAsBin(T& value) noexcept : Value(value) {}
		value_type& GetUnderlyingValue() noexcept { return reinterpret_cast<value_type&>(Value); }

		T& Value;
	};

	template <class TArchive, typename TKey, class TValue>
	bool Serialize(TArchive& archive, TKey&& key, EnumAsBin<TValue> value)
	{
		return Serialize(archive, std::forward<TKey>(key), value.GetUnderlyingValue());
	}

	template <class TArchive, class TValue>
	bool Serialize(TArchive& archive, EnumAsBin<TValue> value)
	{
		return Serialize(archive, value.GetUnderlyingValue());
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

			// Internal Serialize() method has higher priority than global one
			if constexpr (hasSerializeMethod)
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
			else if constexpr (hasGlobalSerializeObject)
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
				constexpr auto hasBinaryWithKeySupport = TArchive::is_binary && can_serialize_binary_with_key_v<TArchive, TKey>;
				constexpr auto isBinaryContainer = is_binary_container<TValue>;
				static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

				size_t arraySize = 0;
				if constexpr (TArchive::IsSaving())
				{
					constexpr auto hasSize = has_size_v<TValue> || has_global_size_v<TValue>;
					constexpr auto isEnumerable = is_enumerable_v<TValue>;
					static_assert(!TArchive::is_binary || (hasSize || isEnumerable), "BitSerializer. Saving to a binary archive requires a known container size.");

					if constexpr (hasSize || isEnumerable) {
						arraySize = GetContainerSize(value);
					}
				}

				// Try to serialize as binary first
				if constexpr (hasBinaryWithKeySupport && isBinaryContainer)
				{
					auto binaryScope = archive.OpenBinaryScope(std::forward<TKey>(key), arraySize);
					if (binaryScope)
					{
						SerializeArray(*binaryScope, value);
						return true;
					}
				}
				if constexpr (hasArrayWithKeySupport)
				{
					auto arrayScope = archive.OpenArrayScope(std::forward<TKey>(key), arraySize);
					if (arrayScope) {
						SerializeArray(*arrayScope, value);
					}
					return arrayScope.has_value();
				}
			}
		}
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

			// Internal Serialize() method has higher priority than global one
			if constexpr (hasSerializeMethod)
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
			else if constexpr (hasGlobalSerializeObject)
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
				constexpr auto hasBinarySupport = TArchive::is_binary && can_serialize_binary_v<TArchive>;
				constexpr auto isBinaryContainer = is_binary_container<TValue>;
				static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

				size_t arraySize = 0;
				if constexpr (TArchive::IsSaving())
				{
					constexpr auto hasSize = has_size_v<TValue> || has_global_size_v<TValue>;
					constexpr auto isEnumerable = is_enumerable_v<TValue>;
					static_assert(!TArchive::is_binary || (hasSize || isEnumerable), "BitSerializer. Saving to a binary archive requires a known container size.");

					if constexpr (hasSize || isEnumerable) {
						arraySize = GetContainerSize(value);
					}
				}

				// Try to serialize as binary first
				if constexpr (hasBinarySupport && isBinaryContainer)
				{
					auto binaryScope = archive.OpenBinaryScope(arraySize);
					if (binaryScope)
					{
						SerializeArray(*binaryScope, value);
						return true;
					}
				}
				if constexpr (hasArraySupport)
				{
					auto arrayScope = archive.OpenArrayScope(arraySize);
					if (arrayScope) {
						SerializeArray(*arrayScope, value);
					}
					return arrayScope.has_value();
				}
			}
		}
	}

	/// <summary>
	/// Serializes the base class.
	/// </summary>
	template <typename TArchive, class TBase>
	bool Serialize(TArchive& archive, BaseObject<TBase>& value)
	{
		constexpr auto isObjectScope = is_object_scope_v<TArchive, typename TArchive::key_type>;
		constexpr auto hasSerializeMethod = has_serialize_method_v<TBase>;
		constexpr auto hasGlobalSerializeObject = has_global_serialize_object_v<TBase>;

		// Usage of the BaseObject<>() helper method is expected in the global SerializeObject() function or in the Serialize() class method.
		static_assert(isObjectScope, "BitSerializer. Incorrect usage of BaseObject, should be as part of class serialization.");

		// Make sure your class supports serialization using internal or global serialize methods.
		static_assert(hasSerializeMethod || hasGlobalSerializeObject,
			"BitSerializer. The class must have defined Serialize() method or global function SerializeObject().");

		if constexpr (isObjectScope && (hasSerializeMethod || hasGlobalSerializeObject))
		{
			// Internal Serialize() method has higher priority than global one
			if constexpr (hasSerializeMethod)
			{
				value.Object.Serialize(archive);
				return true;
			}
			else if constexpr (hasGlobalSerializeObject)
			{
				SerializeObject(archive, value.Object);
				return true;
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
		constexpr auto hasBinaryWithKeySupport = TArchive::is_binary && can_serialize_binary_with_key_v<TArchive, TKey>;
		constexpr auto isBinaryArray = std::is_same_v<TValue, char> || std::is_same_v<TValue, signed char> || std::is_same_v<TValue, unsigned char>;
		static_assert(hasArrayWithKeySupport, "BitSerializer. The archive doesn't support serialize array with key on this level.");

		// Try to serialize as binary first
		if constexpr (hasBinaryWithKeySupport && isBinaryArray)
		{
			if (auto arrayScope = archive.OpenBinaryScope(std::forward<TKey>(key), ArraySize))
			{
				return Detail::SerializeFixedSizeArray(arrayScope.value(), std::begin(cont), std::end(cont));
			}
		}
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
		constexpr auto hasBinarySupport = TArchive::is_binary && can_serialize_binary_v<TArchive>;
		constexpr auto isBinaryArray = std::is_same_v<TValue, char> || std::is_same_v<TValue, signed char> || std::is_same_v<TValue, unsigned char>;
		static_assert(hasArraySupport, "BitSerializer. The archive doesn't support serialize array without key on this level.");

		// Try to serialize as binary first
		if constexpr (hasBinarySupport && isBinaryArray)
		{
			if (auto arrayScope = archive.OpenBinaryScope(ArraySize))
			{
				return Detail::SerializeFixedSizeArray(arrayScope.value(), std::begin(cont), std::end(cont));
			}
		}
		if constexpr (hasArraySupport)
		{
			if (auto arrayScope = archive.OpenArrayScope(ArraySize)) 
			{
				return Detail::SerializeFixedSizeArray(arrayScope.value(), std::begin(cont), std::end(cont));
			}
		}
		return false;
	}

	//-----------------------------------------------------------------------------
	// Serialize constant values (only saving is allowed)
	//-----------------------------------------------------------------------------
	template <typename TArchive, typename TKey, typename TValue>
	bool Serialize(TArchive& archive, TKey&& key, const TValue& value)
	{
		static_assert(!TArchive::IsLoading(), "BitSerializer. Cannot load into constant object.");
		return Serialize(archive, std::forward<TKey>(key), const_cast<TValue&>(value));
	}

	template <typename TArchive, typename TValue>
	bool Serialize(TArchive& archive, const TValue& value)
	{
		static_assert(!TArchive::IsLoading(), "BitSerializer. Cannot load into constant object.");
		return Serialize(archive, const_cast<TValue&>(value));
	}
}
