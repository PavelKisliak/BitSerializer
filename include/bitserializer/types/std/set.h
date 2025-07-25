/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <set>
#include "bitserializer/serialization_detail/generic_set.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::set`.
	 */
	template<typename TArchive, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& archive, std::set<TValue, TComparer, TAllocator>& cont)
	{
		Detail::SerializeSetImpl(archive, cont);
	}

	/**
	 * @brief Serializes `std::multiset`.
	 */
	template<typename TArchive, typename TValue, typename TComparer, typename TAllocator>
	void SerializeArray(TArchive& archive, std::multiset<TValue, TComparer, TAllocator>& cont)
	{
		Detail::SerializeSetImpl(archive, cont);
	}
}
