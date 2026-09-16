/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <forward_list>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::forward_list`.
	 *
	 * @param fixture The fixture instance (propagated to nested elements).
	 * @param cont Reference to the `std::forward_list`.
	 */
	template <typename T, typename TAllocator>
	void BuildFixture(Fixture& fixture, std::forward_list<T, TAllocator>& cont)
	{
		cont.resize(fixture.GetContainerSize());
		for (auto& elem : cont) {
			BuildFixture(fixture, elem);
		}
	}
} // namespace AutoFixture
