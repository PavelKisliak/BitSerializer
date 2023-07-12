/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <chrono>
#include <charconv>
#include "convert_utf.h"

namespace BitSerializer::Convert::Detail
{
	/// <summary>
	/// Converts time since epoch to UTC in the `tm` structure.
	/// </summary>
	inline bool toGmt(time_t in, tm& out)
	{
#ifdef WIN32
		if (gmtime_s(&out, &in) == 0) {
			return true;
		}
#else
		if (gmtime_r(&in, &out) != nullptr) {
			return true;
		}
#endif
		return false;
	}

	/// <summary>
	/// Converts UTC in the `tm` structure to a time value.
	/// </summary>
	inline bool fromGmt(tm& in, time_t& out)
	{
#ifdef _WIN32
		out = _mkgmtime(&in);
#else
		out = timegm(&in);
#endif
		return out != -1;
	}

	/// <summary>
	/// Converts from `std::chrono::system_clock::time_point` to `std::string` (ISO 8601/UTC).
	///	Dates before Unix Epoch (1 January 1970) are not supported (throws `std::out_of_range` exception).
	///	Milliseconds will be rendered only when they present (non-zero).
	/// </summary>
	template <typename TSym, typename TAllocator>
	void To(const std::chrono::system_clock::time_point& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		tm gmt{};
		const time_t time = std::chrono::system_clock::to_time_t(in);
		if (!toGmt(time, gmt) || time < 0) {
			throw std::out_of_range("Argument out of range");
		}
		gmt.tm_year += 1900;
		++gmt.tm_mon;

		constexpr size_t UtcBufSize = 32;
		char buffer[UtcBufSize];
		size_t outSize;

		const auto frSec = in - std::chrono::system_clock::from_time_t(time);
		if (frSec.count() > 0)
		{
			const int ms = static_cast<int>(std::chrono::round<std::chrono::milliseconds>(frSec).count());
			outSize = snprintf(buffer, UtcBufSize, "%d-%02d-%02dT%02d:%02d:%02d.%03dZ", gmt.tm_year, gmt.tm_mon, gmt.tm_mday, gmt.tm_hour, gmt.tm_min, gmt.tm_sec, ms);
		}
		else {
			outSize = snprintf(buffer, UtcBufSize, "%d-%02d-%02dT%02d:%02d:%02dZ", gmt.tm_year, gmt.tm_mon, gmt.tm_mday, gmt.tm_hour, gmt.tm_min, gmt.tm_sec);
		}
		if (outSize <= 0) {
			throw std::runtime_error("Unknown error");
		}
		out.assign(buffer, buffer + outSize);
	}

	/// <summary>
	/// Converts from `std::string_view` (ISO 8601/UTC format: YYYY-MM-DDThh:mm:ss[.SSS]Z) to `std::chrono::system_clock::time_point`.
	///	Dates before Unix Epoch (1 January 1970) are not supported (throws `std::out_of_range` exception).
	///	Examples of allowed dates:
	///	- 2023-05-28T18:59:36Z
	///	- 2023-05-28T18:59:36.925Z
	/// </summary>
	template <typename TSym>
	void To(std::basic_string_view<TSym> in, std::chrono::system_clock::time_point& out)
	{
		constexpr auto invalidInputStringError = "Input string is not a valid ISO datetime";

		auto parseInt = [&invalidInputStringError](const char* buf, const char* end, int& out, int maxValue = 0, char delimiter = 0) -> const char* {
			if (std::isdigit(*buf))
			{
				const std::from_chars_result result = std::from_chars(buf, end, out);
				if (result.ec == std::errc())
				{
					if (maxValue && out > maxValue) {
						throw std::invalid_argument(invalidInputStringError);
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
			throw std::invalid_argument(invalidInputStringError);
		};

		std::tm tm = {};
		int frSec = 0;
		auto parseDatetime = [&invalidInputStringError, &parseInt, &tm, &frSec](const char* pos, const char* end) {
			pos = parseInt(pos, end, tm.tm_year, 0, '-');
			pos = parseInt(pos, end, tm.tm_mon, 12, '-');
			pos = parseInt(pos, end, tm.tm_mday, 31, 'T');
			pos = parseInt(pos, end, tm.tm_hour, 23, ':');
			pos = parseInt(pos, end, tm.tm_min, 59, ':');
			pos = parseInt(pos, end, tm.tm_sec, 59);
			// Parse optional milliseconds
			if (const auto sym = *pos; sym == '.') {
				pos = parseInt(++pos, end, frSec, 999, 'Z');
			}
			else if (sym != 'Z') {
				throw std::invalid_argument(invalidInputStringError);
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

		tm.tm_year -= 1900;
		--tm.tm_mon;
		time_t time = 0;
		if (tm.tm_year < 70 || !fromGmt(tm, time) || time < 0) {
			throw std::out_of_range("Argument out of range");
		}
		out = std::chrono::system_clock::from_time_t(time);
		if (frSec) {
			out += std::chrono::system_clock::duration(std::chrono::milliseconds(frSec));
		}
	}
}
