/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <variant>
#include <type_traits>

#include "bitserializer/convert.h"
#include "bitserializer/key_value.h"
#include "bitserializer/serialization_options.h"
#include "bitserializer/serialization_detail/archive_traits.h"
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/object_traits.h"
#include "bitserializer/serialization_detail/type_registry.h"

namespace BitSerializer
{
	namespace Detail
	{
		/**
		 * @brief Checks whether a variant alternative can be serialized as a value under a key
		 * in the current archive (needed by the `Indexed` and `Named` representations).
		 */
		template <typename TArchive, typename TValue>
		inline constexpr bool can_serialize_alternative_with_key_v =
			can_serialize_value_with_key_v<TArchive, TValue, typename TArchive::key_type>
			|| (Convert::Detail::is_string_type_v<TValue> && can_serialize_value_with_key_v<TArchive, typename TArchive::string_view_type, typename TArchive::key_type>)
			|| ((has_serialize_method_v<TValue> || has_global_serialize_object_v<TValue>) && can_serialize_object_with_key_v<TArchive, typename TArchive::key_type>)
			|| (has_global_serialize_array_v<TValue> && can_serialize_array_with_key_v<TArchive, typename TArchive::key_type>);

		template <size_t TIndex = 0, typename TArchive, typename TKey, typename... TArgs>
		void SerializeVariantAlternative(TArchive& archive, const TKey& key, std::variant<TArgs...>& value, size_t activeIndex)
		{
			if constexpr (TIndex < sizeof...(TArgs))
			{
				if (activeIndex == TIndex)
				{
					if (value.index() != TIndex)
					{
						value.template emplace<TIndex>();
					}
					Serialize(archive, key, std::get<TIndex>(value));
				}
				else
				{
					SerializeVariantAlternative<TIndex + 1>(archive, key, value, activeIndex);
				}
			}
			else if (archive.GetOptions().mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
			{
				throw SerializationException(SerializationErrorCode::MismatchedTypes,
					"Variant index is out of range for target type: " + Convert::ToString(activeIndex));
			}
		}

		/**
		 * @brief Serializes the fields of an object directly into the current scope (flattening).
		 *
		 * Used by the discriminated (`VariantAsDiscriminated`) representation, where the active
		 * alternative's fields are merged into the same object that holds the discriminator,
		 * instead of being nested under a separate value key.
		 *
		 * @note Requires the alternative to be an object type (has `Serialize()` method or
		 * a global `SerializeObject()`), since a scalar or array cannot be flattened.
		 */
		template <typename TArchive, typename TValue>
		void SerializeVariantAlternativeFields(TArchive& archive, TValue& value)
		{
			constexpr auto hasSerializeMethod = has_serialize_method_v<TValue>;
			constexpr auto hasGlobalSerializeObject = has_global_serialize_object_v<TValue>;
			static_assert(hasSerializeMethod || hasGlobalSerializeObject,
				"BitSerializer. Discriminated variant representation (VariantAsDiscriminated) requires every alternative to be an object type with a Serialize() method or a global SerializeObject().");

			if constexpr (hasSerializeMethod)
			{
				value.Serialize(archive);
			}
			else if constexpr (hasGlobalSerializeObject)
			{
				SerializeObject(archive, value);
			}
		}

		/**
		 * @brief Serializes `std::variant` as an object with `index` and `value` fields.
		 */
		template <typename TArchive, typename... TArgs>
		void SerializeVariantIndexed(TArchive& archive, std::variant<TArgs...>& value, std::string_view indexKey, std::string_view valueKey)
		{
			using index_type = size_t;

			if constexpr (TArchive::IsLoading())
			{
				static_assert((std::is_default_constructible_v<TArgs> && ...),
					"BitSerializer. All std::variant alternatives must be default-constructible for deserialization");

				const auto indexName = Convert::To<typename TArchive::key_type>(indexKey);
				const auto valueName = Convert::To<typename TArchive::key_type>(valueKey);

				index_type activeIndex = 0;
				if (Serialize(archive, indexName, activeIndex))
				{
					SerializeVariantAlternative(archive, valueName, value, activeIndex);
				}
			}
			else
			{
				if (value.valueless_by_exception())
				{
					throw SerializationException(SerializationErrorCode::OutOfRange,
						"Cannot serialize std::variant in valueless_by_exception state");
				}

				index_type activeIndex = value.index();
				archive << KeyValue(indexKey, activeIndex);
				std::visit([&archive, &valueKey](auto& activeValue) {
					archive << KeyValue(valueKey, activeValue);
				}, value);
			}
		}

		/**
		 * @brief Serializes `std::variant` as an object with `type` and `value` fields using registered type names.
		 */
		template <typename TArchive, typename... TArgs>
		void SerializeVariantNamed(TArchive& archive, std::variant<TArgs...>& value, std::string_view typeKey, std::string_view valueKey)
		{
			using Registry = TypeRegistry<TArgs...>;

			if constexpr (TArchive::IsLoading())
			{
				static_assert((std::is_default_constructible_v<TArgs> && ...),
					"BitSerializer. All std::variant alternatives must be default-constructible for deserialization");

				std::string typeName;
				const auto typeNameKey = Convert::To<typename TArchive::key_type>(typeKey);
				const auto valueName = Convert::To<typename TArchive::key_type>(valueKey);

				if (Serialize(archive, typeNameKey, typeName))
				{
					const auto* entry = Registry::Find(typeName);
					if (!entry)
					{
						if (archive.GetOptions().mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
						{
							throw SerializationException(SerializationErrorCode::MismatchedTypes,
								"Unknown variant type: " + typeName);
						}
						return;
					}

					Registry::EmplaceByIndex(value, entry->index);
					std::visit([&archive, &valueName](auto& activeValue) {
						Serialize(archive, valueName, activeValue);
					}, value);
				}
			}
			else
			{
				if (value.valueless_by_exception())
				{
					throw SerializationException(SerializationErrorCode::OutOfRange,
						"Cannot serialize std::variant in valueless_by_exception state");
				}

				const auto* entry = Registry::FindByIndex(value.index());
				if (!entry)
				{
					throw SerializationException(SerializationErrorCode::OutOfRange,
						"Variant index out of registry range");
				}

				archive << KeyValue(typeKey, std::string(entry->name));
				std::visit([&archive, &valueKey](auto& activeValue) {
					archive << KeyValue(valueKey, activeValue);
				}, value);
			}
		}

		/**
		 * @brief Serializes `std::variant` as an object with an embedded discriminator.
		 *
		 * The object contains the discriminator (`typeKey`) followed by the active alternative's
		 * fields flattened at the same level. Requires every alternative to be registered via
		 * `BITSERIALIZER_REGISTER_TYPE` and to be an object type.
		 */
		template <typename TArchive, typename... TArgs>
		void SerializeVariantDiscriminated(TArchive& archive, std::variant<TArgs...>& value, std::string_view typeKey)
		{
			using Registry = TypeRegistry<TArgs...>;

			if constexpr (TArchive::IsLoading())
			{
				static_assert((std::is_default_constructible_v<TArgs> && ...),
					"BitSerializer. All std::variant alternatives must be default-constructible for deserialization");

				std::string typeName;
				const auto typeNameKey = Convert::To<typename TArchive::key_type>(typeKey);

				if (Serialize(archive, typeNameKey, typeName))
				{
					const auto* entry = Registry::Find(typeName);
					if (!entry)
					{
						if (archive.GetOptions().mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
						{
							throw SerializationException(SerializationErrorCode::MismatchedTypes,
								"Unknown variant type: " + typeName);
						}
						return;
					}

					Registry::EmplaceByIndex(value, entry->index);
					std::visit([&archive](auto& activeValue) {
						Detail::SerializeVariantAlternativeFields(archive, activeValue);
					}, value);
				}
			}
			else
			{
				if (value.valueless_by_exception())
				{
					throw SerializationException(SerializationErrorCode::OutOfRange,
						"Cannot serialize std::variant in valueless_by_exception state");
				}

				const auto* entry = Registry::FindByIndex(value.index());
				if (!entry)
				{
					throw SerializationException(SerializationErrorCode::OutOfRange,
						"Variant index out of registry range");
				}

				archive << KeyValue(typeKey, std::string(entry->name));
				std::visit([&archive](auto& activeValue) {
					Detail::SerializeVariantAlternativeFields(archive, activeValue);
				}, value);
			}
		}
	}

	/**
	 * @brief A wrapper that explicitly requests indexed serialization of `std::variant`.
	 *
	 * Serializes the variant as an object with `index` and `value` fields,
	 * preserving the type information via integer index.
	 *
	 * @par Example:
	 * @code
	 * std::variant<int, std::string, CUser> data;
	 * archive << KeyValue("data", VariantAsIndexed(data));
	 * @endcode
	 */
	template <typename T>
	struct VariantAsIndexed
	{
		explicit VariantAsIndexed(T& v, std::string_view indexKey = "index", std::string_view valueKey = "value") noexcept
			: value(v)
			, indexKey(indexKey)
			, valueKey(valueKey)
		{ }

		T& value;
		std::string_view indexKey;
		std::string_view valueKey;
	};

	/**
	 * @brief Serializes `VariantAsIndexed<std::variant<...>>` as an object with `index` and `value` fields.
	 */
	template <typename TArchive, typename... TArgs>
	void SerializeObject(TArchive& archive, VariantAsIndexed<std::variant<TArgs...>> taggedVariant)
	{
		constexpr bool allAlternativesSerializableWithKey = (Detail::can_serialize_alternative_with_key_v<TArchive, TArgs> && ...);
		static_assert(allAlternativesSerializableWithKey,
			"BitSerializer. VariantAsIndexed requires every alternative to be serializable with a key in the current archive.");
		if constexpr (allAlternativesSerializableWithKey)
		{
			Detail::SerializeVariantIndexed(archive, taggedVariant.value, taggedVariant.indexKey, taggedVariant.valueKey);
		}
	}

	/**
	 * @brief A wrapper that explicitly requests name-based serialization of `std::variant`.
	 *
	 * Serializes the variant as an object with `type` and `value` fields, using registered type names instead of integer indices.
	 * Requires all alternatives to be registered via BITSERIALIZER_REGISTER_TYPE.
	 *
	 * @par Example:
	 * @code
	 * std::variant<int, std::string, CUser> data;
	 * archive << KeyValue("data", VariantAsNamed(data));
	 * @endcode
	 */
	template <typename T>
	struct VariantAsNamed
	{
		explicit VariantAsNamed(T& v, std::string_view typeKey = "type", std::string_view valueKey = "value") noexcept
			: value(v)
			, typeKey(typeKey)
			, valueKey(valueKey)
		{ }

		T& value;
		std::string_view typeKey;
		std::string_view valueKey;
	};

	/**
	 * @brief Serializes `VariantAsNamed<std::variant<...>>` as an object with `type` and `value` fields.
	 *
	 * Uses TypeRegistry to serialize variant by type name instead of index.
	 * Requires all alternatives to be registered via BITSERIALIZER_REGISTER_TYPE.
	 */
	template <typename TArchive, typename... TArgs>
	void SerializeObject(TArchive& archive, VariantAsNamed<std::variant<TArgs...>> taggedVariant)
	{
		constexpr bool allRegistered = (Detail::is_type_registered_v<TArgs> && ...);
		constexpr bool allAlternativesSerializableWithKey = (Detail::can_serialize_alternative_with_key_v<TArchive, TArgs> && ...);
		static_assert(allRegistered,
			"BitSerializer. VariantAsNamed requires every alternative to be registered with BITSERIALIZER_REGISTER_TYPE.");
		static_assert(allAlternativesSerializableWithKey,
			"BitSerializer. VariantAsNamed requires every alternative to be serializable with a key in the current archive.");
		if constexpr (allRegistered && allAlternativesSerializableWithKey)
		{
			Detail::SerializeVariantNamed(archive, taggedVariant.value, taggedVariant.typeKey, taggedVariant.valueKey);
		}
	}

	/**
	 * @brief A wrapper that requests discriminated (internally-tagged) serialization of `std::variant`.
	 *
	 * The type name is embedded inside the payload next to the active alternative's fields
	 * (the OpenAPI `discriminator` style), instead of being wrapped around it. This allows object
	 * hierarchies to be represented in flat formats such as CSV, where nested values are not possible.
	 *
	 * @par Example:
	 * @code
	 * std::variant<int, std::string, CUser> data;
	 * archive << KeyValue("data", VariantAsDiscriminated(data));
	 * archive << KeyValue("data", VariantAsDiscriminated(data, "kind"));
	 * @endcode
	 */
	template <typename T>
	struct VariantAsDiscriminated
	{
		explicit VariantAsDiscriminated(T& v, std::string_view typeKey = "type") noexcept
			: value(v)
			, typeKey(typeKey)
		{ }

		T& value;
		std::string_view typeKey;
	};

	/**
	 * @brief Serializes `VariantAsDiscriminated<std::variant<...>>` as an object with an embedded discriminator.
	 *
	 * The object contains the discriminator (`typeKey`) followed by the active alternative's
	 * fields flattened at the same level. Requires every alternative to be registered via
	 * `BITSERIALIZER_REGISTER_TYPE` and to be an object type.
	 */
	template <typename TArchive, typename... TArgs>
	void SerializeObject(TArchive& archive, VariantAsDiscriminated<std::variant<TArgs...>> taggedVariant)
	{
		constexpr bool allObjectAlternatives = ((has_serialize_method_v<TArgs> || has_global_serialize_object_v<TArgs>) && ...);
		constexpr bool allRegistered = (Detail::is_type_registered_v<TArgs> && ...);
		static_assert(allObjectAlternatives,
			"BitSerializer. VariantAsDiscriminated requires every alternative to be an object type with a Serialize() method or a global SerializeObject(); primitive types (e.g. int, float, bool, std::string) are not supported.");
		static_assert(allRegistered,
			"BitSerializer. VariantAsDiscriminated requires every alternative to be registered with BITSERIALIZER_REGISTER_TYPE.");
		if constexpr (allObjectAlternatives && allRegistered)
		{
			Detail::SerializeVariantDiscriminated(archive, taggedVariant.value, taggedVariant.typeKey);
		}
	}

	/**
	 * @brief Serializes `std::variant` using the default representation (DEFAULT).
	 *
	 * The representation is selected by `SerializationOptions::variantOptions.mode`:
	 * - `Indexed` (default): object with `index` and `value` fields.
	 * - `Named`: object with `type` and `value` fields using registered type names.
	 * - `Discriminated`: object with an embedded discriminator and the alternative's fields flattened.
	 *
	 * The `Named` and `Discriminated` modes require every alternative to be registered via
	 * `BITSERIALIZER_REGISTER_TYPE`; the `Discriminated` mode also requires every alternative to be
	 * an object type. These requirements are enforced at compile time when possible - if a selected
	 * mode is not available for the given alternatives, a `SerializationException` is thrown at runtime.
	 *
	 * @note This representation requires object support in the target archive.
	 * Flat archives such as CSV may not support nested alternatives.
	 */
	template <typename TArchive, typename... TArgs>
	void SerializeObject(TArchive& archive, std::variant<TArgs...>& value)
	{
		const auto& variantOptions = archive.GetOptions().variantOptions;
		constexpr bool allRegistered = (Detail::is_type_registered_v<TArgs> && ...);
		constexpr bool allObjectAlternatives = ((has_serialize_method_v<TArgs> || has_global_serialize_object_v<TArgs>) && ...);
		constexpr bool canSerializeWithKey = (Detail::can_serialize_alternative_with_key_v<TArchive, TArgs> && ...);

		switch (variantOptions.mode)
		{
		case VariantSerializationMode::Named:
			if constexpr (allRegistered && canSerializeWithKey)
			{
				Detail::SerializeVariantNamed(archive, value, variantOptions.typeKey, variantOptions.valueKey);
			}
			else
			{
				throw SerializationException(SerializationErrorCode::MismatchedTypes,
					"BitSerializer. VariantSerializationMode::Named requires every alternative to be registered with BITSERIALIZER_REGISTER_TYPE and serializable with a key in the current archive.");
			}
			break;
		case VariantSerializationMode::Discriminated:
			if constexpr (allObjectAlternatives)
			{
				if constexpr (allRegistered)
				{
					Detail::SerializeVariantDiscriminated(archive, value, variantOptions.typeKey);
				}
				else
				{
					throw SerializationException(SerializationErrorCode::MismatchedTypes,
						"BitSerializer. VariantSerializationMode::Discriminated requires every alternative to be registered with BITSERIALIZER_REGISTER_TYPE.");
				}
			}
			else
			{
				throw SerializationException(SerializationErrorCode::MismatchedTypes,
					"BitSerializer. VariantSerializationMode::Discriminated requires every alternative to be an object type; primitive types (e.g. int, float, bool, std::string) are not supported.");
			}
			break;
		case VariantSerializationMode::Indexed:
		default:  // NOLINT(clang-diagnostic-covered-switch-default)
			if constexpr (canSerializeWithKey)
			{
				Detail::SerializeVariantIndexed(archive, value, variantOptions.indexKey, variantOptions.valueKey);
			}
			else
			{
				throw SerializationException(SerializationErrorCode::MismatchedTypes,
					"BitSerializer. VariantSerializationMode::Indexed requires every alternative to be serializable with a key in the current archive.");
			}
			break;
		}
	}
}
