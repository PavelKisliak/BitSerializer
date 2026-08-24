/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <utility>

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::pair`.
	 */
	template<typename TArchive, typename TFirst, typename TSecond>
	void SerializeObject(TArchive& archive, std::pair<TFirst, TSecond>& pair)
	{
		archive << KeyValue("key", const_cast<std::remove_const_t<TFirst>&>(pair.first));
		archive << KeyValue("value", pair.second);
	}
}
