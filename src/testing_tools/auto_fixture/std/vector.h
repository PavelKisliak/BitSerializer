/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <vector>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::vector<T>`.
	 *
	 * @param fixture The fixture instance (propagated to nested elements).
	 * @param cont Reference to the `std::vector`.
	 */
	template <typename T, typename TAllocator>
	void BuildFixture(Fixture& fixture, std::vector<T, TAllocator>& cont)
	{
		cont.resize(fixture.GetContainerSize());
		for (auto& elem : cont) {
			BuildFixture(fixture, elem);
		}
	}

	/**
	 * @brief Builds a test fixture for `std::vector<bool>`.
	 *
	 * @param fixture The fixture instance (propagated to nested elements).
	 * @param cont Reference to the `std::vector<bool>`.
	 */
	inline void BuildFixture(Fixture& fixture, std::vector<bool>& cont)
	{
		cont.resize(fixture.GetContainerSize());
		for (size_t i = 0; i < fixture.GetContainerSize(); i++)
		{
			bool elem{};
			BuildFixture(fixture, elem);
			cont[i] = elem;
		}
	}
} // namespace AutoFixture
