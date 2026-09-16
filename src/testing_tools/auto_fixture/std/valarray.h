/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <valarray>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::valarray`.
	 *
	 * @param fixture The fixture instance (propagated to nested elements).
	 * @param cont Reference to the `std::valarray`.
	 */
	template <typename T>
	void BuildFixture(Fixture& fixture, std::valarray<T>& cont)
	{
		cont.resize(fixture.GetContainerSize());
		for (size_t i = 0; i < fixture.GetContainerSize(); i++) {
			BuildFixture(fixture, cont[i]);
		}
	}
} // namespace AutoFixture
