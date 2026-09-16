/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::optional<TValue>` value.
	 *
	 * @param fixture The fixture instance (propagated to the contained value).
	 * @param optionalValue Reference to the `std::optional<TValue>`.
	 */
	template <typename TValue>
	void BuildFixture(Fixture& fixture, std::optional<TValue>& optionalValue)
	{
		TValue& value = optionalValue.emplace();
		BuildFixture(fixture, value);
	}
} // namespace AutoFixture
