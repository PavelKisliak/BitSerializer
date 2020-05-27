/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stack>
#include "bitserializer/serialization_detail/generic_container.h"

namespace BitSerializer
{
	/// <summary>
	/// Serialize std::stack with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TValue, typename TContainer>
	bool Serialize(TArchive& archive, TKey&& key, std::stack<TValue, TContainer>& cont)
	{
		return Detail::SerializeContainer(archive, std::forward<TKey>(key), Detail::GetBaseContainer(cont));
	}

	/// <summary>
	/// Serialize std::stack.
	/// </summary>
	template<typename TArchive, typename TValue, typename TContainer>
	void Serialize(TArchive& archive, std::stack<TValue, TContainer>& cont)
	{
		Detail::SerializeContainer(archive, Detail::GetBaseContainer(cont));
	}
}
