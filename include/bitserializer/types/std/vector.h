/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <vector>
#include "bitserializer/serialization_detail/generic_container.h"

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::vector.
	/// </summary>
	template<typename TArchive, typename TValue, typename TAllocator>
	void SerializeArray(TArchive& archive, std::vector<TValue, TAllocator>& cont)
	{
		Detail::SerializeContainer(archive, cont);
	}

	/// <summary>
	/// Serializes std::vector of boolean types.
	/// </summary>
	template<typename TArchive, typename TAllocator>
	void SerializeArray(TArchive& archive, std::vector<bool, TAllocator>& cont)
	{
		if constexpr (TArchive::IsLoading())
		{
			// Resize container when is known approximate size
			if (const auto estimatedSize = archive.GetEstimatedSize(); estimatedSize != 0)
			{
				cont.resize(estimatedSize);
			}

			// Load existing items
			bool value = false;
			size_t loadedItems = 0;
			for (auto it = cont.begin(); it != cont.end() && !archive.IsEnd(); ++it, ++loadedItems)
			{
				Serialize(archive, value);
				*it = value;
			}
			// Load all left items
			for (; !archive.IsEnd(); ++loadedItems)
			{
				Serialize(archive, value);
				cont.push_back(value);
			}
			// Resize container for case when loaded less items than estimated
			cont.resize(loadedItems);
		}
		else
		{
			for (bool value : cont)
			{
				Serialize(archive, value);
			}
		}
	}
}
