/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <charconv>
#include <limits>
#include <stdexcept>
#include <string>
#include "convert_utf.h"

namespace BitSerializer::Convert::Detail
{
	/// <summary>
	/// Converts any UTF string to integer types.
	/// </summary>
	template <typename T, typename TSym, std::enable_if_t<(std::is_integral_v<T>), int> = 0>
	void To(std::basic_string_view<TSym> in, T& out)
	{
		const auto* it = in.data();
		const auto* end = it + in.size();

		// ReSharper disable once CppPossiblyErroneousEmptyStatements
		for (; (it != end) && (*it == 0x20 || *it == 0x09); ++it);	// Skip spaces

		const auto validateResult = [](std::from_chars_result rc, std::string_view str)
		{
			if (rc.ec != std::errc())
			{
				if (rc.ec == std::errc::result_out_of_range) {
					throw std::out_of_range("Argument out of range");
				}
				if (rc.ec == std::errc::invalid_argument) {
					throw std::invalid_argument("Input string is not a number");
				}
				throw std::runtime_error("Unknown error");
			}

			// Check that string does not contain decimal fractions (parsing a float number to integer is not allowed)
			if (rc.ptr + 1 < str.data() + str.size() && *rc.ptr == '.' && std::isdigit(*(rc.ptr + 1)))
			{
				throw std::invalid_argument("Unable to convert string with float number to integer");
			}
		};

		if constexpr (sizeof(TSym) == sizeof(char)) {
			// Uses from_chars only for convert integers as GCC does not support floating types
			validateResult(std::from_chars(it, end, out), in);
		}
		else {
			std::string utf8Str;
			Utf8::Encode(it, end, utf8Str);
			validateResult(std::from_chars(utf8Str.data(), utf8Str.data() + utf8Str.size(), out), utf8Str);
		}
	}

	namespace _stdWrappers
	{
		template <typename T> T _fromStr(const char*, char**) { throw; }
		template <typename T> T _fromStr(const wchar_t*, wchar_t**) { throw; }

		template <>	inline float _fromStr<float>(const char* str, char** out_strEnd) { return std::strtof(str, out_strEnd); }
		template <>	inline float _fromStr<float>(const wchar_t* str, wchar_t** out_strEnd) { return std::wcstof(str, out_strEnd); }

		template <>	inline double _fromStr<double>(const char* str, char** out_strEnd) { return std::strtod(str, out_strEnd); }
		template <>	inline double _fromStr<double>(const wchar_t* str, wchar_t** out_strEnd) { return std::wcstod(str, out_strEnd); }

		template <>	inline long double _fromStr<long double>(const char* str, char** out_strEnd) { return std::strtold(str, out_strEnd); }
		template <>	inline long double _fromStr<long double>(const wchar_t* str, wchar_t** out_strEnd) { return std::wcstold(str, out_strEnd); }
	}

	/// <summary>
	/// Converts any UTF string to floating types.
	/// </summary>
	template <typename T, typename TSym, std::enable_if_t<(std::is_floating_point_v<T>), int> = 0>
	void To(std::basic_string_view<TSym> in, T& out)
	{
		T result;
		bool isNaN;
		static constexpr size_t bufSize = 64;
		const size_t size = (in.size() < bufSize) ? in.size() : bufSize - 1;
		if constexpr (std::is_same_v<char, TSym>)
		{
			// Copy to temporary buffer for prepare null-terminated c-string
			char buf[bufSize];
			std::memcpy(buf, in.data(), size);
			buf[size] = 0;
			char* endPos = nullptr;
			result = _stdWrappers::_fromStr<T>(buf, &endPos);
			isNaN = buf == endPos;
		}
		else if constexpr (sizeof(TSym) == sizeof(wchar_t))
		{
			wchar_t buf[bufSize];
			std::wmemcpy(buf, reinterpret_cast<wchar_t const*>(in.data()), size);
			buf[size] = 0;
			wchar_t* endPos = nullptr;
			result = _stdWrappers::_fromStr<T>(buf, &endPos);
			isNaN = buf == endPos;
		}
		else
		{
			std::string utf8Str;
			Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
			char* endPos = nullptr;
			result = _stdWrappers::_fromStr<T>(utf8Str.c_str(), &endPos);
			isNaN = utf8Str.data() == endPos;
		}

		if (errno == ERANGE) {
			throw std::out_of_range("Argument out of range");
		}
		if (isNaN) {
			throw std::invalid_argument("Input string is not a number");
		}
		out = result;
	}

