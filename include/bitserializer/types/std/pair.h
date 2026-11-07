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
		if constexpr (TArchive::IsSaving())
		{
			archive << KeyValue("key", std::as_const(pair.first));
			archive << KeyValue("value", std::as_const(pair.second));
		}
		else
		{
			archive << KeyValue("key", pair.first);
			archive << KeyValue("value", pair.second);
		}
	}
}
