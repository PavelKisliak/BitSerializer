/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* Copyright (C) 2017 Howard Hinnant (datetime algorithms from 'date' library)  *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cctype>
#include <cstdlib>
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
	constexpr TTarget SafeDurationCast(const std::chrono::duration<TRep, TPeriod>& duration)
	{
		using TDivRatio = std::ratio_divide<TPeriod, typename TTarget::period>;
		using TTargetRep = typename TTarget::rep;
		using TOpRep = std::common_type_t<TTargetRep, TRep, intmax_t>;

		if constexpr (TDivRatio::den == 1)
		{
			if constexpr (TDivRatio::num == 1)
			{
				const auto v = static_cast<TTargetRep>(duration.count());
				if (v != duration.count()) {  // NOLINT(clang-diagnostic-sign-compare)
					throw std::out_of_range("Target duration is not enough");
				}
				return TTarget(v);
			}
			else
			{
				const auto v = static_cast<TTargetRep>(static_cast<TOpRep>(duration.count()) * static_cast<TOpRep>(TDivRatio::num));
				if (v && v / TDivRatio::num != duration.count()) {  // NOLINT(clang-diagnostic-sign-compare)
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
				if (v * TDivRatio::den != duration.count()) {  // NOLINT(clang-diagnostic-sign-compare)
					throw std::out_of_range("Target duration is not enough");
				}
				return TTarget(v);
			}
			else
			{
				const auto v = static_cast<TTargetRep>(static_cast<TOpRep>(duration.count()) * static_cast<TOpRep>(TDivRatio::num) / static_cast<TOpRep>(TDivRatio::den));
				if (v && v / TDivRatio::num != duration.count()) {  // NOLINT(clang-diagnostic-sign-compare)
					throw std::out_of_range("Target duration is not enough");
				}
				return TTarget(v);
			}
		}
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
		out.append(buffer, buffer + outSize);
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ss[.SSS]Z)) to `tm_ext` structure (includes ms).
	/// </summary>
	template <typename TSym>
	static void To(std::basic_string_view<TSym> in, tm_ext& out)
	{
		auto parseDatetime = [](const char* pos, const char* end)
		{
			auto parseDatetimePart = [](const char* buf, const char* end, int& outValue, int maxValue = 0, char delimiter = 0) -> const char*
			{
				if (buf != end && std::isdigit(*buf))
				{
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
				}
				throw std::invalid_argument("Input string is not a valid ISO datetime: YYYY-MM-DDThh:mm:ss[.SSS]Z");
			};

			tm_ext utc{};
			pos = parseDatetimePart(pos, end, utc.tm_year, 0, '-');
			pos = parseDatetimePart(pos, end, utc.tm_mon, 12, '-');
			pos = parseDatetimePart(pos, end, utc.tm_mday, DaysInMonth[utc.tm_mon - 1], 'T');
			pos = parseDatetimePart(pos, end, utc.tm_hour, 23, ':');
			pos = parseDatetimePart(pos, end, utc.tm_min, 59, ':');
			pos = parseDatetimePart(pos, end, utc.tm_sec, 59);
			// Parse optional milliseconds
			if (const auto sym = *pos; sym == '.') {
				pos = parseDatetimePart(++pos, end, utc.ms, 999, 'Z');
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
		out = static_cast<tm>(tmExt);	// Ignore ms part
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
	///	Milliseconds will be rendered only when they present (non-zero).
	/// </summary>
	template <typename TClock, typename TDuration, typename TSym, typename TAllocator, std::enable_if_t<(TClock::is_steady == false), int> = 0>
	void To(const std::chrono::time_point<TClock, TDuration>& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		const time_t time = std::chrono::floor<std::chrono::seconds>(in).time_since_epoch().count();
		auto ms = static_cast<int>((in - std::chrono::seconds(time)).time_since_epoch().count());
		if (ms != 0)
		{
			if (ms < 0) ms += 1000;
			const tm_ext utc(UnixTimeToUtc(time), ms);
			To(utc, out);
		}
		else
		{
			const tm utc = UnixTimeToUtc(time);
			To(utc, out);
		}
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ss[.SSS]Z) to `std::chrono::time_point`.
	///	Examples of allowed dates:
	///	- 1872-01-01T00:00:00Z
	///	- 2023-07-14T22:44:51.925Z
	/// </summary>
	template <typename TSym, typename TClock, typename TDuration, std::enable_if_t<(TClock::is_steady == false), int> = 0>
	void To(std::basic_string_view<TSym> in, std::chrono::time_point<TClock, TDuration>& out)
	{
		tm_ext tmExt;
		To(in, tmExt);

		const auto time = UtcToUnixTime(tmExt);
		constexpr auto maxSec = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::time_point<TClock, TDuration>::max())
			.time_since_epoch().count();
		constexpr auto minSec = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::time_point<TClock, TDuration>::min())
			.time_since_epoch().count();
		if (time > maxSec || time < minSec) {
			throw std::out_of_range("Target timepoint range is not enough to store parsed datetime");
		}

		out = std::chrono::time_point<TClock, TDuration>(std::chrono::seconds(time));
		if (tmExt.ms) {
			out += std::chrono::milliseconds(tmExt.ms);
		}
	}

	/// <summary>
	/// Converts from `std::chrono::duration` to `std::string` (ISO 8601/Duration: PnDTnHnMnS).
	/// </summary>
	template <typename TRep, typename TPeriod, typename TSym, typename TAllocator>
	void To(const std::chrono::duration<TRep, TPeriod>& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		if (!in.count()) {
			out.append({'P', 'T', '0', 'S'});
			return;
		}

		const auto printTimePart = [](auto time, char type, std::string& outStr)
		{
			if (time.count())
			{
				if constexpr (std::is_signed_v<typename decltype(time)::rep>)
				{
					const uint64_t absTime = std::abs(time.count());
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
	void To(std::basic_string_view<TSym> in, std::chrono::duration<TRep, TPeriod>& out)
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
