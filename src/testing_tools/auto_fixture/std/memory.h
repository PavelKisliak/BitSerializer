/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::unique_ptr<TValue>`.
	 *
	 * @param fixture The fixture instance (propagated to the pointed value).
	 * @param uniquePtr Reference to the `std::unique_ptr<TValue>`.
	 */
	template <typename TValue>
	void BuildFixture(Fixture& fixture, std::unique_ptr<TValue>& uniquePtr)
	{
		uniquePtr = std::make_unique<TValue>();
		BuildFixture(fixture, *uniquePtr);
	}

	/**
	 * @brief Builds a test fixture for `std::shared_ptr<TValue>`.
	 *
	 * @param fixture The fixture instance (propagated to the pointed value).
	 * @param sharedPtr Reference to the `std::shared_ptr<TValue>`.
	 */
	template <typename TValue>
	void BuildFixture(Fixture& fixture, std::shared_ptr<TValue>& sharedPtr)
	{
		sharedPtr = std::make_shared<TValue>();
		BuildFixture(fixture, *sharedPtr);
	}
} // namespace AutoFixture
