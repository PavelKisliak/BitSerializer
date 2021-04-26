/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
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
		if constexpr (TArchive::IsLoading()) {
			cont.resize(archive.GetSize());
		}
		bool value = false;
		const auto size = cont.size();
		for (size_t i = 0; i < size; i++)
		{
			if constexpr (TArchive::IsLoading()) {
				Serialize(archive, value);
				cont[i] = value;
			}
			else
			{
				value = cont[i];
				Serialize(archive, value);
			}
		}
	}
}
