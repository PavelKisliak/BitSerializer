/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <valarray>
#include <vector>
#include "bitserializer/serialization_detail/generic_container.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::valarray`.
	 */
	template<typename TArchive, typename TValue>
	void SerializeArray(TArchive& archive, std::valarray<TValue>& cont)
	{
		if constexpr (TArchive::IsLoading())
		{
			// Use std::vector as temp buffer due to unknown size of loading elements
			std::vector<TValue> temp;
			Detail::SerializeContainer(archive, temp);

			cont.resize(temp.size());
			for (size_t i = 0; i < temp.size(); ++i)
			{
				cont[i] = std::move(temp[i]);
			}
		}
		else
		{
			for (auto& value : cont)
			{
				Serialize(archive, value);
			}
		}
	}
}
