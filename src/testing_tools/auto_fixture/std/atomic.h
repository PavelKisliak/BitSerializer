/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <atomic>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for atomic types.
	 *
	 * @tparam T Atomic integral type.
	 * @param fixture The fixture instance.
	 * @param value Reference to the atomic variable to populate.
	 */
	template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	void BuildFixture(Fixture& fixture, std::atomic<T>& value)
	{
		T temp;
		BuildFixture(fixture, temp);
		value = temp;
	}
} // namespace AutoFixture
