/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <deque>
#include "bitserializer/serialization_detail/generic_container.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::deque`.
	 */
	template<typename TArchive, typename TValue, typename TAllocator>
	void SerializeArray(TArchive& archive, std::deque<TValue, TAllocator>& cont)
	{
		Detail::SerializeContainer(archive, cont);
	}
}
