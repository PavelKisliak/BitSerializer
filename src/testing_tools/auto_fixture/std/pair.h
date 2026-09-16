/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <utility>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::pair` value.
	 *
	 * @param fixture The fixture instance (propagated to the pair elements).
	 * @param pair Reference to the `std::pair`.
	 */
	template <typename TKey, typename TValue>
	void BuildFixture(Fixture& fixture, std::pair<TKey, TValue>& pair)
	{
		BuildFixture(fixture, pair.first);
		BuildFixture(fixture, pair.second);
	}
} // namespace AutoFixture
