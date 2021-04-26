/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stack>
#include "bitserializer/serialization_detail/generic_container.h"

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::stack.
	/// </summary>
	template<typename TArchive, typename TValue, typename TContainer>
	void SerializeArray(TArchive& archive, std::stack<TValue, TContainer>& cont)
	{
		Detail::SerializeContainer(archive, Detail::GetBaseContainer(cont));
	}
}
