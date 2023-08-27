/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* Copyright (C) 2017 Howard Hinnant (datetime algorithms from 'date' library)  *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cctype>
#include <cstdlib>
#include <climits>
#include <chrono>
#include <charconv>
#include "convert_utf.h"


namespace BitSerializer
{
	/// <summary>
	/// Wrapper for `time_t` type, used to distinguish between time_t and integer types.
	///	Usage example:
	///	   time_t time = Convert::To<CRawTime>("2044-01-01T00:00:00Z");
	///	   auto isoDate = Convert::To<std::string>(CRawTime(time));
	/// </summary>
	struct CRawTime
	{
		CRawTime() = default;
		explicit CRawTime(time_t time) noexcept : Time(time) {}

		operator time_t& () noexcept { return Time; }
		operator const time_t& () const noexcept { return Time; }

		time_t Time = 0;
	};
}

namespace BitSerializer::Convert::Detail
{
	constexpr size_t UtcBufSize = 32;
	static constexpr int DaysInMonth[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	struct tm_ext : tm
	{
		tm_ext() : tm() { }
		tm_ext(tm inTm, int inMs) : tm(inTm), ms(inMs) { }

		int ms = 0;
	};

	/// <summary>
	/// Converts Unix time to UTC expressed in the `tm` structure.
	/// </summary>
	static tm UnixTimeToUtc(time_t dateTime) noexcept
	{
		// Based on Howard Hinnant's algorithm
		static_assert(sizeof(int) >= 4, "This algorithm has not been ported to a 16 bit integers");

		auto days = dateTime / 86400;
		const auto time = static_cast<int>(dateTime - days * 86400);
		if (time < 0) --days;

		auto const z = days + 719468;
		auto const era = (z >= 0 ? z : z - 146096) / 146097;
		auto const doe = static_cast<unsigned>(z - era * 146097);				// [0, 146096]
		auto const yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;	// [0, 399]
		auto const y = yoe + era * 400;
		auto const doy = doe - (365 * yoe + yoe / 4 - yoe / 100);				// [0, 365]
		auto const mp = (5 * doy + 2) / 153;									// [0, 11]
		auto const d = doy - (153 * mp + 2) / 5 + 1;							// [1, 31]
		auto const m = mp < 10 ? mp + 3 : mp - 9;								// [1, 12]

		tm utc{};
		utc.tm_year = static_cast<int>(y) + (m <= 2);
		utc.tm_mon = static_cast<int>(m);
		utc.tm_mday = static_cast<int>(d);
		utc.tm_hour = time / 3600;
		utc.tm_min = time % 3600 / 60;
		utc.tm_sec = time % 60;
		// Adjust time before EPOCH
		if (time < 0)
		{
			if (utc.tm_sec < 0) utc.tm_sec += 60;
			utc.tm_min += utc.tm_sec == 0 ? 60 : 59;
			if (utc.tm_min == 60)
			{
				utc.tm_min = 0;
				utc.tm_hour += 24;
			}
			else {
				utc.tm_hour += 23;
			}
		}
		return utc;
	}

	/// <summary>
	/// Converts UTC expressed in the `tm` structure to Unix time.
	/// </summary>
	static time_t UtcToUnixTime(const tm& utc) noexcept
	{
		// Based on Howard Hinnant's algorithm
		static_assert(sizeof(int) >= 4, "This algorithm has not been ported to a 16 bit integers");

		auto const y = static_cast<int>(utc.tm_year) - (utc.tm_mon <= 2);
		auto const m = static_cast<unsigned>(utc.tm_mon);
		auto const d = static_cast<unsigned>(utc.tm_mday);
		auto const era = (y >= 0 ? y : y - 399) / 400;
		auto const yoe = static_cast<unsigned>(y - era * 400);				// [0, 399]
		auto const doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;	// [0, 365]
		auto const doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;				// [0, 146096]

		const auto days = era * 146097 + static_cast<int>(doe) - 719468;
		const auto time = static_cast<long long>(utc.tm_hour) * 3600 + static_cast<long long>(utc.tm_min) * 60 + utc.tm_sec;
		return static_cast<long long>(days) * 86400 + time;
	}

	/// <summary>
	/// Converts WITHOUT truncation a `std::chrono::duration` to another type of duration.
	/// </summary>
	/// <exception cref="std::out_of_range">Thrown when overflow target value.</exception>
	template <class TTarget, class TRep, class TPeriod>
	TTarget SafeDurationCast(const std::chrono::duration<TRep, TPeriod>& duration)
	{
		if constexpr (std::is_same_v<TTarget, std::chrono::duration<TRep, TPeriod>>) {
			return duration;
		}

		using TDivRatio = std::ratio_divide<TPeriod, typename TTarget::period>;
		using TTargetRep = typename TTarget::rep;
		using TOpRep = std::common_type_t<TTargetRep, TRep, intmax_t>;

		if constexpr (TDivRatio::den == 1)
		{
			if constexpr (TDivRatio::num == 1)
			{
				const auto v = static_cast<TTargetRep>(duration.count());
				if (static_cast<TRep>(v) != duration.count()) {
					throw std::out_of_range("Target duration is not enough");
				}
				return TTarget(v);
			}
			else
			{
				const auto v = static_cast<TTargetRep>(static_cast<TOpRep>(duration.count()) * static_cast<TOpRep>(TDivRatio::num));
				if (v && static_cast<TRep>(v / TDivRatio::num) != duration.count()) {
					throw std::out_of_range("Target duration is not enough");
				}
				return TTarget(v);
			}
		}
		else
		{
			if constexpr (TDivRatio::num == 1)
			{
				const auto v = static_cast<TTargetRep>(static_cast<TOpRep>(duration.count()) / static_cast<TOpRep>(TDivRatio::den));
				if (static_cast<TRep>(v * TDivRatio::den) != duration.count()) {
					throw std::out_of_range("Target duration is not enough");
				}
				return TTarget(v);
			}
			else
			{
				const auto v = static_cast<TTargetRep>(static_cast<TOpRep>(duration.count()) * static_cast<TOpRep>(TDivRatio::num) / static_cast<TOpRep>(TDivRatio::den));
				if (v && static_cast<TRep>(v / TDivRatio::num) != duration.count()) {
					throw std::out_of_range("Target duration is not enough");
				}
				return TTarget(v);
			}
		}
	}

	/// <summary>
	/// Adds `std::chrono::duration` to `std::chrono::timepoint` WITHOUT truncation.
	/// </summary>
	/// <exception cref="std::out_of_range">Thrown when overflow target value.</exception>
	template <class TTargetClock, class TTargetDuration, class TSrcRep, class TSrcPeriod>
	void SafeAddDuration(std::chrono::time_point<TTargetClock, TTargetDuration>& tp, const std::chrono::duration<TSrcRep, TSrcPeriod>& duration)
	{
		if (duration.count() == 0) {
			return;
		}

		using TSrcDur = std::chrono::duration<TSrcRep, TSrcPeriod>;
#if defined(__clang__)
		const auto cmpRep = 0;
#else
		const auto cmpRep = tp.time_since_epoch().count();
#endif
		constexpr auto maxSrcDur = std::chrono::time_point_cast<TSrcDur>(std::chrono::time_point<TTargetClock, TTargetDuration>::max())
			.time_since_epoch();
		constexpr auto minSrcDur = std::chrono::time_point_cast<TSrcDur>(std::chrono::time_point<TTargetClock, TTargetDuration>::min())
			.time_since_epoch();
		if (duration > maxSrcDur || duration < minSrcDur) {
			throw std::out_of_range("Target timepoint range is not enough");
		}

		TTargetDuration adaptedDuration;
		try {
			adaptedDuration = SafeDurationCast<TTargetDuration>(duration);
		}
		catch (const std::out_of_range&) {
			throw std::out_of_range("Target timepoint range is not enough");
		}

		const auto newTp = tp + adaptedDuration;
		if (tp.time_since_epoch().count() < 0)
		{
			if (adaptedDuration.count() < 0 && newTp.time_since_epoch().count() >= cmpRep) {
				throw std::out_of_range("Target timepoint range is not enough");
			}
		}
		else
		{
			if (adaptedDuration.count() > 0 && newTp.time_since_epoch().count() <= cmpRep) {
				throw std::out_of_range("Target timepoint range is not enough");
			}
		}
		tp = std::chrono::time_point<TTargetClock, TTargetDuration>(newTp);
	}

	/// <summary>
	/// Converts UTC expressed in the `tm` structure to `std::string` (ISO 8601/UTC).
	/// </summary>
	template <typename TSym, typename TAllocator>
	void To(const tm& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		char buffer[UtcBufSize];
		const size_t outSize = snprintf(buffer, UtcBufSize, "%04d-%02d-%02dT%02d:%02d:%02dZ", in.tm_year, in.tm_mon, in.tm_mday, in.tm_hour, in.tm_min, in.tm_sec);
		if (outSize <= 0) {
			throw std::runtime_error("Unknown error");
		}
		if (in.tm_year >= 10000) {
			out.push_back('+');
		}
		out.append(buffer, buffer + outSize);
	}

	/// <summary>
	/// Converts UTC expressed in the `tm_ext` structure (includes ms) to `std::string` (ISO 8601/UTC).
	/// </summary>
	template <typename TSym, typename TAllocator>
	void To(const tm_ext& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		char buffer[UtcBufSize];
		const size_t outSize = snprintf(buffer, UtcBufSize, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
			in.tm_year, in.tm_mon, in.tm_mday, in.tm_hour, in.tm_min, in.tm_sec, in.ms);
		if (outSize <= 0) {
			throw std::runtime_error("Unknown error");
		}
		if (in.tm_year >= 10000) {
			out.push_back('+');
		}
		out.append(buffer, buffer + outSize);
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ss[.SSS]Z)) to `tm_ext` structure (includes ms).
	///	Fractions of second are optional, only 3 digits are allowed.
	/// </summary>
	template <typename TSym>
	static void To(std::basic_string_view<TSym> in, tm_ext& out)
	{
		auto parseDatetime = [](const char* pos, const char* end)
		{
			auto parseDatetimePart = [](const char* buf, const char* end, int& outValue, int maxValue = 0, char delimiter = 0, bool isYear = false) -> const char*
			{
				if (buf != end && (std::isdigit(*buf) || isYear))
				{
					if (isYear && *buf == '+') {
						++buf;
					}
					const std::from_chars_result result = std::from_chars(buf, end, outValue);
					if (result.ec == std::errc())
					{
						if (maxValue && outValue > maxValue) {
							throw std::invalid_argument("Input datetime contains out-of-bounds values");
						}
						if (delimiter)
						{
							if (result.ptr != end && *result.ptr == delimiter) {
								return result.ptr + 1;
							}
						}
						else {
							return result.ptr;
						}
					}
					if (result.ec == std::errc::result_out_of_range) {
						throw std::out_of_range("ISO datetime contains too big number");
					}
				}
				throw std::invalid_argument("Input string is not a valid ISO datetime: YYYY-MM-DDThh:mm:ss[.SSS]Z");
			};

			tm_ext utc{};
			pos = parseDatetimePart(pos, end, utc.tm_year, 0, '-', true);
			pos = parseDatetimePart(pos, end, utc.tm_mon, 12, '-');
			pos = parseDatetimePart(pos, end, utc.tm_mday, DaysInMonth[utc.tm_mon - 1], 'T');
			pos = parseDatetimePart(pos, end, utc.tm_hour, 23, ':');
			pos = parseDatetimePart(pos, end, utc.tm_min, 59, ':');
			pos = parseDatetimePart(pos, end, utc.tm_sec, 59);
			// Parse optional fractions of second
			if (const auto sym = *pos; sym == '.')
			{
				int value = 0;
				const auto frPos = ++pos;
				pos = parseDatetimePart(frPos, end, value, 999, 'Z');
				const auto digits = pos - frPos - 1;
				// Accordingly to ISO: 0.500 and 0.5 = 500ms
				if (digits == 3) {
					utc.ms = value;
				}
				else if (digits == 2 && value < 100) {
					utc.ms = value * 10;
				}
				else if (digits == 1 && value < 10) {
					utc.ms = value * 100;
				}
				else {
					throw std::invalid_argument("ISO datetime contains more than 3 digits in the fractions of second");
				}
			}
			else if (sym != 'Z') {
				throw std::invalid_argument("Input string is not a valid ISO datetime: YYYY-MM-DDThh:mm:ss[.SSS]Z");
			}
			return utc;
		};

		if constexpr (sizeof(TSym) == sizeof(char)) {
			out = parseDatetime(in.data(), in.data() + in.size());
		}
		else
		{
			std::string utf8Str;
			Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
			out = parseDatetime(utf8Str.data(), utf8Str.data() + utf8Str.size());
		}
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ssZ) to `tm` structure.
	/// </summary>
	template <typename TSym>
	void To(std::basic_string_view<TSym> in, tm& out)
	{
		tm_ext tmExt;
		To(in, tmExt);
		out = static_cast<tm>(tmExt);	// Ignore 'ms' part
	}

