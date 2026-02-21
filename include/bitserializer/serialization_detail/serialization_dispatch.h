/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/key_value.h"
#include "bitserializer/serialization_detail/archive_traits.h"

namespace BitSerializer::Detail
{
	/**
	 * @brief Dispatches serialization of an arbitrary value to the archive.
	 *
	 * This overload handles values without explicit keys (e.g., array elements, root objects).
	 *
	 * @tparam TArchive Type of the serialization archive.
	 * @tparam TValue   Type of the value being serialized.
	 * @param archive   Archive used for serialization.
	 * @param value     Value to dispatch (consumed via perfect forwarding).
	 */
	template <class TArchive, class TValue>
	void Dispatch(TArchive& archive, TValue&& value)
	{
		Serialize(archive, value);
	}

	/**
	 * @brief Dispatches serialization of a key-value pair with optional validators/refiners.
	 *
	 * Handles both saving and loading modes:
	 * - During saving: Serializes key (with automatic transcoding to archive's key_type if needed) and value.
	 * - During loading: Deserializes value, then applies refiners and validators. Validation errors are collected in the archive context.
	 *
	 * @tparam TArchive Type of the serialization archive.
	 * @tparam TKey     Type of the key (any string-like or convertible type).
	 * @tparam TValue   Type of the value being serialized.
	 * @tparam TArgs    Types of validators/refiners attached to the pair.
	 * @param archive   Archive used for serialization.
	 * @param keyValue  KeyValue wrapper (key + value + metadata).
	 */
	template <class TArchive, class TKey, class TValue, class... TArgs>
	void Dispatch(TArchive& archive, KeyValue<TKey, TValue, TArgs...>&& keyValue)
	{
		bool result;
		if constexpr (BitSerializer::is_convertible_to_one_from_tuple_v<TKey, typename TArchive::supported_key_types>)
		{
			result = Serialize(archive, keyValue.GetKey(), keyValue.GetValue());
		}
		else
		{
			const auto key = Convert::To<typename TArchive::key_type>(keyValue.GetKey());
			result = Serialize(archive, key, keyValue.GetValue());
		}

		// Handle validation only during loading
		if constexpr (TArchive::IsLoading())
		{
			keyValue.VisitArgs([result, &keyValue, &archive](auto& handler)
			{
				using HandlerType = std::decay_t<decltype(handler)>;
				constexpr auto isValidator = is_validator_v<HandlerType, TValue>;
				constexpr auto isRefiner = is_refiner_v<HandlerType, TValue>;
				static_assert(isValidator || isRefiner, "BitSerializer. Unknown signature of argument passed to KeyValue");

				if constexpr (isValidator)
				{
					if (auto validationError = handler(keyValue.GetValue(), result))
					{
						auto path = archive.GetPath() + TArchive::path_separator + Convert::ToString(keyValue.GetKey());
						archive.GetContext().AddValidationError(std::move(path), std::move(*validationError));
					}
				}
				else if constexpr (isRefiner)
				{
					handler(keyValue.GetValue(), result);
				}
			});
		}
	}

	/**
	 * @brief Dispatches serialization of an attribute-value pair.
	 *
	 * Requires the archive to support attribute scopes (e.g., XML attributes).
	 *
	 * @tparam TArchive     Type of the serialization archive.
	 * @tparam TAttrKey     Type of the attribute key.
	 * @tparam TValue       Type of the attribute value.
	 * @tparam TArgs        Types of validators/refiners (applied during loading).
	 * @param archive       Archive used for serialization.
	 * @param attrValue     AttributeValue wrapper (key + value + metadata).
	 */
	template <class TArchive, class TAttrKey, class TValue, class... TArgs>
	void Dispatch(TArchive& archive, AttributeValue<TAttrKey, TValue, TArgs...>&& attrValue)
	{
		constexpr auto hasSupportAttributes = BitSerializer::can_serialize_attribute_v<TArchive>;
		static_assert(hasSupportAttributes, "BitSerializer. The archive doesn't support serialization attribute (on current level or for format at all)");

		if constexpr (hasSupportAttributes)
		{
			auto attributesScope = archive.OpenAttributeScope();
			if (attributesScope)
			{
				Dispatch(*attributesScope, std::forward<KeyValue<TAttrKey, TValue, TArgs...>>(attrValue));
			}
		}
	}

	/**
	 * @brief Dispatches serialization of a property-value pair with automatic adaptation.
	 *
	 * Decision matrix:
	 * - XML + String-convertible type	-> Attribute
	 * - XML + Non-convertible type		-> Element (KeyValue)
	 * - Non-XML formats				-> Key-Value pair
	 *
	 * @tparam TArchive     Type of the serialization archive.
	 * @tparam TKey         Type of the property key.
	 * @tparam TValue       Type of the property value.
	 * @tparam TArgs        Types of validators/refiners (applied during loading).
	 * @param archive       Archive used for serialization.
	 * @param propValue     PropertyValue wrapper (key + value + metadata).
	 */
	template <class TArchive, class TKey, class TValue, class... TArgs>
	void Dispatch(TArchive& archive, PropertyValue<TKey, TValue, TArgs...>&& propValue)
	{
		constexpr auto hasSupportAttributes = BitSerializer::can_serialize_attribute_v<TArchive>;
		constexpr auto isConvertible = Convert::IsConvertible<TValue, typename TArchive::key_type>();

		if constexpr (isConvertible && hasSupportAttributes)
		{
			Dispatch(archive, std::forward<AttributeValue<TKey, TValue, TArgs...>>(propValue));
		}
		else
		{
			Dispatch(archive, std::forward<KeyValue<TKey, TValue, TArgs...>>(propValue));
		}
	}

} // namespace BitSerializer::Detail
