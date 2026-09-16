/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <map>
#include <unordered_map>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::map`.
	 *
	 * @param fixture The fixture instance (propagated to keys and values).
	 * @param cont Reference to the `std::map`.
	 */
	template <typename TKey, typename TValue, typename TComparer, typename TAllocator>
	void BuildFixture(Fixture& fixture, std::map<TKey, TValue, TComparer, TAllocator>& cont)
	{
		cont.clear();
		for (size_t i = 0; i < fixture.GetContainerSize(); i++)
		{
			TValue value;
			BuildFixture(fixture, value);
			cont.emplace(fixture.Build<TKey>(), std::move(value));
		}
	}

	/**
	 * @brief Builds a test fixture for `std::multimap` (allows duplicate elements).
	 *
	 * @param fixture The fixture instance (propagated to keys and values).
	 * @param cont Reference to the `std::multimap`.
	 */
	template <typename TKey, typename TValue, typename TComparer, typename TAllocator>
	void BuildFixture(Fixture& fixture, std::multimap<TKey, TValue, TComparer, TAllocator>& cont)
	{
		cont.clear();
		TKey key;
		for (size_t i = 0; i < fixture.GetContainerSize(); i++)
		{
			if (i % 2 == 0) {
				BuildFixture(fixture, key);
			}
			TValue value;
			BuildFixture(fixture, value);
			cont.emplace(key, std::move(value));
		}
	}

	/**
	 * @brief Builds a test fixture for `std::unordered_map`.
	 *
	 * @param fixture The fixture instance (propagated to keys and values).
	 * @param cont Reference to the `std::unordered_map`.
	 */
	template <typename TKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void BuildFixture(Fixture& fixture, std::unordered_map<TKey, TValue, THasher, TKeyEq, TAllocator>& cont)
	{
		cont.clear();
		for (size_t i = 0; i < fixture.GetContainerSize(); i++)
		{
			TValue value;
			BuildFixture(fixture, value);
			cont.emplace(fixture.Build<TKey>(), std::move(value));
		}
	}

	/**
	 * @brief Builds a test fixture for `std::unordered_multimap` (allows duplicate elements).
	 *
	 * @param fixture The fixture instance (propagated to keys and values).
	 * @param cont Reference to the `std::unordered_multimap`.
	 */
	template <typename TKey, typename TValue, typename THasher, typename TKeyEq, typename TAllocator>
	void BuildFixture(Fixture& fixture, std::unordered_multimap<TKey, TValue, THasher, TKeyEq, TAllocator>& cont)
	{
		cont.clear();
		TKey key;
		for (size_t i = 0; i < fixture.GetContainerSize(); i++)
		{
			if (i % 2 == 0) {
				BuildFixture(fixture, key);
			}
			TValue value;
			BuildFixture(fixture, value);
			cont.emplace(key, std::move(value));
		}
	}
} // namespace AutoFixture