	/// <summary>
	/// Converts Unix time in the `CRawTime` to `std::string` (ISO 8601/UTC).
	/// </summary>
	template <typename TSym, typename TAllocator>
	void To(const CRawTime& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		const tm utc = UnixTimeToUtc(in.Time);
		To(utc, out);
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ssZ) to Unix time in the `CRawTime`.
	/// </summary>
	template <typename TSym>
	void To(std::basic_string_view<TSym> in, CRawTime& out)
	{
		tm tm{};
		To(in, tm);
		out.Time = UtcToUnixTime(tm);
	}

	/// <summary>
	/// Converts from `std::chrono::time_point` to `std::string` (ISO 8601/UTC).
	///	Fractions of second will be rendered only when they present (non-zero).
	/// </summary>
	template <typename TClock, typename TDuration, typename TSym, typename TAllocator, std::enable_if_t<(TClock::is_steady == false), int> = 0>
	static void To(const std::chrono::time_point<TClock, TDuration>& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		using TDays = std::chrono::duration<typename TDuration::rep, std::ratio<86400>>;
		const auto datePart = std::chrono::floor<TDays>(in);
		const auto timePart = in - datePart;
		auto timeInSec = std::chrono::floor<std::chrono::seconds>(timePart).count();
		auto days = datePart.time_since_epoch().count();

		// Based on Howard Hinnant's algorithm
		static_assert(sizeof(int) >= 4, "This algorithm has not been ported to a 16 bit integers");
		auto const z = days + 719468ll;
		auto const era = (z >= 0 ? z : z - 146096) / 146097;
		auto const doe = static_cast<unsigned>(z - era * 146097);				// [0, 146096]
		auto const yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;	// [0, 399]
		auto const y = yoe + era * 400;
		auto const doy = doe - (365 * yoe + yoe / 4 - yoe / 100);				// [0, 365]
		auto const mp = (5 * doy + 2) / 153;									// [0, 11]
		auto const d = doy - (153 * mp + 2) / 5 + 1;							// [1, 31]
		auto const m = mp < 10 ? mp + 3 : mp - 9;								// [1, 12]

		tm_ext utc{};
		utc.tm_year = static_cast<int>(y) + (m <= 2);
		utc.tm_mon = static_cast<int>(m);
		utc.tm_mday = static_cast<int>(d);
		utc.tm_hour = static_cast<int>(timeInSec / 3600);
		utc.tm_min = timeInSec % 3600 / 60;
		utc.tm_sec = timeInSec % 60;
		utc.ms = static_cast<int>(std::chrono::round<std::chrono::milliseconds>(timePart - std::chrono::seconds(timeInSec)).count());
		if (utc.ms != 0)
		{
			if (utc.ms < 0) utc.ms += 1000;
			To(utc, out);
		}
		else {
			To(static_cast<tm&>(utc), out);		// Print without 'ms' part
		}
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ss[.SSS]Z) to `std::chrono::time_point`.
	///	Fractions of second are optional, only 3 digits are allowed.
	///	Examples of allowed dates:
	///	- 1872-01-01T00:00:00Z
	///	- 2023-07-14T22:44:51.925Z
	/// </summary>
	template <typename TSym, typename TClock, typename TDuration, std::enable_if_t<(TClock::is_steady == false), int> = 0>
	static void To(std::basic_string_view<TSym> in, std::chrono::time_point<TClock, TDuration>& out)
	{
		tm_ext tmExt;
		To(in, tmExt);

		// Based on Howard Hinnant's algorithm
		static_assert(sizeof(int) >= 4, "This algorithm has not been ported to a 16 bit integers");
		auto const y = static_cast<int>(tmExt.tm_year) - (tmExt.tm_mon <= 2);
		auto const m = static_cast<unsigned>(tmExt.tm_mon);
		auto const d = static_cast<unsigned>(tmExt.tm_mday);
		auto const era = (y >= 0 ? y : y - 399) / 400;
		auto const yoe = static_cast<unsigned>(y - era * 400);				// [0, 399]
		auto const doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;	// [0, 365]
		auto const doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;				// [0, 146096]

		std::chrono::time_point<TClock, TDuration> tp;
		const int64_t days = era * 146097ll + doe - 719468;
		if ((y >= 1970 && days < 0) || (y < 1970 && days > 0)) {
			throw std::out_of_range("Target timepoint range is not enough");
		}
		SafeAddDuration(tp, std::chrono::duration<int64_t, std::ratio<86400>>(days));
		const auto time = static_cast<long long>(tmExt.tm_hour) * 3600 + static_cast<long long>(tmExt.tm_min) * 60 + tmExt.tm_sec;
		SafeAddDuration(tp, std::chrono::seconds(time));
		SafeAddDuration(tp, std::chrono::milliseconds(tmExt.ms));
		out = tp;
	}

