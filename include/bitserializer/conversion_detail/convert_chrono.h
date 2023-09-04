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

	template <typename TDuration = std::chrono::nanoseconds,
		std::enable_if_t<TDuration::period::num == 1 && std::chrono::seconds::period::den < TDuration::period::den, int> = 0>
	struct tm_ext : tm
	{
		tm_ext() : tm() { }
		tm_ext(tm inTm, TDuration inSecFractions) : tm(inTm), secFractions(inSecFractions) { }

		TDuration secFractions{};
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
				if (static_cast<TOpRep>(duration.count()) > std::numeric_limits<TOpRep>::max() / TDivRatio::num ||
					static_cast<TOpRep>(duration.count()) < std::numeric_limits<TOpRep>::min() / TDivRatio::num)
				{
					throw std::out_of_range("Target duration is not enough");
				}
				const auto v = static_cast<TTargetRep>(static_cast<TOpRep>(duration.count()) * static_cast<TOpRep>(TDivRatio::num));
				return TTarget(v);
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

		TTargetDuration adaptedDuration{};
		try {
			adaptedDuration = SafeDurationCast<TTargetDuration>(duration);
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
	/// Parses fractions of second.
	/// Supported up to 9 digits, which is enough for values represented in nanoseconds.
	/// </summary>
	/// <returns>Pointer to next character or nullptr when failed</returns>
	template <class TTargetRep, class TTargetPeriod>
	const char* ParseSecondFractions(const char* pos, const char* endPos, std::chrono::duration<TTargetRep, TTargetPeriod>& outTime) noexcept
	{
		static_assert(TTargetPeriod::num == 1 && std::chrono::seconds::period::den < TTargetPeriod::den, "Target duration must be more precise than a second");
		uint32_t value;
		const std::from_chars_result result = std::from_chars(pos, endPos, value);
		if (result.ec == std::errc())
		{
			if (value == 0) {
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
	char* printSecondsFractions(char* pos, char* end, std::chrono::duration<TRep, TPeriod> time, bool fixedWidth = true) noexcept
	{
		static_assert(TPeriod::num == 1 && std::chrono::seconds::period::den < TPeriod::den, "Source duration must be more precise than a second");
		static_assert(std::chrono::nanoseconds::period::den >= TPeriod::den, "Maximum allowed precision is nanoseconds");
		if (time >= std::chrono::seconds(1)) {
			return nullptr;
		}
		if (pos != end) {
			*pos++ = '.';
		}
		auto val = TPeriod::den > std::nano::den ? std::chrono::floor<std::chrono::microseconds>(time).count() : time.count();
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
				if constexpr (isSecondsPart && TPeriod::num == 1 && std::chrono::seconds::period::den < TPeriod::den)
				{
					pos = printSecondsFractions(pos, endPos, timeLeft, false);
					timeLeft = std::chrono::duration < TRep, TPeriod>(0);
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
	template <typename TSym, typename TAllocator, typename TSecondsDur>
	void To(const tm_ext<TSecondsDur>& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out, int minFractionDigits = 3)
	{
		char buffer[UtcBufSize];
		const size_t outSize = snprintf(buffer, UtcBufSize, "%04d-%02d-%02dT%02d:%02d:%02d",
			in.tm_year, in.tm_mon, in.tm_mday, in.tm_hour, in.tm_min, in.tm_sec);
		auto pos = printSecondsFractions(buffer + outSize, buffer + UtcBufSize, in.secFractions);
		if (outSize <= 0 || pos == nullptr) {
			throw std::runtime_error("Unknown error");
		}
		if (in.tm_year >= 10000) {
			out.push_back('+');
		}
		out.append(buffer, pos);
		out.push_back('Z');
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ss[.SSS]Z)) to `tm_ext` structure (includes ms).
	///	Fractions of second are optional, supported up to 9 digits.
	/// </summary>
	template <typename TSym, typename TSecondsDur>
	static void To(std::basic_string_view<TSym> in, tm_ext<TSecondsDur>& out)
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
			if (pos != end && *pos == '.')
			{
				if (pos = ParseSecondFractions(++pos, end, utc.secFractions); pos == nullptr) {
					throw std::invalid_argument("Input ISO datetime has invalid fractions of second");
				}
			}
			// Should have 'Z' at the end of UTC datetime
			if (pos == end || *pos != 'Z') {
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

		tm_ext<std::common_type_t<std::chrono::milliseconds, TDuration>> utc;
		utc.tm_year = static_cast<int>(y) + (m <= 2);
		utc.tm_mon = static_cast<int>(m);
		utc.tm_mday = static_cast<int>(d);
		utc.tm_hour = static_cast<int>(timeInSec / 3600);
		utc.tm_min = timeInSec % 3600 / 60;
		utc.tm_sec = timeInSec % 60;
		// Print fractions if time is based on a duration that's more precise than seconds
		if constexpr (TDuration::period::num == 1 && std::chrono::seconds::period::den < TDuration::period::den)
		{
			utc.secFractions = timePart - std::chrono::seconds(timeInSec);
			To(utc, out);
		}
		else {
			To(static_cast<tm&>(utc), out);
		}
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ss[.SSS]Z) to `std::chrono::time_point`.
	///	Fractions of second are optional, supported up to 9 digits.
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
		auto const y = tmExt.tm_year - (tmExt.tm_mon <= 2);
		auto const m = static_cast<unsigned>(tmExt.tm_mon);
		auto const d = static_cast<unsigned>(tmExt.tm_mday);
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
		const auto time = static_cast<long long>(tmExt.tm_hour) * 3600 + static_cast<long long>(tmExt.tm_min) * 60 + tmExt.tm_sec;

		std::chrono::time_point<TClock, TDuration> tp;
		SafeAddDuration(tp, std::chrono::seconds(time));
		SafeAddDuration(tp, std::chrono::round<TDuration>(tmExt.secFractions));
		SafeAddDuration(tp, std::chrono::duration<int64_t, std::ratio<86400>>(days));
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

		using days = std::chrono::duration<TRep, std::ratio<86400>>;
		using hours = std::chrono::duration<TRep, std::ratio<3600>>;
		using minutes = std::chrono::duration<TRep, std::ratio<60>>;
		using seconds = std::chrono::duration<TRep, std::ratio<1>>;

		char buf[UtcBufSize];
		char* pos = buf;
		char* end = buf + UtcBufSize;
		if (in.count() < 0) {
			*pos++ = '-';
		}
		*pos++ = 'P';
		auto timeLeft = in;
		pos = PrintDurationPart<days>(timeLeft, pos, end, 'D');
		if (timeLeft.count())
		{
			*pos++ = 'T';
			pos = PrintDurationPart<hours>(timeLeft, pos, end, 'H');
			pos = PrintDurationPart<minutes>(timeLeft, pos, end, 'M');
			pos = PrintDurationPart<seconds>(timeLeft, pos, end, 'S');
		}
		out.append(buf, pos);
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/Duration format: PnWnDTnHnMnS) to `std::chrono::duration`.
	///	Examples of allowed durations: P25DT55M41S, P1W, PT10H20S, -P10DT25M (supported signed durations, but they are not officially defined).
	/// Durations which contains years, month, or with base UTC (2003-02-15T00:00:00Z/P2M) are not allowed.
	///	The decimal fraction supported only for seconds part, maximum 9 digits (enough for values with nanosecond precision).
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
						if (sym == '.')
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
							duration += std::chrono::round<TTargetDuration>(isNegative ? -ns : ns);
						}

						const auto origDur = duration;
						if (isNegative)
						{
							constexpr uint64_t maxI64Negative = 9223372036854775808u;
							if (value <= maxI64Negative)
							{
								if ((duration += transformToDuration(-static_cast<int64_t>(value), sym, isDatePart)) > origDur) {
									throw std::out_of_range("Target type is not enough to store parsed duration");
								}
							}
							else {
								throw std::out_of_range("ISO duration contains too big number");
							}
						}
						else
						{
							if ((duration += transformToDuration(value, sym, isDatePart)) < origDur) {
								throw std::out_of_range("Target type is not enough to store parsed duration");
							}
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
