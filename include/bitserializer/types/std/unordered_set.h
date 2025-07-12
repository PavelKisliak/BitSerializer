/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <unordered_set>
#include "bitserializer/serialization_detail/generic_set.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::unordered_set`.
	 */
	template<typename TArchive, typename TValue, typename THasher, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& archive, std::unordered_set<TValue, THasher, TComparer, TAllocator>& cont)
	{
		Detail::SerializeSetImpl(archive, cont);
	}

	/**
	 * @brief Serializes `std::unordered_multiset`.
	 */
	template<typename TArchive, typename TValue, typename THasher, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& archive, std::unordered_multiset<TValue, THasher, TComparer, TAllocator>& cont)
	{
		Detail::SerializeSetImpl(archive, cont);
	}
}
