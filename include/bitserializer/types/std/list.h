/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <list>
#include "bitserializer/serialization_detail/generic_container.h"

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::list.
	/// </summary>
	template<typename TArchive, typename TValue, typename TAllocator>
	void SerializeArray(TArchive& archive, std::list<TValue, TAllocator>& cont)
	{
		Detail::SerializeContainer(archive, cont);
	}
}
