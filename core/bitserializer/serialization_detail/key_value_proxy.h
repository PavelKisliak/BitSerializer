/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "key_value.h"
#include "attr_key_value.h"
#include "archive_traits.h"
#include "serialization_context.h"

namespace BitSerializer::KeyValueProxy
{
	template <class TArchive, class TValue>
	void SplitAndSerialize(TArchive& archive, TValue&& value)
	{
		Serialize(archive, std::forward<TValue>(value));
	}

	template <class TArchive, class TKey, class TValue, class... TValidators>
	static void SplitAndSerialize(TArchive& archive, KeyValue<TKey, TValue, TValidators...>&& keyValue)
	{
		constexpr auto hasSupportKeyType = BitSerializer::is_type_convertible_to_one_from_tuple_v<TKey, typename TArchive::supported_key_types>;
		static_assert(hasSupportKeyType, "BitSerializer. The archive doesn't support this key type.");

		const bool result = Serialize(archive, keyValue.GetKey(), keyValue.GetValue());

		// Validation when loading
		if constexpr (archive.IsLoading())
		{
			auto validationResult = keyValue.ValidateValue(result);
			if (validationResult.has_value())
			{
				auto path = archive.GetPath() + TArchive::path_separator + Convert::ToString(keyValue.GetKey());
				Context.AddValidationErrors(std::move(path), std::move(*validationResult));
			}
		}
	}

	template <class TArchive, class TKey, class TValue, class... TValidators>
	static void SplitAndSerialize(TArchive& archive, AutoKeyValue<TKey, TValue, TValidators...>&& keyValue)
	{
		// Checks key type and adapts it to archive if needed
		if constexpr (std::is_convertible_v<TKey, typename TArchive::key_type>)
			SplitAndSerialize(archive, std::forward<KeyValue<TKey, TValue, TValidators...>>(keyValue));
		else
			SplitAndSerialize(archive, keyValue.template AdaptAndMoveToBaseKeyValue<typename TArchive::key_type>());
	}

	template <class TArchive, class TAttrKey, class TValue, class... TValidators>
	static void SplitAndSerialize(TArchive& archive, AttributeValue<TAttrKey, TValue, TValidators...>&& keyValue)
	{
		constexpr auto hasSupportAttributes = BitSerializer::can_serialize_attribute_v<TArchive>;
		static_assert(hasSupportAttributes, "BitSerializer. The archive doesn't support serialization attribute (on current level or for format at all)");

		if constexpr (hasSupportAttributes)
		{
			auto attributesScope = archive.OpenAttributeScope();
			if (attributesScope)
				SplitAndSerialize(*attributesScope, std::forward<KeyValue<TAttrKey, TValue, TValidators...>>(keyValue));
		}
	}

	template <class TArchive, class TAttrKey, class TValue, class... TValidators>
	static void SplitAndSerialize(TArchive& archive, AutoAttributeValue<TAttrKey, TValue, TValidators...>&& keyValue)
	{
		// Checks key type and adapts it to archive if needed
		if constexpr (std::is_convertible_v<TAttrKey, typename TArchive::key_type>)
			SplitAndSerialize(archive, std::forward<AttributeValue<TAttrKey, TValue, TValidators...>>(keyValue));
		else
			SplitAndSerialize(archive, keyValue.template AdaptAndMoveToBaseAttributeValue<typename TArchive::key_type>());
	}

}	// namespace BitSerializer::KeyValueProxy