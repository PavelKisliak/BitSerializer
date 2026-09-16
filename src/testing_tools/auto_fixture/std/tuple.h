/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::tuple` value.
	 *
	 * @param fixture The fixture instance (propagated to the tuple elements).
	 * @param value Reference to the `std::tuple`.
	 */
	template <typename ...TArgs>
	void BuildFixture(Fixture& fixture, std::tuple<TArgs...>& value)
	{
		std::apply([&fixture](auto&&... args) {
			((BuildFixture(fixture, args)), ...);
		}, value);
	}
} // namespace AutoFixture
