/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <set>
#include <unordered_set>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::set`.
	 *
	 * @param fixture The fixture instance (propagated to elements).
	 * @param cont Reference to the `std::set`.
	 */
	template <typename T, typename TComparer, typename TAllocator>
	void BuildFixture(Fixture& fixture, std::set<T, TComparer, TAllocator>& cont)
	{
		cont.clear();
		for (size_t i = 0; i < fixture.GetContainerSize(); i++) {
			T element;
			BuildFixture(fixture, element);
			cont.emplace(std::move(element));
		}
	}

	/**
	 * @brief Builds a test fixture for `std::multiset` (allows duplicate elements).
	 *
	 * @param fixture The fixture instance (propagated to elements).
	 * @param cont Reference to the `std::multiset`.
	 */
	template <typename T, typename TComparer, typename TAllocator>
	void BuildFixture(Fixture& fixture, std::multiset<T, TComparer, TAllocator>& cont)
	{
		cont.clear();
		T element;
		for (size_t i = 0; i < fixture.GetContainerSize(); i++)
		{
			// Duplicated element
			if (i % 2 == 0) {
				BuildFixture(fixture, element);
			}
			cont.insert(element);
		}
	}

	/**
	 * @brief Builds a test fixture for `std::unordered_set`.
	 *
	 * @param fixture The fixture instance (propagated to elements).
	 * @param cont Reference to the `std::unordered_set`.
	 */
	template <typename T, typename THasher, typename TKeyEq, typename TAllocator>
	void BuildFixture(Fixture& fixture, std::unordered_set<T, THasher, TKeyEq, TAllocator>& cont)
	{
		cont.clear();
		for (size_t i = 0; i < fixture.GetContainerSize(); i++)
		{
			T element;
			BuildFixture(fixture, element);
			cont.emplace(std::move(element));
		}
	}

	/**
	 * @brief Builds a test fixture for `std::unordered_multiset` (allows duplicate elements).
	 *
	 * @param fixture The fixture instance (propagated to elements).
	 * @param cont Reference to the `std::unordered_multiset`.
	 */
	template <typename T, typename THasher, typename TKeyEq, typename TAllocator>
	void BuildFixture(Fixture& fixture, std::unordered_multiset<T, THasher, TKeyEq, TAllocator>& cont)
	{
		cont.clear();
		T element;
		for (size_t i = 0; i < fixture.GetContainerSize(); i++)
		{
			// Duplicated element
			if (i % 2 == 0) {
				BuildFixture(fixture, element);
			}
			cont.insert(element);
		}
	}
} // namespace AutoFixture
