/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
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

		// Validation when loading
		if constexpr (TArchive::IsLoading())
		{
			keyValue.VisitArgs([result, &keyValue, &archive](auto& handler)
			{
				using Type = std::decay_t<decltype(handler)>;
				constexpr auto isValidator = is_validator_v<Type, TValue>;
				static_assert(isValidator, "Unknown signature of passed KeyValue argument");

				if constexpr (isValidator)
				{
					if (auto validationError = handler(keyValue.GetValue(), result))
					{
						auto path = archive.GetPath() + TArchive::path_separator + Convert::ToString(keyValue.GetKey());
						archive.GetContext().AddValidationError(std::move(path), std::move(*validationError));
					}
				}
			});
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
}
