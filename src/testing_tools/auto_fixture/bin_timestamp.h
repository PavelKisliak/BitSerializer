/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

#include "testing_tools/auto_fixture.h"

namespace std
{
	/**
	 * @brief Specialization of `std::numeric_limits<T>` for `CBinTimestamp`.
	 */
	template<>
	class numeric_limits<BitSerializer::Detail::CBinTimestamp>
	{
	public:
		static BitSerializer::Detail::CBinTimestamp (min)()
		{
			return BitSerializer::Detail::CBinTimestamp((std::numeric_limits<int64_t>::min)(), (std::numeric_limits<int32_t>::min)());
		}

		static BitSerializer::Detail::CBinTimestamp (max)()
		{
			return BitSerializer::Detail::CBinTimestamp((std::numeric_limits<int64_t>::max)(), (std::numeric_limits<int32_t>::max)());
		}
	};
}

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `CBinTimestamp`.
	 *
	 * @param fixture The fixture instance (propagated to nested fields).
	 * @param timestamp Reference to the `CBinTimestamp`.
	 */
	inline void BuildFixture(Fixture& fixture, BitSerializer::Detail::CBinTimestamp& timestamp)
	{
		BuildFixture(fixture, timestamp.seconds);
		BuildFixture(fixture, timestamp.nanoseconds);
	}
} // namespace AutoFixture
