/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <array>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::array`.
	 *
	 * @param fixture The fixture instance (propagated to nested elements).
	 * @param cont Reference to the `std::array`.
	 */
	template <typename T, size_t Size>
	void BuildFixture(Fixture& fixture, std::array<T, Size>& cont)
	{
		for (size_t i = 0; i < Size; i++) {
			BuildFixture(fixture, cont[i]);
		}
	}
} // namespace AutoFixture
