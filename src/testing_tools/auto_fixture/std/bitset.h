/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <bitset>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::bitset`.
	 *
	 * @param fixture The fixture instance (used to generate random bits).
	 * @param cont Reference to the `std::bitset`.
	 */
	template <size_t Size>
	void BuildFixture(Fixture& fixture, std::bitset<Size>& cont)
	{
		for (size_t i = 0; i < Size; i++) {
			cont.set(i, fixture.Build<bool>());
		}
	}
} // namespace AutoFixture
