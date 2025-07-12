/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <array>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::array`.
	 */
	template<typename TArchive, typename TValue, size_t ArraySize>
	void SerializeArray(TArchive& archive, std::array<TValue, ArraySize>& cont)
	{
		Detail::SerializeFixedSizeArray(archive, std::begin(cont), std::end(cont));
	}
}
