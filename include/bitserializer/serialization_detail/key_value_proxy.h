/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/serialization_detail/key_value.h"
#include "bitserializer/serialization_detail/attr_key_value.h"
#include "bitserializer/serialization_detail/archive_traits.h"
#include "bitserializer/serialization_detail/serialization_context.h"

namespace BitSerializer::KeyValueProxy
{
	template <class TArchive, class TValue>
	void SplitAndSerialize(TArchive& archive, TValue&& value)
	{
		Serialize(archive, value);
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

	template <class TArchive, class TAttrKey, class TValue, class... TValidators>
	static void SplitAndSerialize(TArchive& archive, AttributeValue<TAttrKey, TValue, TValidators...>&& keyValue)
	{
		constexpr auto hasSupportAttributes = BitSerializer::can_serialize_attribute_v<TArchive>;
		static_assert(hasSupportAttributes, "BitSerializer. The archive doesn't support serialization attribute (on current level or for format at all)");

		if constexpr (hasSupportAttributes)
		{
			auto attributesScope = archive.OpenAttributeScope();
			if (attributesScope) {
				SplitAndSerialize(*attributesScope, std::forward<KeyValue<TAttrKey, TValue, TValidators...>>(keyValue));
			}
		}
	}
}
