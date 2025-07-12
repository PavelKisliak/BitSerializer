/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <utility>

namespace BitSerializer::Detail
{
	/**
	 * @brief Generic function for serializing set-like containers.
	 *
	 * Supports both loading and saving operations depending on the archive mode.
	 *
	 * When loading:
	 * - Clears the container.
	 * - Reads values from the archive and inserts them into the set.
	 * - Uses insertion hinting for performance optimization.
	 *
	 * When saving:
	 * - Iterates through all elements and serializes them.
	 *
	 * @tparam TArchive The archive type used for serialization.
	 * @tparam TSet     Type of the set container (e.g., `std::set`, `std::unordered_set`).
	 * @param scope     Archive scope object used for serializing set content.
	 * @param cont      Reference to the set being serialized.
	 */
	template<typename TArchive, typename TSet>
	static void SerializeSetImpl(TArchive& scope, TSet& cont)
	{
		using TValue = typename TSet::value_type;

		if constexpr (TArchive::IsLoading())
		{
			cont.clear();
			auto hint = cont.begin();
			while (!scope.IsEnd())
			{
				TValue value;
				Serialize(scope, value);
				hint = cont.insert(hint, std::move(value));
			}
		}
		else
		{
			for (auto& elem : cont)
			{
				Serialize(scope, elem);
			}
		}
	}
} // namespace BitSerializer::Detail
