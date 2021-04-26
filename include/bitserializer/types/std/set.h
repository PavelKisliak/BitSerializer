/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <set>
#include "bitserializer/serialization_detail/generic_set.h"

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::set.
	/// </summary>
	template<typename TArchive, typename TValue, typename TAllocator>
	void SerializeArray(TArchive& archive, std::set<TValue, TAllocator>& cont)
	{
		Detail::SerializeSetImpl(archive, cont);
	}

	/// <summary>
	/// Serializes std::multiset.
	/// </summary>
	template<typename TArchive, typename TValue, typename TAllocator>
	void SerializeArray(TArchive& archive, std::multiset<TValue, TAllocator>& cont)
	{
		Detail::SerializeSetImpl(archive, cont);
	}
}
