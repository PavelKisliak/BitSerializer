/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <chrono>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::chrono::time_point`.
	 *
	 * @param fixture The fixture instance (provides the random engine).
	 * @param timePoint Reference to the `std::chrono::time_point`.
	 */
	template <typename TClock, typename TDuration>
	void BuildFixture(Fixture& fixture, std::chrono::time_point<TClock, TDuration>& timePoint)
	{
		constexpr auto tpMaxSec = std::chrono::time_point_cast<std::chrono::seconds>((std::chrono::time_point<TClock, TDuration>::max)())
			.time_since_epoch().count();
		constexpr auto tpMinSec = std::chrono::time_point_cast<std::chrono::seconds>((std::chrono::time_point<TClock, TDuration>::min)())
			.time_since_epoch().count();

		constexpr int64_t time_0000_01_01T00_00_00 = -62167219200;
		constexpr int64_t time_9999_12_31T23_59_59 = 253402300799;

		const int64_t seconds = fixture.Rand(
			(std::max)(tpMinSec, time_0000_01_01T00_00_00),
			(std::min)(tpMaxSec, time_9999_12_31T23_59_59));

		timePoint = std::chrono::time_point<TClock, TDuration>(std::chrono::duration_cast<TDuration>(std::chrono::seconds(seconds)));
	}

	/**
	 * @brief Builds a test fixture for `std::chrono::duration`.
	 *
	 * @param fixture The fixture instance (provides the random engine).
	 * @param duration Reference to the `std::chrono::duration`.
	 */
	template <typename TRep, typename TPeriod>
	void BuildFixture(Fixture& fixture, std::chrono::duration<TRep, TPeriod>& duration)
	{
		using TCommon = std::common_type_t<TRep, intmax_t>;
		duration = std::chrono::duration<TRep, TPeriod>(fixture.Rand<TCommon>(
			(std::numeric_limits<TRep>::min)(), (std::numeric_limits<TRep>::max)()));
	}
} // namespace AutoFixture
