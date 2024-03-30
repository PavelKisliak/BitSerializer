/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/conversion_detail/convert_chrono.h"

namespace BitSerializer
{
	namespace Detail
	{
		/// <summary>
		/// Internal representation of a timestamp for serialization into binary archives.
		/// The archive may not support a timestamp at all, or may throw a `SerializationException` with `Overflow` code if it's too large.
		/// </summary>
		struct CBinTimestamp
		{
			CBinTimestamp() = default;
			explicit CBinTimestamp(int64_t seconds, int32_t nanoseconds = 0)
				: Seconds(seconds)
				, Nanoseconds(nanoseconds)
			{ }

			bool operator==(const CBinTimestamp& rhs) const
			{
				return Seconds == rhs.Seconds && Nanoseconds == rhs.Nanoseconds;
			}

			[[nodiscard]] std::string ToString() const
			{
				return std::to_string(Seconds) + " " + std::to_string(Nanoseconds);
			}

			int64_t Seconds{};
			int32_t Nanoseconds{};		// Must not be larger than 999999999
		};

		//-----------------------------------------------------------------------------

		template <typename TClock, typename TDuration>
		void To(const std::chrono::time_point<TClock, TDuration>& timePoint, CBinTimestamp& outTimestamp)
		{
			const auto epochTime = timePoint.time_since_epoch();
			// When duration period is equal or greater than seconds
			if constexpr (std::ratio_greater_equal_v<typename TDuration::period, std::chrono::seconds::period>)
			{
				outTimestamp.Seconds = Convert::Detail::SafeDurationCast<std::chrono::seconds>(epochTime).count();
				outTimestamp.Nanoseconds = 0;
			}
			else
			{
				outTimestamp.Seconds = std::chrono::duration_cast<std::chrono::seconds>(epochTime).count();
				const auto leftTime = epochTime - std::chrono::duration_cast<TDuration>(std::chrono::seconds(outTimestamp.Seconds));
				outTimestamp.Nanoseconds = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(leftTime).count());
			}
		}

		template <typename TClock, typename TDuration>
		void To(const CBinTimestamp& timestamp, std::chrono::time_point<TClock, TDuration>& outTimePoint)
		{
			outTimePoint = std::chrono::time_point<TClock, TDuration>(
				Convert::Detail::SafeDurationCast<TDuration>(std::chrono::seconds(timestamp.Seconds)));
			if (timestamp.Nanoseconds)
			{
				// When duration period is greater than seconds (allowed rounding only seconds fractions)
				if constexpr (std::ratio_greater_v<typename TDuration::period, std::chrono::seconds::period>)
				{
					throw std::out_of_range("The precision of target duration type is not sufficient to store nanoseconds");
				}
				else
				{
					// Only seconds fractions can be rounded to target type
					auto leftTime = std::chrono::round<TDuration>(std::chrono::nanoseconds(timestamp.Nanoseconds));
					Convert::Detail::SafeAddDuration(outTimePoint, leftTime);
				}
			}
		}

		//-----------------------------------------------------------------------------

		template <class TRep, class TPeriod>
		void To(const std::chrono::duration<TRep, TPeriod>& duration, CBinTimestamp& outTimestamp)
		{
			// When duration period is equal or greater than seconds
			if constexpr (std::ratio_greater_equal_v<TPeriod, std::chrono::seconds::period>)
			{
				outTimestamp.Seconds = Convert::Detail::SafeDurationCast<std::chrono::seconds>(duration).count();
				outTimestamp.Nanoseconds = 0;
			}
			else
			{
				outTimestamp.Seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
				const auto leftTime = duration - std::chrono::duration_cast<std::chrono::duration<TRep, TPeriod>>(std::chrono::seconds(outTimestamp.Seconds));
				outTimestamp.Nanoseconds = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(leftTime).count());
			}
		}

		template <class TRep, class TPeriod>
		void To(const CBinTimestamp& timestamp, std::chrono::duration<TRep, TPeriod>& outDuration)
		{
			using TDuration = std::chrono::duration<TRep, TPeriod>;

			outDuration = Convert::Detail::SafeDurationCast<TDuration>(std::chrono::seconds(timestamp.Seconds));
			if (timestamp.Nanoseconds)
			{
				// When duration period is greater than seconds (allowed rounding only seconds fractions)
				if constexpr (std::ratio_greater_v<TPeriod, std::chrono::seconds::period>)
				{
					throw std::out_of_range("The precision of target duration type is not sufficient to store nanoseconds");
				}
				else
				{
					// Only seconds fractions can be rounded to target type
					Convert::Detail::SafeAddDuration(outDuration, std::chrono::round<TDuration>(std::chrono::nanoseconds(timestamp.Nanoseconds)));
				}
			}
		}
	}
}
