/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/conversion_detail/convert_chrono.h"

namespace BitSerializer::Detail
{
	/**
	 * @brief Internal representation of a timestamp used in binary archives (like as MsgPack).
	 *
	 * This structure holds time data as seconds and nanoseconds components,
	 * suitable for serialization into formats that require fixed-size binary layout.
	 *
	 * If the value is too large, a `SerializationException` with code `Overflow` may be thrown during serialization.
	 */
	struct CBinTimestamp
	{
		CBinTimestamp() = default;

		/**
		 * @brief Constructs a timestamp from seconds and optional nanoseconds.
		 *
		 * @param[in] inSeconds     Number of seconds since epoch.
		 * @param[in] inNanoseconds Nanoseconds fraction (must be between 0 and 999,999,999).
		 */
		explicit CBinTimestamp(int64_t inSeconds, int32_t inNanoseconds = 0)
			: seconds(inSeconds)
			, nanoseconds(inNanoseconds)
		{
		}

		/**
		 * @brief Equality comparison operator.
		 */
		bool operator==(const CBinTimestamp& rhs) const
		{
			return seconds == rhs.seconds && nanoseconds == rhs.nanoseconds;
		}

		/**
		 * @brief Converts the timestamp to a string representation.
		 */
		[[nodiscard]] std::string ToString() const
		{
			return std::to_string(seconds) + " " + std::to_string(nanoseconds);
		}

		int64_t seconds{};		///< Total seconds since epoch.
		int32_t nanoseconds{};	///< nanoseconds fraction (must not exceed 999,999,999).
	};

	//-----------------------------------------------------------------------------

	/**
	 * @brief Converts a `std::chrono::time_point` to a `CBinTimestamp`.
	 *
	 * @param timePoint    Input time point to convert.
	 * @param outTimestamp Output timestamp to populate.
     * @throws std::out_of_range if overflow occurs during conversion.
	 */
	template <typename TClock, typename TDuration>
	void To(const std::chrono::time_point<TClock, TDuration>& timePoint, CBinTimestamp& outTimestamp)
	{
		const auto epochTime = timePoint.time_since_epoch();
		if constexpr (std::ratio_greater_equal_v<typename TDuration::period, std::chrono::seconds::period>)
		{
			// Duration precision is equal or coarser than seconds
			outTimestamp.seconds = Convert::Detail::SafeDurationCast<std::chrono::seconds>(epochTime).count();
			outTimestamp.nanoseconds = 0;
		}
		else
		{
			// Duration has finer precision than seconds
			outTimestamp.seconds = std::chrono::duration_cast<std::chrono::seconds>(epochTime).count();
			const auto leftTime = epochTime - std::chrono::duration_cast<TDuration>(std::chrono::seconds(outTimestamp.seconds));
			outTimestamp.nanoseconds = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(leftTime).count());
		}
	}

	/**
	 * @brief Converts a `CBinTimestamp` to a `std::chrono::time_point`.
	 *
	 * @param timestamp    Input binary timestamp.
	 * @param outTimePoint Output time point to populate.
	 * @throws std::out_of_range if overflow occurs during conversion.
	 */
	template <typename TClock, typename TDuration>
	void To(const CBinTimestamp& timestamp, std::chrono::time_point<TClock, TDuration>& outTimePoint)
	{
		outTimePoint = std::chrono::time_point<TClock, TDuration>(
			Convert::Detail::SafeDurationCast<TDuration>(std::chrono::seconds(timestamp.seconds)));
		if (timestamp.nanoseconds)
		{
			// When duration period is greater than seconds (allowed rounding only seconds fractions)
			if constexpr (std::ratio_greater_v<typename TDuration::period, std::chrono::seconds::period>)
			{
				throw std::out_of_range("The precision of target duration type is not sufficient to store nanoseconds");
			}
			else
			{
				// Only seconds fractions can be rounded to target type
				auto leftTime = std::chrono::round<TDuration>(std::chrono::nanoseconds(timestamp.nanoseconds));
				Convert::Detail::SafeAddDuration(outTimePoint, leftTime);
			}
		}
	}

	//-----------------------------------------------------------------------------

	/**
	 * @brief Converts a `std::chrono::duration` to a `CBinTimestamp`.
	 *
	 * @param duration     Input duration to convert.
	 * @param outTimestamp Output timestamp to populate.
	 * @throws std::out_of_range if overflow occurs during conversion.
	 */
	template <class TRep, class TPeriod>
	void To(const std::chrono::duration<TRep, TPeriod>& duration, CBinTimestamp& outTimestamp)
	{
		// When duration period is equal or greater than seconds
		if constexpr (std::ratio_greater_equal_v<TPeriod, std::chrono::seconds::period>)
		{
			outTimestamp.seconds = Convert::Detail::SafeDurationCast<std::chrono::seconds>(duration).count();
			outTimestamp.nanoseconds = 0;
		}
		else
		{
			outTimestamp.seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
			const auto leftTime = duration - std::chrono::duration_cast<std::chrono::duration<TRep, TPeriod>>(std::chrono::seconds(outTimestamp.seconds));
			outTimestamp.nanoseconds = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(leftTime).count());
		}
	}

	/**
	 * @brief Converts a `CBinTimestamp` to a `std::chrono::duration`.
	 *
	 * @param timestamp   Input binary timestamp.
	 * @param outDuration Output duration to populate.
	 * @throws std::out_of_range if overflow occurs during conversion.
	 */
	template <class TRep, class TPeriod>
	void To(const CBinTimestamp& timestamp, std::chrono::duration<TRep, TPeriod>& outDuration)
	{
		using TDuration = std::chrono::duration<TRep, TPeriod>;

		outDuration = Convert::Detail::SafeDurationCast<TDuration>(std::chrono::seconds(timestamp.seconds));
		if (timestamp.nanoseconds)
		{
			// When duration period is greater than seconds (allowed rounding only seconds fractions)
			if constexpr (std::ratio_greater_v<TPeriod, std::chrono::seconds::period>)
			{
				throw std::out_of_range("The precision of target duration type is not sufficient to store nanoseconds");
			}
			else
			{
				// Only seconds fractions can be rounded to target type
				Convert::Detail::SafeAddDuration(outDuration, std::chrono::round<TDuration>(std::chrono::nanoseconds(timestamp.nanoseconds)));
			}
		}
	}
} // namespace BitSerializer::Detail