	/// <summary>
	/// Converts any UTF string to boolean. Supports conversion from "0|1" and "true|false" strings.
	/// </summary>
	template <typename TSym>
	void To(std::basic_string_view<TSym> in, bool& ret_Val)
	{
		const auto* startIt = in.data();
		const auto* endIt = startIt + in.size();

		// ReSharper disable once CppPossiblyErroneousEmptyStatements
		for (; (startIt != endIt) && (*startIt == 0x20 || *startIt == 0x09); ++startIt);	// Skip spaces

		const auto size = endIt - startIt;
		if (size >= 1)
		{
			if (std::isdigit(*startIt))
			{
				if (*startIt == '1' && (size == 1 || !std::isdigit(startIt[1])))
				{
					ret_Val = true;
					return;
				}

				if (*startIt == '0' && (size == 1 || !std::isdigit(startIt[1])))
				{
					ret_Val = false;
					return;
				}

				throw std::out_of_range("Argument out of range");
			}

			if (size >= 4 &&
				(startIt[0] == 't' || startIt[0] == 'T') &&
				(startIt[1] == 'r' || startIt[1] == 'R') &&
				(startIt[2] == 'u' || startIt[2] == 'U') &&
				(startIt[3] == 'e' || startIt[3] == 'E'))
			{
				ret_Val = true;
				return;
			}

			if (size >= 5 &&
				(startIt[0] == 'f' || startIt[0] == 'F') &&
				(startIt[1] == 'a' || startIt[1] == 'A') &&
				(startIt[2] == 'l' || startIt[2] == 'L') &&
				(startIt[3] == 's' || startIt[3] == 'S') &&
				(startIt[4] == 'e' || startIt[4] == 'E'))
			{
				ret_Val = false;
				return;
			}
		}

		throw std::invalid_argument("Input string is not a boolean");
	}

	//------------------------------------------------------------------------------

	/// <summary>
	/// Converts any integer types to any UTF string.
	/// </summary>
	template <class T, typename TSym, typename TAllocator, std::enable_if_t<(std::is_integral_v<T>), int> = 0>
	void To(const T& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		if constexpr (std::is_same_v<char, TSym>) {
			out.append(std::to_string(in));
		}
		else if constexpr (std::is_same_v<TSym, wchar_t>) {
			out.append(std::to_wstring(in));
		}
		else
		{
			const auto utf8Str = std::to_string(in);
			Utf8::Decode(utf8Str.cbegin(), utf8Str.cend(), out);
		}
	}

	namespace _formatTemplates
	{
		template <typename T> const char* _get() { throw; }
		template <typename T> const wchar_t* _getW() { throw; }

		template <>	constexpr const char* _get<float>() { return "%.6g"; }
		template <>	constexpr const wchar_t* _getW<float>() { return L"%.6g"; }

		template <>	constexpr const char* _get<double>() { return "%.15g"; }
		template <>	constexpr const wchar_t* _getW<double>() { return L"%.15g"; }

		template <>	constexpr const char* _get<long double>() { return "%.15Lg"; }
		template <>	constexpr const wchar_t* _getW<long double>() { return L"%.15Lg"; }
	}

	/// <summary>
	/// Converts any floating point types to any UTF string.
	/// </summary>
	template <class T, typename TSym, typename TAllocator, std::enable_if_t<(std::is_floating_point_v<T>), int> = 0>
	void To(const T& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		constexpr auto bufSize = std::numeric_limits<T>::digits + 1;
		if constexpr (sizeof(TSym) == sizeof(wchar_t)) {
			wchar_t buf[bufSize];
			const int result = swprintf(buf, bufSize, _formatTemplates::_getW<T>(), in);
			if (result < 0 || result >= bufSize) {
				throw std::overflow_error("Internal error");
			}
			out.append(std::cbegin(buf), std::cbegin(buf) + result);
		}
		else
		{
			char buf[bufSize];
			const int result = snprintf(buf, bufSize, _formatTemplates::_get<T>(), in);
			if (result < 0 || result >= bufSize) {
				throw std::overflow_error("Internal error");
			}
			if constexpr (std::is_same_v<char, TSym>) {
				out.append(std::cbegin(buf), std::cbegin(buf) + result);
			}
			else {
				Utf8::Decode(buf, buf + result, out);
			}
		}
	}

	/// <summary>
	/// Converts boolean type to any UTF string.
	/// The output representation is "true|false", please cast your boolean type to <int> if you would like to represent it as "1|0".
	/// </summary>
	template <typename TSym, typename TAllocator>
	void To(const bool& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		if (in)
		{
			out.append({ static_cast<TSym>('t'), static_cast<TSym>('r'), static_cast<TSym>('u'), static_cast<TSym>('e') });
		}
		else
		{
			out.append({ static_cast<TSym>('f'), static_cast<TSym>('a'), static_cast<TSym>('l'), static_cast<TSym>('s'), static_cast<TSym>('e') });
		}
	}
}
