/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <variant>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	namespace Detail
	{
		template <size_t TIndex = 0, typename... TArgs>
		void BuildVariantFixture(Fixture& fixture, std::variant<TArgs...>& value, size_t activeIndex)
		{
			if constexpr (TIndex < sizeof...(TArgs))
			{
				if (activeIndex == TIndex)
				{
					value.template emplace<TIndex>();
					BuildFixture(fixture, std::get<TIndex>(value));
				}
				else
				{
					BuildVariantFixture<TIndex + 1>(fixture, value, activeIndex);
				}
			}
		}
	}

	/**
	 * @brief Builds a test fixture for `std::variant` value.
	 *
	 * Randomly selects one of the alternatives and initializes it.
	 *
	 * @param fixture The fixture instance (propagated to the active alternative).
	 * @param value Reference to the `std::variant`.
	 */
	template <typename ...TArgs>
	void BuildFixture(Fixture& fixture, std::variant<TArgs...>& value)
	{
		static_assert(sizeof...(TArgs) > 0);
		const size_t activeIndex = fixture.Rand() % sizeof...(TArgs);
		Detail::BuildVariantFixture(fixture, value, activeIndex);
	}
} // namespace AutoFixture
