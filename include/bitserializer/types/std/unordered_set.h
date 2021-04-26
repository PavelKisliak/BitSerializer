/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <unordered_set>
#include "bitserializer/serialization_detail/generic_set.h"

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::unordered_set.
	/// </summary>
	template<typename TArchive, typename TValue, typename THasher, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& archive, std::unordered_set<TValue, THasher, TComparer, TAllocator>& cont)
	{
		Detail::SerializeSetImpl(archive, cont);
	}
}
