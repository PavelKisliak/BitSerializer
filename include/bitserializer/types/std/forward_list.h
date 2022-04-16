/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <forward_list>

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::forward_list with key.
	/// </summary>
	template<typename TArchive, typename TValue, typename TAllocator>
	void SerializeArray(TArchive& arrayScope, std::forward_list<TValue, TAllocator>& cont)
	{
		if constexpr (TArchive::IsLoading())
		{
			if (const auto estimatedSize = arrayScope.GetEstimatedSize())
			{
				cont.resize(estimatedSize);
			}
			else if (cont.empty())
			{
				cont.resize(1);
			}

			// Load existing items
			size_t loadedItems = 0;
			auto LastIt = cont.begin();
			for (auto it = LastIt; it != cont.end() && !arrayScope.IsEnd(); ++it, ++loadedItems)
			{
				Serialize(arrayScope, *it);
				LastIt = it;
			}
			// Load all left items
			for (; !arrayScope.IsEnd(); ++loadedItems)
			{
				TValue value;
				Serialize(arrayScope, value);
				LastIt = cont.emplace_after(LastIt, std::move(value));
			}
			// Resize container for case when loaded less items than estimated
			cont.resize(loadedItems);
		}
		else
		{
			for (auto& value : cont)
			{
				Serialize(arrayScope, value);
			}
		}
	}
}
