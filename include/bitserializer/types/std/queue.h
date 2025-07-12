/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <queue>
#include "bitserializer/serialization_detail/generic_container.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::queue`.
	 */
	template<typename TArchive, typename TValue, typename TContainer>
	void SerializeArray(TArchive& archive, std::queue<TValue, TContainer>& cont)
	{
		Detail::SerializeContainer(archive, Detail::GetBaseContainer(cont));
	}

	/**
	 * @brief Serializes `std::priority_queue`.
	 */
	template<typename TArchive, typename TValue, typename TContainer, typename TComparer>
	void SerializeArray(TArchive& archive, std::priority_queue<TValue, TContainer, TComparer>& cont)
	{
		Detail::SerializeContainer(archive, Detail::GetBaseContainer(cont));
	}
}
