/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <queue>
#include "bitserializer/serialization_detail/generic_container.h"

namespace BitSerializer
{
	/// <summary>
	/// Serialize std::queue with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TValue, typename TContainer>
	bool Serialize(TArchive& archive, TKey&& key, std::queue<TValue, TContainer>& cont)
	{
		return Detail::SerializeContainer(archive, std::forward<TKey>(key), Detail::GetBaseContainer(cont));
	}

	/// <summary>
	/// Serialize std::queue.
	/// </summary>
	template<typename TArchive, typename TValue, typename TContainer>
	void Serialize(TArchive& archive, std::queue<TValue, TContainer>& cont)
	{
		Detail::SerializeContainer(archive, Detail::GetBaseContainer(cont));
	}

	//-----------------------------------------------------------------------------

	/// <summary>
	/// Serialize std::priority_queue with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TValue, typename TContainer, typename TComparer>
	bool Serialize(TArchive& archive, TKey&& key, std::priority_queue<TValue, TContainer, TComparer>& cont)
	{
		return Detail::SerializeContainer(archive, std::forward<TKey>(key), Detail::GetBaseContainer(cont));
	}

	/// <summary>
	/// Serialize std::priority_queue.
	/// </summary>
	template<typename TArchive, typename TValue, typename TContainer, typename TComparer>
	void Serialize(TArchive& archive, std::priority_queue<TValue, TContainer, TComparer>& cont)
	{
		Detail::SerializeContainer(archive, Detail::GetBaseContainer(cont));
	}
}
