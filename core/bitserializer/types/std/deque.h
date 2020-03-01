/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <deque>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
	/// <summary>
	/// Serialize std::deque with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TValue, typename TAllocator>
	bool Serialize(TArchive& archive, TKey&& key, std::deque<TValue, TAllocator>& cont)
	{
		return Detail::SerializeContainer(archive, std::forward<TKey>(key), cont);
	}
	
	/// <summary>
	/// Serialize std::deque.
	/// </summary>
	template<typename TArchive, typename TValue, typename TAllocator>
	void Serialize(TArchive& archive, std::deque<TValue, TAllocator>& cont)
	{
		Detail::SerializeContainer(archive, cont);
	}
}
