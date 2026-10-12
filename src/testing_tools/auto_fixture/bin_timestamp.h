/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

#include "testing_tools/auto_fixture.h"

namespace std
{
	/**
	 * @brief Specialization of `std::numeric_limits<T>` for `BinTimestamp`.
	 */
	template<>
	class numeric_limits<BitSerializer::Detail::BinTimestamp>
	{
	public:
		static BitSerializer::Detail::BinTimestamp (min)()
		{
			return BitSerializer::Detail::BinTimestamp((std::numeric_limits<int64_t>::min)(), (std::numeric_limits<int32_t>::min)());
		}

		static BitSerializer::Detail::BinTimestamp (max)()
		{
			return BitSerializer::Detail::BinTimestamp((std::numeric_limits<int64_t>::max)(), (std::numeric_limits<int32_t>::max)());
		}
	};
}

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `BinTimestamp`.
	 *
	 * @param fixture The fixture instance (propagated to nested fields).
	 * @param timestamp Reference to the `BinTimestamp`.
	 */
	inline void BuildFixture(Fixture& fixture, BitSerializer::Detail::BinTimestamp& timestamp)
	{
		BuildFixture(fixture, timestamp.seconds);
		BuildFixture(fixture, timestamp.nanoseconds);
	}
} // namespace AutoFixture
