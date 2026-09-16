/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <queue>
#include <stack>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::queue`.
	 *
	 * @param fixture The fixture instance (propagated to nested elements).
	 * @param cont Reference to the `std::queue`.
	 */
	template <typename T>
	void BuildFixture(Fixture& fixture, std::queue<T>& cont)
	{
		for (size_t i = 0; i < fixture.GetContainerSize(); i++) {
			cont.push(fixture.Build<T>());
		}
	}

	/**
	 * @brief Builds a test fixture for `std::priority_queue`.
	 *
	 * @param fixture The fixture instance (propagated to nested elements).
	 * @param cont Reference to the `std::priority_queue`.
	 */
	template <typename T>
	void BuildFixture(Fixture& fixture, std::priority_queue<T>& cont)
	{
		for (size_t i = 0; i < fixture.GetContainerSize(); i++) {
			cont.push(fixture.Build<T>());
		}
	}

	/**
	 * @brief Builds a test fixture for `std::stack`.
	 *
	 * @param fixture The fixture instance (propagated to nested elements).
	 * @param cont Reference to the `std::stack`.
	 */
	template <typename T>
	void BuildFixture(Fixture& fixture, std::stack<T>& cont)
	{
		for (size_t i = 0; i < fixture.GetContainerSize(); i++) {
			cont.push(fixture.Build<T>());
		}
	}
} // namespace AutoFixture
