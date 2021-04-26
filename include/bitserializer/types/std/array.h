/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <array>
#include "bitserializer/serialization_detail/generic_container.h"

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::array.
	/// </summary>
	template<typename TArchive, typename TValue, size_t ArraySize>
	void SerializeArray(TArchive& archive, std::array<TValue, ArraySize>& cont)
	{
		return Detail::SerializeContainer(archive, cont);
	}
}
