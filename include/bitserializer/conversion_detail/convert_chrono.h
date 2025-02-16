/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* Copyright (C) 2017 Howard Hinnant (datetime algorithms from 'date' library)  *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cctype>
#include <cstdlib>
#include <cinttypes>
#include <climits>
#include <chrono>
#include <charconv>
#include <optional>
#include "bitserializer/conversion_detail/convert_utf.h"


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
	constexpr int DaysInMonth[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	template <class TFractions = std::chrono::nanoseconds,
		std::enable_if_t<TFractions::period::num == 1 && std::chrono::seconds::period::den < TFractions::period::den, int> = 0>
	struct CDateTimeParts
	{
		CDateTimeParts() = default;
		CDateTimeParts(const tm& tm, std::optional<TFractions> secFractions = std::nullopt)
			: Year(tm.tm_year), Month(tm.tm_mon), Day(tm.tm_mday), Hour(tm.tm_hour)
			, Min(tm.tm_min), Sec(tm.tm_sec), SecFractions(secFractions)
		{ }

		int64_t Year = 0;
		int Month = 1;
		int Day = 1;
		int Hour = 1;
		int Min = 1;
		int Sec = 1;
		std::optional<TFractions> SecFractions;
	};

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
				if (duration.count() != static_cast<TRep>(v) || (duration.count() > 0 && v < 0) || (duration.count() < 0 && v > 0)) {
					throw std::out_of_range("Target duration is not enough");
				}
				return TTarget(v);
			}
			else
			{
				if (static_cast<TOpRep>(duration.count()) > std::numeric_limits<TOpRep>::max() / TDivRatio::num ||
					static_cast<TOpRep>(duration.count()) < std::numeric_limits<TOpRep>::min() / TDivRatio::num) 
				{
					throw std::out_of_range("Target duration is not enough");
				}
				const auto v = static_cast<TOpRep>(duration.count()) * static_cast<TOpRep>(TDivRatio::num);
				const auto t = static_cast<TTargetRep>(v);
				if (v != static_cast<TOpRep>(t) || (v > 0 && t < 0) || (v < 0 && t > 0)) {
					throw std::out_of_range("Target duration is not enough");
				}
				return TTarget(t);
			}
		}
		else
		{
			if constexpr (TDivRatio::num == 1)
			{
				const auto v = static_cast<TTargetRep>(static_cast<TOpRep>(duration.count()) / static_cast<TOpRep>(TDivRatio::den));
				if (static_cast<TRep>(v * TDivRatio::den) != duration.count()) {
					throw std::out_of_range("Precision of target duration is not enough");
				}
				return TTarget(v);
			}
			else
			{
				if (static_cast<TOpRep>(duration.count()) > std::numeric_limits<TOpRep>::max() / TDivRatio::num ||
					static_cast<TOpRep>(duration.count()) < std::numeric_limits<TOpRep>::min() / TDivRatio::num)
				{
					throw std::out_of_range("Target duration is not enough");
				}

				const auto v = static_cast<TTargetRep>(static_cast<TOpRep>(duration.count()) * static_cast<TOpRep>(TDivRatio::num) / static_cast<TOpRep>(TDivRatio::den));
				if (v && static_cast<TRep>(v * TDivRatio::den / TDivRatio::num) != duration.count()) {
					throw std::out_of_range("Precision of target duration is not enough");
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

		using TOpDur = std::chrono::duration<std::common_type_t<TSrcRep, typename TTargetDuration::rep, intmax_t>, typename TTargetDuration::period>;
		TOpDur adaptedDuration{};
		try {
			adaptedDuration = SafeDurationCast<TOpDur>(duration);
		}
		catch (const std::out_of_range&) {
			throw std::out_of_range("Target timepoint range is not enough");
		}

		if ((adaptedDuration.count() > 0 && tp > std::chrono::time_point<TTargetClock, TTargetDuration>::max() - adaptedDuration) ||
			(adaptedDuration.count() < 0 && tp < std::chrono::time_point<TTargetClock, TTargetDuration>::min() - adaptedDuration)) {
			throw std::out_of_range("Target timepoint range is not enough");
		}

		const auto newTp = tp + adaptedDuration;
		tp = std::chrono::time_point<TTargetClock, TTargetDuration>(newTp);
	}

	/// <summary>
	/// Adds two `std::chrono::duration` WITHOUT truncation.
	/// </summary>
	/// <exception cref="std::out_of_range">Thrown when overflow target value.</exception>
	template <class TTargetRep, class TTargetPeriod, class TSrcRep, class TSrcPeriod>
	void SafeAddDuration(std::chrono::duration<TTargetRep, TTargetPeriod>& target, const std::chrono::duration<TSrcRep, TSrcPeriod>& src)
	{
		if (src.count() == 0) {
			return;
		}

		using TTargetDuration = std::chrono::duration<TTargetRep, TTargetPeriod>;
		auto adaptedDuration = SafeDurationCast<TTargetDuration>(src);
		if ((adaptedDuration.count() > 0 && target > TTargetDuration::max() - adaptedDuration) ||
			(adaptedDuration.count() < 0 && target < TTargetDuration::min() - adaptedDuration)) {
			throw std::out_of_range("Target duration is not enough");
		}
		target += adaptedDuration;
	}

	/// <summary>
	/// Parses fractions of second.
	/// Supported up to 9 digits, which is enough for values represented in nanoseconds.
	/// </summary>
	/// <returns>Pointer to next character or nullptr when failed</returns>
	template <class TTargetRep, class TTargetPeriod>
	const char* ParseSecondFractions(const char* pos, const char* endPos, std::chrono::duration<TTargetRep, TTargetPeriod>& outTime) noexcept
	{
		static_assert(std::ratio_less_v<TTargetPeriod, std::chrono::seconds::period>, "Target duration must be more precise than a second");
		uint32_t value;
		const std::from_chars_result result = std::from_chars(pos, endPos, value);
		if (result.ec == std::errc())
		{
			if (value == 0)
			{
				outTime = std::chrono::duration<TTargetRep, TTargetPeriod>(0);
				return result.ptr;
			}
			const size_t digits = result.ptr - pos;
			constexpr uint64_t dec[] = { 1ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull, 10000000ull, 100000000ull, 1000000000ull };
			constexpr uint64_t mult = 1000000000ull;
			if (digits < std::size(dec))
			{
				const auto time = static_cast<TTargetRep>(TTargetPeriod::den * mult / (dec[digits] * mult / value));
				outTime = std::chrono::duration<TTargetRep, TTargetPeriod>(time);
				return result.ptr;
			}
		}
		return nullptr;
	}

	/// <summary>
	/// Prints fractions of second to target buffer.
	/// Supported up to 9 digits, which is enough for values with nanosecond precision.
	/// </summary>
	/// <returns>Pointer to next character or nullptr when failed (in case when passed buffer is not enough)</returns>
	template <class TRep, class TPeriod>
	char* PrintSecondsFractions(char* pos, const char* end, std::chrono::duration<TRep, TPeriod> time, bool fixedWidth = true) noexcept
	{
		static_assert(std::ratio_less_v<TPeriod, std::chrono::seconds::period>, "Source duration must be more precise than a second");
		static_assert(std::ratio_greater_equal_v<TPeriod, std::chrono::nanoseconds::period>, "Maximum allowed precision is nanoseconds");
		if (time >= std::chrono::seconds(1)) {
			return nullptr;
		}
		if (pos != end) {
			*pos++ = '.';
		}
		auto val = TPeriod::den > std::nano::den
			? std::abs(std::chrono::floor<std::chrono::microseconds>(time).count())
			: std::abs(time.count());
		for (auto div : { 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1 })
		{
			if (pos == end) {
				return nullptr;
			}
			if (div < TPeriod::den)
			{
				const auto n = val / div;
				*pos = static_cast<char>(n) + '0';
				val -= n * div;
				++pos;
				if (!fixedWidth && val == 0) {
					break;
				}
			}
		}
		return pos;
	}

	/// <summary>
	/// Subtracts and prints time part from passed duration.
	/// </summary>
	/// <returns>Pointer to the next character or throws exception when passed buffer is not enough</returns>
	template <class TPrintDur, class TRep, class TPeriod>
	char* PrintDurationPart(std::chrono::duration<TRep, TPeriod>& timeLeft, char* pos, char* endPos, char suffix)
	{
		constexpr bool isSecondsPart = std::is_same_v<std::chrono::seconds::period, typename TPrintDur::period>;
		const auto timePart = std::chrono::duration_cast<TPrintDur>(timeLeft);
		if (timePart.count() || (isSecondsPart && timeLeft.count()))
		{
			uint64_t val;
			if constexpr (std::is_signed_v<TRep>)
			{
				constexpr uint64_t maxI64Negative = 9223372036854775808u;
				val = timePart.count() == LLONG_MIN ? maxI64Negative : static_cast<uint64_t>(std::abs(timePart.count()));
			}
			else {
				val = timePart.count();
			}
			const auto rc = std::to_chars(pos, endPos, val);
			if (rc.ec == std::errc())
			{
				pos = rc.ptr;
				timeLeft -= std::chrono::duration_cast<std::chrono::duration<TRep, TPeriod>>(timePart);
				// Print fractions only when current part is seconds and source duration is more precise than second
				if constexpr (isSecondsPart && std::ratio_less_v<TPeriod, std::chrono::seconds::period>)
				{
					pos = PrintSecondsFractions(pos, endPos, timeLeft, false);
					timeLeft = std::chrono::duration<TRep, TPeriod>(0);
				}
				if (pos != nullptr && pos != endPos)
				{
					*pos++ = suffix;
					return pos;
				}
			}
			throw std::runtime_error("Internal error: insufficient buffer size");
		}
		return pos;
	}

	/// <summary>
	/// Parses ISO 8601/UTC datetime in the format: YYYY-MM-DDThh:mm:ss[.SSS]Z
	/// </summary>
	template <typename TSym>
	static CDateTimeParts<> ParseIsoUtc(std::basic_string_view<TSym> in)
	{
		auto parseDatetime = [](const char* pos, const char* end)
		{
			auto parseDatetimePart = [](const char* buf, const char* end, auto& outValue, std::optional<int> minValue = std::nullopt, std::optional<int> maxValue = std::nullopt, char delimiter = 0, bool isYear = false) -> const char*
			{
				if (buf != end && (std::isdigit(*buf) || isYear))
				{
					if (isYear && *buf == '+') {
						++buf;
					}
					const std::from_chars_result result = std::from_chars(buf, end, outValue);
					if (result.ec == std::errc())
					{
						if ((minValue && outValue < minValue) || (maxValue && outValue > maxValue)) {
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

			CDateTimeParts utc{};
			pos = parseDatetimePart(pos, end, utc.Year, std::nullopt, std::nullopt, '-', true);
			pos = parseDatetimePart(pos, end, utc.Month, 1, 12, '-');
			pos = parseDatetimePart(pos, end, utc.Day, 1, DaysInMonth[utc.Month - 1], 'T');
			pos = parseDatetimePart(pos, end, utc.Hour, 0, 23, ':');
			pos = parseDatetimePart(pos, end, utc.Min, 0, 59, ':');
			pos = parseDatetimePart(pos, end, utc.Sec, 0, 59);
			// Parse optional fractions of second
			if (pos != end && (*pos == '.' || *pos == ','))
			{
				std::chrono::nanoseconds ns(0);
				if (pos = ParseSecondFractions(++pos, end, ns); pos == nullptr) {
					throw std::invalid_argument("Input ISO datetime has invalid fractions of second");
				}
				utc.SecFractions = ns;
			}
			// Should have 'Z' at the end of UTC datetime
			if (pos == end || *pos != 'Z') {
				throw std::invalid_argument("Input string is not a valid ISO datetime: YYYY-MM-DDThh:mm:ss[.SSS]Z");
			}
			return utc;
		};

		if constexpr (sizeof(TSym) == sizeof(char)) {
			return parseDatetime(in.data(), in.data() + in.size());
		}
		else
		{
			std::string utf8Str;
			Utf::Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
			return parseDatetime(utf8Str.data(), utf8Str.data() + utf8Str.size());
		}
	}

	/// <summary>
	/// Prints calendar date and time in the ISO 8601/UTC to target buffer.
	/// </summary>
	/// <returns>Pointer to next character or throws exception when passed buffer is not enough</returns>
	template <class TFractions>
	char* PrintIsoUtc(const CDateTimeParts<TFractions>& utc, char* pos, char* endPos)
	{
		if (pos != endPos)
		{
			if (utc.Year >= 10000) {
				*pos++ = '+';
			}
			const size_t outSize = snprintf(pos, endPos - pos, "%04" PRId64 "-%02d-%02dT%02d:%02d:%02d", utc.Year, utc.Month, utc.Day, utc.Hour, utc.Min, utc.Sec);
			if (outSize > 0)
			{
				pos += outSize;
				if (utc.SecFractions) {
					pos = PrintSecondsFractions(pos, endPos, utc.SecFractions.value());
				}
				if (pos != endPos)
				{
					*pos++ = 'Z';
					return pos;
				}
			}
		}
		throw std::runtime_error("Internal error: insufficient buffer size");
	}

	/// <summary>
	/// Converts UTC expressed in the `tm` structure to `std::string` (ISO 8601/UTC).
	/// </summary>
	template <typename TSym, typename TAllocator>
	void To(const tm& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		const CDateTimeParts<> utc(in);
		char buf[UtcBufSize];
		char* pos = PrintIsoUtc(utc, buf, buf + sizeof(buf));
		out.append(buf, pos);
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ssZ) to `tm` structure.
	/// </summary>
	template <typename TSym>
	void To(std::basic_string_view<TSym> in, tm& out)
	{
		const CDateTimeParts<> utc = ParseIsoUtc(in);
		if (utc.Year > std::numeric_limits<int>::max() || utc.Year < std::numeric_limits<int>::min()) {
			throw std::out_of_range("The target range of years in the `tm` structure is not sufficient");
		}
		out.tm_year = static_cast<int>(utc.Year);
		out.tm_mon = utc.Month;
		out.tm_mday = utc.Day;
		out.tm_hour = utc.Hour;
		out.tm_min = utc.Min;
		out.tm_sec = utc.Sec;
		out.tm_yday = out.tm_isdst = 0;
	}

	/// <summary>
	/// Converts from `std::chrono::time_point` to `std::string` (ISO 8601/UTC).
	///	Fractions of second will be rendered only when they present (non-zero).
	/// </summary>
	template <typename TClock, typename TDuration, typename TSym, typename TAllocator, std::enable_if_t<!TClock::is_steady, int> = 0>
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

		CDateTimeParts<std::common_type_t<std::chrono::milliseconds, TDuration>> utc;
		utc.Year = y + (m <= 2);
		utc.Month = static_cast<int>(m);
		utc.Day = static_cast<int>(d);
		utc.Hour = static_cast<int>(timeInSec / 3600);
		utc.Min = timeInSec % 3600 / 60;
		utc.Sec = timeInSec % 60;
		// Print fractions if time is based on a duration that's more precise than seconds
		if constexpr (std::ratio_less_v<typename TDuration::period, std::chrono::seconds::period>) {
			utc.SecFractions = timePart - std::chrono::seconds(timeInSec);
		}
		char buf[UtcBufSize];
		char* pos = PrintIsoUtc(utc, buf, buf + sizeof(buf));
		out.append(buf, pos);
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ss[.SSS]Z) to `std::chrono::time_point`.
	///	Fractions of second are optional, supported up to 9 digits.
	///	Examples of allowed dates:
	///	- 1872-01-01T00:00:00Z
	///	- 2023-07-14T22:44:51.925Z
	/// </summary>
	template <typename TSym, typename TClock, typename TDuration, std::enable_if_t<!TClock::is_steady, int> = 0>
	static void To(std::basic_string_view<TSym> in, std::chrono::time_point<TClock, TDuration>& out)
	{
		const CDateTimeParts<> utc = ParseIsoUtc(in);

		// Based on Howard Hinnant's algorithm
		static_assert(sizeof(int) >= 4, "This algorithm has not been ported to a 16 bit integers");
		auto const y = utc.Year - (utc.Month <= 2);
		auto const m = static_cast<unsigned>(utc.Month);
		auto const d = static_cast<unsigned>(utc.Day);
		auto const era = (y >= 0 ? y : y - 399) / 400;
		auto const yoe = static_cast<unsigned>(y - era * 400);				// [0, 399]
		auto const doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;	// [0, 365]
		auto const doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;				// [0, 146096]

		if (static_cast<int64_t>(era) > std::numeric_limits<int64_t>::max() / 146097ll ||
			static_cast<int64_t>(era) < std::numeric_limits<int64_t>::min() / 146097ll)
		{
			throw std::out_of_range("Target duration is not enough");
		}
		const int64_t days = era * 146097ll + (static_cast<int>(doe) - 719468);
		const auto time = static_cast<long long>(utc.Hour) * 3600 + static_cast<long long>(utc.Min) * 60 + utc.Sec;

		std::chrono::time_point<TClock, TDuration> tp;
		SafeAddDuration(tp, std::chrono::seconds(time));
		if (utc.SecFractions) {
			// Only seconds fractions can be rounded to target timepoint type
			SafeAddDuration(tp, std::chrono::round<TDuration>(utc.SecFractions.value()));
		}
		SafeAddDuration(tp, std::chrono::duration<int64_t, std::ratio<86400>>(days));
		out = tp;
	}

	/// <summary>
	/// Converts Unix time in the `CRawTime` to `std::string` (ISO 8601/UTC).
	/// </summary>
	template <typename TSym, typename TAllocator>
	void To(const CRawTime& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		const std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<time_t>> tp(std::chrono::seconds(in.Time));
		To(tp, out);
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ssZ) to Unix time in the `CRawTime`.
	/// </summary>
	template <typename TSym>
	void To(std::basic_string_view<TSym> in, CRawTime& out)
	{
		std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<time_t>> tp;
		To(in, tp);
		out.Time = tp.time_since_epoch().count();
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

		using days_t = std::chrono::duration<TRep, std::ratio<86400>>;
		using hours_t = std::chrono::duration<TRep, std::ratio<3600>>;
		using minutes_t = std::chrono::duration<TRep, std::ratio<60>>;
		using seconds_t = std::chrono::duration<TRep, std::ratio<1>>;

		char buf[UtcBufSize];
		char* pos = buf;
		char* end = buf + UtcBufSize;
		if (in.count() < 0) {
			*pos++ = '-';
		}
		*pos++ = 'P';
		auto timeLeft = in;
		pos = PrintDurationPart<days_t>(timeLeft, pos, end, 'D');
		if (timeLeft.count())
		{
			*pos++ = 'T';
			pos = PrintDurationPart<hours_t>(timeLeft, pos, end, 'H');
			pos = PrintDurationPart<minutes_t>(timeLeft, pos, end, 'M');
			pos = PrintDurationPart<seconds_t>(timeLeft, pos, end, 'S');
		}
		out.append(buf, pos);
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/Duration format: PnWnDTnHnMnS) to `std::chrono::duration`.
	///	Examples of allowed durations: P25DT55M41S, P1W, PT10H20.346S, -P10DT25M (supported signed durations, but they are no officially defined).
	/// Durations which contains years, month, or with base UTC (2003-02-15T00:00:00Z/P2M) are not allowed.
	///	The decimal fraction supported only for seconds part, maximum 9 digits.
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
					using TSrcRep = decltype(value);
					if (isDatePart)
					{
						if (type == 'W') {
							return SafeDurationCast<TTargetDuration>(std::chrono::duration<TSrcRep, std::ratio<604800>>(value));
						} if (type == 'D') {
							return SafeDurationCast<TTargetDuration>(std::chrono::duration<TSrcRep, std::ratio<86400>>(value));
						} if (type == 'Y' || type == 'M') {
							throw std::invalid_argument("An ISO duration that contains a year, or month is not allowed");
						}
					}
					else
					{
						if (type == 'H') {
							return SafeDurationCast<TTargetDuration>(std::chrono::duration<TSrcRep, std::ratio<3600>>(value));
						} if (type == 'M') {
							return SafeDurationCast<TTargetDuration>(std::chrono::duration<TSrcRep, std::ratio<60>>(value));
						} if (type == 'S') {
							return SafeDurationCast<TTargetDuration>(std::chrono::duration<TSrcRep, std::ratio<1>>(value));
						}
					}
					throw std::invalid_argument("Input string is not a valid ISO duration: PnWnDTnHnMnS");
				};

				if (pos != end && std::isdigit(*pos))
				{
					uint64_t value = 0;
					const std::from_chars_result result = std::from_chars(pos, end, value);
					if (result.ec == std::errc() && result.ptr != end)
					{
						pos = result.ptr;
						auto sym = *pos++;
						if (sym == '.' || sym == ',')
						{
							// Parse seconds fractions
							std::chrono::nanoseconds ns(0);
							pos = ParseSecondFractions(pos, end, ns);
							if (pos == nullptr) {
								throw std::invalid_argument("Input ISO duration has invalid fractions of second");
							}
							if (pos != end)
							{
								sym = *pos++;
								if (sym != 'S') {
									throw std::invalid_argument("Input ISO duration has fractions in the non-seconds part");
								}
							}
							SafeAddDuration(duration, std::chrono::round<TTargetDuration>(isNegative ? -ns : ns));
						}

						if (isNegative)
						{
							constexpr uint64_t maxI64Negative = 9223372036854775808u;
							if (value <= maxI64Negative) {
								SafeAddDuration(duration, transformToDuration(-static_cast<int64_t>(value), sym, isDatePart));
							}
							else {
								throw std::out_of_range("ISO duration contains too big number");
							}
						}
						else {
							SafeAddDuration(duration, transformToDuration(value, sym, isDatePart));
						}
						return pos;
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
				bool isNegative;
				if (isNegative = *pos == '-'; isNegative || *pos == '+') {
					++pos;
				}
				if (*pos == 'P')
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
			Utf::Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
			out = parseDuration(utf8Str.data(), utf8Str.data() + utf8Str.size());
		}
	}
}