	/// <summary>
	/// Converts from `std::chrono::duration` to `std::string` (ISO 8601/Duration: PnDTnHnMnS).
	/// </summary>
	template <typename TRep, typename TPeriod, typename TSym, typename TAllocator>
	static void To(const std::chrono::duration<TRep, TPeriod>& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		if (!in.count()) {
			out.append({ 'P', 'T', '0', 'S' });
			return;
		}

		const auto printTimePart = [](auto time, char type, std::string& outStr)
		{
			if (time.count())
			{
				if constexpr (std::is_signed_v<TRep>)
				{
					constexpr uint64_t maxI64Negative = 9223372036854775808u;
					const uint64_t absTime = time.count() == LLONG_MIN ? maxI64Negative : static_cast<uint64_t>(std::abs(time.count()));
					outStr.append(std::to_string(absTime));
				}
				else {
					outStr.append(std::to_string(time.count()));
				}
				outStr.push_back(type);
			}
			return time;
		};

		using days = std::chrono::duration<TRep, std::ratio<86400>>;
		using hours = std::chrono::duration<TRep, std::ratio<3600>>;
		using minutes = std::chrono::duration<TRep, std::ratio<60>>;
		using seconds = std::chrono::duration<TRep, std::ratio<1>>;

		std::string isoDur(in.count() < 0 ? "-P" : "P");
		const auto hoursLeft = in - printTimePart(std::chrono::duration_cast<days>(in), 'D', isoDur);
		if (hoursLeft.count())
		{
			isoDur.push_back('T');
			const auto minutesLeft = hoursLeft - printTimePart(std::chrono::duration_cast<hours>(hoursLeft), 'H', isoDur);
			const auto secondsLeft = minutesLeft - printTimePart(std::chrono::duration_cast<minutes>(minutesLeft), 'M', isoDur);
			printTimePart(std::chrono::duration_cast<seconds>(secondsLeft), 'S', isoDur);
		}
		out.append(isoDur.data(), isoDur.data() + isoDur.size());
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/Duration format: PnWnDTnHnMnS) to `std::chrono::duration`.
	///	Examples of allowed durations: P25DT55M41S, P1W, PT10H20S, -P10DT25M (supported signed durations, but they are no officially defined).
	/// Durations which contains years, month, or with base UTC (2003-02-15T00:00:00Z/P2M) are not allowed.
	///	The decimal fraction of smallest value like "P0.5D" is not supported.
	/// </summary>
	template <typename TSym, typename TRep, typename TPeriod>
	static void To(std::basic_string_view<TSym> in, std::chrono::duration<TRep, TPeriod>& out)
	{
		using TTargetDuration = std::chrono::duration<TRep, TPeriod>;
		const auto parseDuration = [](const char* pos, const char* end)
		{
			const auto parseNextPart = [](const char* pos, const char* end, bool isDatePart, bool isNegative, TTargetDuration& duration)
			{
				const auto transformToDuration = [](auto value, char type, bool isDatePart) -> TTargetDuration
				{
					if (isDatePart)
					{
						if (type == 'W') {
							return SafeDurationCast<TTargetDuration>(std::chrono::duration<uint64_t, std::ratio<604800>>(value));
						} if (type == 'D') {
							return SafeDurationCast<TTargetDuration>(std::chrono::duration<uint64_t, std::ratio<86400>>(value));
						} if (type == 'Y' || type == 'M') {
							throw std::invalid_argument("An ISO duration that contains a year, or month is not allowed");
						}
					}
					else
					{
						if (type == 'H') {
							return SafeDurationCast<TTargetDuration>(std::chrono::duration<uint64_t, std::ratio<3600>>(value));
						} if (type == 'M') {
							return SafeDurationCast<TTargetDuration>(std::chrono::duration<uint64_t, std::ratio<60>>(value));
						} if (type == 'S') {
							return SafeDurationCast<TTargetDuration>(std::chrono::duration<uint64_t, std::ratio<1>>(value));
						}
					}
					throw std::invalid_argument("Input string is not a valid ISO duration: PnWnDTnHnMnS");
				};

				if (pos != end && std::isdigit(*pos))
				{
					uint64_t value = 0;
					std::from_chars_result result = std::from_chars(pos, end, value);
					if (result.ec == std::errc() && result.ptr != end)
					{
						const auto type = *result.ptr++;
						const auto origDur = duration;
						if (isNegative)
						{
							constexpr uint64_t maxI64Negative = 9223372036854775808u;
							if (value <= maxI64Negative)
							{
								if ((duration += transformToDuration(-static_cast<int64_t>(value), type, isDatePart)) > origDur) {
									throw std::out_of_range("Target type is not enough to store parsed duration");
								}
								return result.ptr;
							}
							result.ec = std::errc::result_out_of_range;
						}
						else
						{
							if ((duration += transformToDuration(value, type, isDatePart)) < origDur) {
								throw std::out_of_range("Target type is not enough to store parsed duration");
							}
							return result.ptr;
						}
					}
					if (result.ec == std::errc::result_out_of_range) {
						throw std::out_of_range("ISO duration contains too big number");
					}
				}
				throw std::invalid_argument("Input string is not a valid ISO duration: PnWnDTnHnMnS");
			};

			constexpr auto minSizeIsoDuration = 3;
			if (end - pos >= minSizeIsoDuration)
			{
				if (const bool isNegative = *pos == '-'; *pos == 'P' || (isNegative && *(++pos) == 'P'))
				{
					if (isNegative && !std::is_signed_v<typename TTargetDuration::rep>) {
						throw std::out_of_range("Target duration type can't store negative values");
					}

					++pos;
					TTargetDuration duration(0);
					bool isDatePart = true;
					do
					{
						if (isDatePart && *pos == 'T')
						{
							isDatePart = false;
							++pos;
						}
						pos = parseNextPart(pos, end, isDatePart, isNegative, duration);
					} while (pos != end && !std::isspace(*pos));
					return duration;
				}
			}
			throw std::invalid_argument("Input string is not a valid ISO duration: PnWnDTnHnMnS");
		};

		if constexpr (sizeof(TSym) == sizeof(char)) {
			out = parseDuration(in.data(), in.data() + in.size());
		}
		else
		{
			std::string utf8Str;
			Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
			out = parseDuration(utf8Str.data(), utf8Str.data() + utf8Str.size());
		}
	}
}
