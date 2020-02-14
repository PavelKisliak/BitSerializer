/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <array>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
	/// <summary>
	/// Serialize std::array with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TValue, size_t ArraySize>
	bool Serialize(TArchive& archive, TKey&& key, std::array<TValue, ArraySize>& cont)
	{
		return Detail::SerializeContainer(archive, key, cont);
	}

	/// <summary>
	/// Serialize std::array.
	/// </summary>
	template<typename TArchive, typename TValue, size_t ArraySize>
	void Serialize(TArchive& archive, std::array<TValue, ArraySize>& cont)
	{
		Detail::SerializeContainer(archive, cont);
	}
}
