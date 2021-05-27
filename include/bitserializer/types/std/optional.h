/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::optional.
	/// </summary>
	template <class TArchive, typename TKey, class TValue>
	bool Serialize(TArchive& archive, TKey&& key, std::optional<TValue>& optionalValue)
	{
		if constexpr (TArchive::IsLoading())
		{
			if (!optionalValue.has_value()) {
				optionalValue = TValue();
			}
			if (!Serialize(archive, std::forward<TKey>(key), optionalValue.value())) {
				optionalValue = std::nullopt;
			}
			return true;
		}
		else
		{
			if (optionalValue.has_value()) {
				return Serialize(archive, std::forward<TKey>(key), optionalValue.value());
			}

			std::nullptr_t value = nullptr;
			return Serialize(archive, std::forward<TKey>(key), value);
		}
	}

	template<typename TArchive, typename TValue>
	bool Serialize(TArchive& archive, std::optional<TValue>& optionalValue)
	{
		if constexpr (TArchive::IsLoading())
		{
			if (!optionalValue.has_value()) {
				optionalValue = TValue();
			}
			if (!Serialize(archive, optionalValue.value())) {
				optionalValue = std::nullopt;
			}
			return true;
		}
		else
		{
			if (optionalValue.has_value()) {
				return Serialize(archive, optionalValue.value());
			}

			std::nullptr_t value = nullptr;
			return Serialize(archive, value);
		}
	}
}
