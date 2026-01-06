/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/key_value.h"
#include "bitserializer/serialization_detail/attr_key_value.h"
#include "bitserializer/serialization_detail/archive_traits.h"
#include "bitserializer/serialization_detail/serialization_context.h"

namespace BitSerializer::KeyValueProxy
{
	/**
	 * @brief Serializes a plain value directly using the provided archive.
	 *
	 * This overload handles basic values that do not require key-value splitting.
	 *
	 * @tparam TArchive Type of the serialization archive.
	 * @tparam TValue   Type of the value being serialized.
	 * @param archive   Archive used for serialization.
	 * @param value     Value to serialize.
	 */
	template <class TArchive, class TValue>
	void SplitAndSerialize(TArchive& archive, TValue&& value)
	{
		Serialize(archive, value);
	}

	/**
	 * @brief Serializes a `KeyValue` wrapper by splitting its key and value components.
	 *
	 * Handles both saving and loading, including validator invocation on load.
	 *
	 * @tparam TArchive    Type of the serialization archive.
	 * @tparam TKey        Type of the key.
	 * @tparam TValue      Type of the value being serialized.
	 * @tparam TArgs       Types of extra parameters.
	 * @param archive      Archive used for serialization.
	 * @param keyValue     KeyValue wrapper containing key, value, and validators.
	 */
	template <class TArchive, class TKey, class TValue, class... TArgs>
	static void SplitAndSerialize(TArchive& archive, KeyValue<TKey, TValue, TArgs...>&& keyValue)
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
	 * @brief Serializes an `AttributeValue` wrapper by opening an attribute scope.
	 *
	 * Requires the archive to support attribute scopes.
	 *
	 * @tparam TArchive     Type of the serialization archive.
	 * @tparam TAttrKey     Type of the attribute key.
	 * @tparam TValue       Type of the attribute value.
	 * @tparam TArgs        Types of extra parameters.
	 * @param archive       Archive used for serialization.
	 * @param attrValuePair AttributeValue wrapper containing key, value, and validators.
	 */
	template <class TArchive, class TAttrKey, class TValue, class... TArgs>
	static void SplitAndSerialize(TArchive& archive, AttributeValue<TAttrKey, TValue, TArgs...>&& attrValuePair)
	{
		constexpr auto hasSupportAttributes = BitSerializer::can_serialize_attribute_v<TArchive>;
		static_assert(hasSupportAttributes, "BitSerializer. The archive doesn't support serialization attribute (on current level or for format at all)");

		if constexpr (hasSupportAttributes)
		{
			auto attributesScope = archive.OpenAttributeScope();
			if (attributesScope)
			{
				SplitAndSerialize(*attributesScope, std::forward<KeyValue<TAttrKey, TValue, TArgs...>>(attrValuePair));
			}
		}
	}

} // namespace BitSerializer::KeyValueProxy
