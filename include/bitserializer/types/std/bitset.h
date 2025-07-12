/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <bitset>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::bitset`.
	 */
	template<typename TArchive, size_t Size>
	void SerializeArray(TArchive& archive, std::bitset<Size>& cont)
	{
		bool value = false;
		for (size_t i = 0; i < Size; i++)
		{
			if constexpr (TArchive::IsLoading())
			{
				Serialize(archive, value);
				cont.set(i, value);
			}
			else
			{
				value = cont.test(i);
				Serialize(archive, value);
			}
		}
	}
}
