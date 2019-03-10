/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "key_value.h"
#include "archive_traits.h"
#include "bitserializer/string_conversion.h"
#include "serialization_context.h"

namespace BitSerializer::KeyValueProxy
{
	template <class TArchive, class TValue, std::enable_if_t<is_archive_scope_v<TArchive>, int> = 0>
	inline void SplitAndSerialize(TArchive& archive, TValue&& value)
	{
		Serialize(archive, std::forward<TValue>(value));
	}

	template <class TArchive, class TKey, class TValue, class... Validators, std::enable_if_t<is_archive_scope_v<TArchive>, int> = 0>
	static void SplitAndSerialize(TArchive& archive, KeyValue<TKey, TValue, Validators...>&& keyValue)
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
				auto path = archive.GetPath() + TArchive::path_separator + Convert::ToWString(keyValue.GetKey());
				Context.AddValidationErrors(path, std::move(*validationResult));
			}
		}
	}

	template <class TArchive, class TKey, class TValue, class... Validators, std::enable_if_t<is_archive_scope_v<TArchive>, int> = 0>
	static void SplitAndSerialize(TArchive& archive, AutoKeyValue<TKey, TValue, Validators...>&& keyValue)
	{
		// Checks key type and adapts it to archive if needed
		if constexpr (std::is_convertible_v<TKey, typename TArchive::key_type>)
			archive << std::forward<KeyValue<TKey, TValue, Validators...>>(keyValue);
		else
			archive << keyValue.template AdaptAndMoveToBaseKeyValue<typename TArchive::key_type>();
	}

}	// namespace BitSerializer::KeyValueProxy