/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <utility>
#include "bitserializer/convert.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::pair`.
	 */
	template<typename TArchive, typename TFirst, typename TSecond>
	void SerializeObject(TArchive& archive, std::pair<TFirst, TSecond>& pair)
	{
		using value_type = std::pair<TFirst, TSecond>;
		using noConstKeyType = std::remove_const_t<typename value_type::first_type>;

		static const auto keyName = Convert::To<typename TArchive::key_type>("key");
		static const auto valueName = Convert::To<typename TArchive::key_type>("value");

		archive << KeyValue(keyName, const_cast<noConstKeyType&>(pair.first));
		archive << KeyValue(valueName, pair.second);
	}
}
