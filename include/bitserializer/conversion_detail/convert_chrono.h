/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* Copyright (C) 2017 Howard Hinnant (datetime algorithms from 'date' library)  *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <chrono>
#include <charconv>
#include "convert_utf.h"

namespace BitSerializer::Convert::Detail
{
	/// <summary>
	/// Converts Unix time to UTC expressed in the `tm` structure.
	/// </summary>
	inline tm UnixTimeToUtc(time_t dateTime) noexcept
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
		if (time < 0) utc.tm_hour += 23;
		utc.tm_min = time % 3600 / 60;
		if (time < 0) utc.tm_min += 59;
		utc.tm_sec = time % 60;
		if (time < 0) utc.tm_sec += 60;
		return utc;
	}

	/// <summary>
	/// Converts UTC expressed in the `tm` structure to Unix time.
	/// </summary>
	inline time_t UtcToUnixTime(const tm& utc) noexcept
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
	/// Converts from `std::chrono::time_point` to `std::string` (ISO 8601/UTC).
	///	Milliseconds will be rendered only when they present (non-zero).
	/// </summary>
	template <typename TClock, typename TDuration, typename TSym, typename TAllocator, std::enable_if_t<(TClock::is_steady == false), int> = 0>
	static void To(const std::chrono::time_point<TClock, TDuration>& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		const time_t time = std::chrono::floor<std::chrono::seconds>(in).time_since_epoch().count();
		auto ms = static_cast<int>((in - std::chrono::seconds(time)).time_since_epoch().count());
		const tm utc = UnixTimeToUtc(time);

		constexpr size_t UtcBufSize = 32;
		char buffer[UtcBufSize];
		size_t outSize;
		if (ms != 0)
		{
			if (ms < 0) ms += 1000;
			outSize = snprintf(buffer, UtcBufSize, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", utc.tm_year, utc.tm_mon, utc.tm_mday, utc.tm_hour, utc.tm_min, utc.tm_sec, ms);
		}
		else {
			outSize = snprintf(buffer, UtcBufSize, "%04d-%02d-%02dT%02d:%02d:%02dZ", utc.tm_year, utc.tm_mon, utc.tm_mday, utc.tm_hour, utc.tm_min, utc.tm_sec);
		}
		if (outSize <= 0) {
			throw std::runtime_error("Unknown error");
		}
		out.assign(buffer, buffer + outSize);
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ss[.SSS]Z) to `std::chrono::time_point`.
	///	Examples of allowed dates:
	///	- 1872-01-01T00:00:00Z
	///	- 2023-07-14T22:44:51.925Z
	/// </summary>
	template <typename TSym, typename TClock, typename TDuration, std::enable_if_t<(TClock::is_steady == false), int> = 0>
	static void To(std::basic_string_view<TSym> in, std::chrono::time_point<TClock, TDuration>& out)
	{
		auto parseDatetimePart = [](const char* buf, const char* end, int& out, int maxValue = 0, char delimiter = 0) -> const char* {
			if (std::isdigit(*buf))
			{
				const std::from_chars_result result = std::from_chars(buf, end, out);
				if (result.ec == std::errc())
				{
					if (maxValue && out > maxValue) {
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

		std::tm utc = {};
		int ms = 0;
		auto parseDatetime = [&parseDatetimePart, &utc, &ms](const char* pos, const char* end) {
			static constexpr int daysInMonth[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
			pos = parseDatetimePart(pos, end, utc.tm_year, 0, '-');
			pos = parseDatetimePart(pos, end, utc.tm_mon, 12, '-');
			pos = parseDatetimePart(pos, end, utc.tm_mday, daysInMonth[utc.tm_mon - 1], 'T');
			pos = parseDatetimePart(pos, end, utc.tm_hour, 23, ':');
			pos = parseDatetimePart(pos, end, utc.tm_min, 59, ':');
			pos = parseDatetimePart(pos, end, utc.tm_sec, 59);
			// Parse optional milliseconds
			if (const auto sym = *pos; sym == '.') {
				pos = parseDatetimePart(++pos, end, ms, 999, 'Z');
			}
			else if (sym != 'Z') {
				throw std::invalid_argument("Input string is not a valid ISO datetime: YYYY-MM-DDThh:mm:ss[.SSS]Z");
			}
		};

		if constexpr (sizeof(TSym) == sizeof(char)) {
			parseDatetime(in.data(), in.data() + in.size());
		}
		else {
			std::string utf8Str;
			Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
			parseDatetime(utf8Str.data(), utf8Str.data() + utf8Str.size());
		}

		const auto time = UtcToUnixTime(utc);
		constexpr auto maxSec = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::time_point<TClock, TDuration>::max())
			.time_since_epoch().count();
		constexpr auto minSec = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::time_point<TClock, TDuration>::min())
			.time_since_epoch().count();
		if (time > maxSec || time < minSec) {
			throw std::out_of_range("Target timepoint range is not enough to store parsed datetime");
		}

		out = std::chrono::time_point<TClock, TDuration>(std::chrono::seconds(time));
		if (ms) {
			out += std::chrono::milliseconds(ms);
		}
	}
}
