/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <forward_list>
#include "bitserializer/serialization_detail/generic_container.h"

namespace BitSerializer
{
	/// <summary>
	/// Serialize std::forward_list with key.
	/// </summary>	
	template<typename TArchive, typename TKey, typename TValue, typename TAllocator>
	bool Serialize(TArchive& archive, TKey&& key, std::forward_list<TValue, TAllocator>& cont)
	{
		return Detail::SerializeContainer(archive, std::forward<TKey>(key), cont);
	}

	/// <summary>
	/// Serialize std::forward_list.
	/// </summary>	
	template<typename TArchive, typename TValue, typename TAllocator>
	void Serialize(TArchive& archive, std::forward_list<TValue, TAllocator>& cont)
	{
		Detail::SerializeContainer(archive, cont);
	}
}
